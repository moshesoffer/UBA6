const logger = require('../utils/logger');
const { createUbaAndTest, deleteUbaDeviceAndTest } = require('../services/transactionsService');
const {
	getUbaDevices,
	getConnectedSum,
	updateUbaDevice,
} = require('../services/ubaDeviceService');
const {getRunningAmount, getAllLatestInstantTestResults} = require('../services/runningTestService');
const { ubaChannels,} = require('../utils/constants');
const { getLastInstantTestResult, getConnectedAmount } = require('../utils/testResultsHelper');

//fetching all data for main page
exports.getUbaDevices = async (req, res) => {
	try {
		logger.debug(`uba-devices going to call all promises`);
		//TODO getRunningAmount might be not needed because calling getUbaDevices already returns running tests and then can see what is running
		//TODO also getAllLatestInstantTestResults instead of calling db, go over all getUbaDevices running tests and get info from memory and if its not in memory then fetch from db and put in memory that way also next time it will be called we wont need to call db.
		const [running, ubaDevices, latestInstantTestResults] = await Promise.all([getRunningAmount(), getUbaDevices(), getAllLatestInstantTestResults()]);
		const ubaDevicesUniqueSN = [...new Map(ubaDevices.map(item => [item.ubaSN, item.ubaSN])).values()];
		logger.debug(`uba-devices going to enrichUbaDevices`);
		const ubaEnriched = enrichUbaDevices(ubaDevices, latestInstantTestResults);
		result = {
			ubaDevices: ubaEnriched,
			ubaTotal: {
				configured: ubaDevicesUniqueSN.length,
				connected: getConnectedAmount(),
				running,
			}
		};
		res.json(result);
	} catch (error) {
		logger.error('getUbaDevices', error);
		res.sendStatus(500);
	}
};

const enrichUbaDevices = (ubaDevices, latestInstantTestResults) => ubaDevices.map(ubaDevice => {

	let testState = null;
	let testCurrentStep = null;
	let voltage = null;
	let current = null;
	let temp = null;
	let capacity = null;
	let error = null;
	let timestamp = null;
	let memCreatedTime = null;
	const now = Date.now();

	for (const result of latestInstantTestResults) {
		if (ubaDevice.runningTestID === result.runningTestID) {
			let mostLatestObj = result;
			const lastInstantFromMem = getLastInstantTestResult(result.runningTestID);
			if (lastInstantFromMem && lastInstantFromMem.timestamp.getTime() >= result.timestamp.getTime()) {
				logger.debug(`Using last instant test result from memory for runningTestID ${lastInstantFromMem.memCreatedTime}`);
				mostLatestObj = lastInstantFromMem;
			}
			({
				testState,
				testCurrentStep,
				voltage,
				current,
				temp,
				capacity,
				error,
				timestamp,
				memCreatedTime,
			} = mostLatestObj);

			break;
		}
	}
	
	return {
		...ubaDevice,
		testState,
		testCurrentStep,
		voltage,
		current,
		temp,
		capacity,
		error,
		lastInstantResultsTimestamp: timestamp,
		ubaDeviceConnectedTimeAgoMs: memCreatedTime ? now - memCreatedTime.getTime() : null,
	};
});

exports.createUbaAndTest = async (req, res) => {
	try {
		if (!Object.values(ubaChannels).includes(req.body.ubaChannel)) {
			throw new Error(`Invalid value ${req.body.ubaChannel} of the ubaChannel parameter.`);
		}
		logger.info(`uba-devices going to createUbaDevice`);
		const ids = await createUbaAndTest(req.body);
		logger.info(`uba-devices finished to createUbaAndTest`);
		res.status(201).json(ids);
	} catch (error) {
		logger.error('createUbaAndTest', error);
		res.sendStatus(500);
	}
};

exports.updateUbaDevice = async (req, res) => {
	try {
		await updateUbaDevice(req.params?.serial, req.body);
		res.end();
	} catch (error) {
		logger.error('updateUbaDevice', error);
		res.sendStatus(500);
	}
};

exports.deleteUbaDeviceAndTest = async (req, res) => {
	try {
		await deleteUbaDeviceAndTest(req.params?.serial);
		res.status(204).end();
	} catch (error) {
		logger.error('deleteUbaDeviceAndTest', error);
		res.sendStatus(500);
	}
};

