const logger = require('../utils/logger');
const { machineModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');
const { getCountUbaDeviceByMachineMac } = require('./ubaDeviceService');
const {validateString, } = require('../utils/validators');

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
	const count = await getCountUbaDeviceByMachineMac(mac);
	if (count > 0) {
		throw new Error(`Machine has ${count} uba devices, can't delete.`);
	}
    await deleteModel(machineModel, mac);
};

const getMachine = async (machineMac) => {
	
	if (!validateString(machineMac) || !validateString(machineMac.trim())) {
		throw new Error(`Invalid machineMac.`);
	}
	const query = `SELECT * FROM \`${machineModel.tableName}\` WHERE \`mac\` = ?;`;
	const result = await selectQuery(machineModel.tableName, query, [machineMac.trim(),]);
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