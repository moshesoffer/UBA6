const logger = require('../utils/logger');
const { createUbaAndTest, deleteUbaDeviceAndTest } = require('../services/transactionsService');
const {
	getUbaDevices,
	getConnectedSum,
	updateUbaDevice,
} = require('../services/ubaDeviceService');
const {getRunningAmount, getInstantTestResults} = require('../services/runningTestService');
const { ubaChannels,} = require('../utils/constants');
const {enrichUbaDevices,} = require('../utils/helper');

//fetching all data for main page
exports.getUbaDevices = async (req, res) => {
	try {
		logger.debug(`uba-devices going to call all promises`);
		const [connected, running, ubaDevices, instantTestResults] = await Promise.all([getConnectedSum(), getRunningAmount(), getUbaDevices(), getInstantTestResults()]);
		const ubaDevicesUniqueSN = [...new Map(ubaDevices.map(item => [item.ubaSN, item.ubaSN])).values()];
		logger.debug(`uba-devices going to enrichUbaDevices`);
		const ubaEnriched = enrichUbaDevices(ubaDevices, instantTestResults);
		result = {
			ubaDevices: ubaEnriched,
			ubaTotal: {
				configured: ubaDevicesUniqueSN.length,
				connected,
				running,
			}
		};
		res.json(result);
	} catch (error) {
		logger.error('getUbaDevices', error);
		res.sendStatus(500);
	}
};

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
		res.status(204).json( { success: true } );
	} catch (error) {
		logger.error('deleteUbaDeviceAndTest', error);
		res.sendStatus(500);
	}
};

