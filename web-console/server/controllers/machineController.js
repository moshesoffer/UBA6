const logger = require('../utils/logger');
const { getMachines, createMachine, updateMachine, deleteMachine } = require('../services/machineService');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

exports.getMachines = async (req, res) => {
	try {
		const result = await withTimeout(getMachines(), AWAIT_TIMEOUT);
		res.json(result);
	} catch (error) {
		logger.error('getMachines', error);
		res.sendStatus(500);
	}
};

exports.createMachine = async (req, res) => {
	try {
		await withTimeout(createMachine(req.body), AWAIT_TIMEOUT);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('createMachine', error);
		res.sendStatus(500);
	}
};

exports.updateMachine = async (req, res) => {
	try {
		await withTimeout(updateMachine(req.params?.mac, req.body), AWAIT_TIMEOUT);
		res.end();
	} catch (error) {
		logger.error('updateMachine', error);
		res.sendStatus(500);
	}
};

exports.deleteMachine = async (req, res) => {
	try {
		await withTimeout(deleteMachine(req.params?.mac), AWAIT_TIMEOUT);
		res.status(204).end();
	} catch (error) {
		logger.error('deleteMachine', error);
		return res.status(400).json({ error: error.message });
	}
};

