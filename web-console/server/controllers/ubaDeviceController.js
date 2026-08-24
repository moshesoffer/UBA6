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
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const dateFromUtc = utcDate => {
	const date = new Date(utcDate);
	return new Date(date.getTime() + date.getTimezoneOffset() * 60000);
}

const formatSeconds = seconds => [
	parseInt(seconds / 60 / 60, 10),
	parseInt(seconds / 60 % 60, 10),
	parseInt(seconds % 60, 10),
	// eslint-disable-next-line prefer-named-capture-group
].join(':').replace(/\b(\d)\b/ug, '0$1');

const getRuntime = (timestamp, startTimestamp) => {
	const now = dateFromUtc(timestamp);
	const start = dateFromUtc(startTimestamp);

	let diff = now.getTime() - start.getTime();
	diff = Math.round(diff / 1000);

	return diff;
};

const runtimeDataMap = new Map();

const createRuntimeData = () => ({
	startTimeA: -1,
	pausedateChnlA: 0,
	runtimeChnlA: null,
	rundateChnlA: 0,

	startTimeB: -1,
	pausedateChnlB: 0,
	runtimeChnlB: null,
	rundateChnlB: 0,

	testLastStep: 0,
});

const getRuntimeData = ubaSN => {
	if (!runtimeDataMap.has(ubaSN)) {
		//logger.debug(`RUNTIME INITIALIZE: SN=${ubaSN}`);
		runtimeDataMap.set(ubaSN, createRuntimeData());
	} else {
		//logger.debug(`RUNTIME EXISTING: SN=${ubaSN}`);
	}

	return runtimeDataMap.get(ubaSN);
};

//fetching all data for main page
exports.getUbaDevices = async (req, res) => {
	try {
		const [running, ubaDevices, latestInstantTestResults] = await Promise.all([getRunningAmount(), getUbaDevices(), getAllLatestInstantTestResults()]);
		const ubaDevicesUniqueSN = [...new Map(ubaDevices.map(item => [item.ubaSN, item.ubaSN])).values()];
		//logger.debug(`uba-devices going to enrichUbaDevices`);
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

const updateRuntimeData = (ubaDevice, runtimeData, testState) => {
	const timestamp = ubaDevice.lastInstantResultsTimestamp;

		if (ubaDevice.channel === 'A') {
		if (
			testState === 'Charge' ||
			testState === 'Discharge' ||
			testState === 'Pause'
		) {
			if (runtimeData.startTimeA === -1) {
				runtimeData.startTimeA = timestamp;
			}

			const currTime = getRuntime(
				timestamp,
				runtimeData.startTimeA
			);

			runtimeData.rundateChnlA =
				currTime - runtimeData.pausedateChnlA;

			runtimeData.runtimeChnlA =
				formatSeconds(runtimeData.rundateChnlA);

		} else if ((testState === 'Init') ||
				   (testState === 'TestCompleate')) { 
			runtimeData.startTimeA = -1;
			runtimeData.pausedateChnlA = 0;
			runtimeData.runtimeChnlA = 0;
			runtimeData.rundateChnlA = 0;
		}
	}

	if (ubaDevice.channel === 'B') {
		if (
			testState === 'Charge' ||
			testState === 'Discharge' ||
			testState === 'Pause'
		) {
			if (runtimeData.startTimeB === -1) {
				runtimeData.startTimeB = timestamp;
			}

			const currTime = getRuntime(
				timestamp,
				runtimeData.startTimeB
			);

			runtimeData.rundateChnlB =
				currTime - runtimeData.pausedateChnlB;

			runtimeData.runtimeChnlB =
				formatSeconds(runtimeData.rundateChnlB);
			
		} else if ((testState === 'Init') ||
				   (testState === 'TestCompleate')) { 
			runtimeData.startTimeB = -1;
			runtimeData.pausedateChnlB = 0;
			runtimeData.runtimeChnlB = 0;
			runtimeData.rundateChnlB = 0;
		}
	}
};

let testLastStep = 0;

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
				//logger.debug(`==> Using last instant test result from memory for runningTestID ${lastInstantFromMem.memCreatedTime}`);
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
	
	// testState is now populated
	const runtimeData = getRuntimeData(ubaDevice.ubaSN);

	updateRuntimeData(
		{
			...ubaDevice,
			lastInstantResultsTimestamp: timestamp,
		},
		runtimeData,
		testState
	);

	//testLastStep = (testCurrentStep === 0) ? testLastStep : testCurrentStep;
	if (testCurrentStep !== 0) {
		runtimeData.testLastStep = testCurrentStep;
	}
	//logger.debug(`==> Using last ${testLastStep}`);

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
		runtimeData,
		testLastStep: runtimeData.testLastStep,
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

