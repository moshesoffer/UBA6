const logger = require('../utils/logger');
const { machineModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');

const getMachines = async () => {
    return await selectQuery(machineModel.tableName, machineModel.selectAllQuery);
};

const createMachine = async (machine) => {
    await createModel(machineModel, machine);
};

const updateMachine = async (mac, machine) => {
    await updateModel(machineModel, mac, machine);
};

const deleteMachine = async (mac) => {
    await deleteModel(machineModel, mac);
};


module.exports = {
	getMachines,
	createMachine,
    updateMachine,
    deleteMachine,

};