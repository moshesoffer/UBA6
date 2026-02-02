const logger = require('../utils/logger');
const {	createTestRoutine, updateTestRoutine, getTestRoutines, deleteTestRoutine} = require('../services/testRoutineService');

exports.getTestRoutines = async (req, res) => {
	try {
		const result = await getTestRoutines();
		res.json(result);
	} catch (error) {
		logger.error('getTestRoutines', error);
		res.sendStatus(500);
	}
};

exports.createTestRoutine = async (req, res) => {
	try {
		await createTestRoutine(req.body);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('createTestRoutine', error);
		res.sendStatus(500);
	}
};

exports.updateTestRoutine = async (req, res) => {
	try {
		await updateTestRoutine(req.params?.id, req.body);
		res.end();
	} catch (error) {
		logger.error('updateTestRoutine', error);
		res.sendStatus(500);
	}
};

exports.deleteTestRoutine = async (req, res) => {
	try {
		await deleteTestRoutine(req.params?.id);
		res.status(204).end();
	} catch (error) {
		logger.error('deleteTestRoutine', error);
		res.sendStatus(500);
	}
};

/*const sleepsss = async(ms) => {
	return new Promise((resolve) => setTimeout(resolve, ms));
};*/

