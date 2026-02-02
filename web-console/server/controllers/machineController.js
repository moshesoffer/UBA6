const logger = require('../utils/logger');
const { getMachines, createMachine, updateMachine, deleteMachine } = require('../services/machineService');

exports.getMachines = async (req, res) => {
	try {
		const result = await getMachines();
		res.json(result);
	} catch (error) {
		logger.error('getMachines', error);
		res.sendStatus(500);
	}
};

//currently not in use
exports.createMachine = async (req, res) => {
	try {
		await createMachine(req.body);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('createMachine', error);
		res.sendStatus(500);
	}
};

//currently not in use
exports.updateMachine = async (req, res) => {
	try {
		await updateMachine(req.params?.mac, req.body);
		res.end();
	} catch (error) {
		logger.error('updateMachine', error);
		res.sendStatus(500);
	}
};

//currently not in use
exports.deleteMachine = async (req, res) => {
	try {
		await deleteMachine(req.params?.mac);
		res.status(204).json( { success: true } );
	} catch (error) {
		logger.error('deleteMachine', error);
		res.sendStatus(500);
	}
};

