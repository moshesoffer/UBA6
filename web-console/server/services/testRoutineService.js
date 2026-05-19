const logger = require('../utils/logger');
const { validatePlan, } = require('../utils/validators');

const { testRoutineModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');

const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const getTestRoutines = async() => {
	//Moshe
	logger.info(`getTestRoutines .`);
	return await selectQuery(testRoutineModel.tableName, testRoutineModel.selectAllQuery);;
};

const createTestRoutine = async data => {
	//Moshe
	logger.info(`createTestRoutine .`);
	let dataPlan = validatePlan(data?.plan, true);
	logger.info(`createTestRoutine.validatePlan ${dataPlan}`);
	data.plan = JSON.stringify(dataPlan);
	await createModel(testRoutineModel, data);;
}

const updateTestRoutine = async (id, data) => {
	//Moshe
	logger.info(`updateTestRoutine .`);
	let dataPlan = validatePlan(data?.plan, false);
	logger.info(`updateTestRoutine.validatePlan ${dataPlan}`);
	if(dataPlan) data.plan = JSON.stringify(dataPlan);
	await updateModel(testRoutineModel, id, data);;
}

const deleteTestRoutine = async (id) => {
	await deleteModel(testRoutineModel, id);;
};

module.exports = {
	createTestRoutine,
	updateTestRoutine,
	getTestRoutines,
	deleteTestRoutine
};
