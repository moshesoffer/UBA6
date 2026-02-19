const logger = require('../utils/logger');
const { machineModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');
const { getCountUbaDeviceByMachineMac } = require('./ubaDeviceService');
const { validateString } = require('../utils/validators');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const getMachines = async () => {
    return await withTimeout(selectQuery(machineModel.tableName, machineModel.selectAllQuery), AWAIT_TIMEOUT);
};

const createMachine = async (machine) => {
    await withTimeout(createModel(machineModel, machine), AWAIT_TIMEOUT);
};

const updateMachine = async (mac, machine) => {
    await withTimeout(updateModel(machineModel, mac, machine), AWAIT_TIMEOUT);
};

const deleteMachine = async (mac) => {
	const count = await withTimeout(getCountUbaDeviceByMachineMac(mac), AWAIT_TIMEOUT);
	if (count > 0) {
		throw new Error(`Machine has ${count} uba devices, can't delete.`);
	}
    await withTimeout(deleteModel(machineModel, mac), AWAIT_TIMEOUT);
};

const getMachine = async (machineMac) => {
	if (!validateString(machineMac) || !validateString(machineMac.trim())) {
		throw new Error(`Invalid machineMac.`);
	}
	const query = `SELECT * FROM \`${machineModel.tableName}\` WHERE \`mac\` = ?;`;
	const result = await withTimeout(selectQuery(machineModel.tableName, query, [machineMac.trim(),]), AWAIT_TIMEOUT);
	logger.info(`getMachine Executing machineMac: ${machineMac}`);
	return result[0];
	
};

module.exports = {
	getMachines,
	createMachine,
    updateMachine,
    deleteMachine,
    getMachine
};