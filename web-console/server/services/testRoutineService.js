const logger = require('../utils/logger');
const { validatePlan, } = require('../utils/validators');

const { testRoutineModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');

const getTestRoutines = async() => {
	return await selectQuery(testRoutineModel.tableName, testRoutineModel.selectAllQuery);;
};

const createTestRoutine = async data => {
	let dataPlan = validatePlan(data?.plan, true);
	data.plan = JSON.stringify(dataPlan);
	await createModel(testRoutineModel, data);
}

const updateTestRoutine = async (id, data) => {
	let dataPlan = validatePlan(data?.plan, false);
	if(dataPlan) data.plan = JSON.stringify(dataPlan);
	await updateModel(testRoutineModel, id, data);
}

const deleteTestRoutine = async (id) => {
	await deleteModel(testRoutineModel, id);
};

module.exports = {
	createTestRoutine,
	updateTestRoutine,
	getTestRoutines,
	deleteTestRoutine
};
