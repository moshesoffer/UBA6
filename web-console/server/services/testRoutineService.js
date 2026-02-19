const logger = require('../utils/logger');
const { validatePlan, } = require('../utils/validators');

const { testRoutineModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');

const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const getTestRoutines = async() => {
	return await selectQuery(testRoutineModel.tableName, testRoutineModel.selectAllQuery);;
};

const createTestRoutine = async data => {
	let dataPlan = validatePlan(data?.plan, true);
	data.plan = JSON.stringify(dataPlan);
	await withTimeout(createModel(testRoutineModel, data), AWAIT_TIMEOUT);;
}

const updateTestRoutine = async (id, data) => {
	let dataPlan = validatePlan(data?.plan, false);
	if(dataPlan) data.plan = JSON.stringify(dataPlan);
	await withTimeout(updateModel(testRoutineModel, id, data), AWAIT_TIMEOUT);;
}

const deleteTestRoutine = async (id) => {
	await withTimeout(deleteModel(testRoutineModel, id), AWAIT_TIMEOUT);;
};

module.exports = {
	createTestRoutine,
	updateTestRoutine,
	getTestRoutines,
	deleteTestRoutine
};
