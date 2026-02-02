const logger = require('../utils/logger');
const {validateString, validateArray,} = require('../utils/validators');
const pool = require('../db');
const { ubaDeviceModel } = require('../models');
const {selectQuery, createModel, updateModel, deleteModel} = require('../db/genericCRUD');
const { sendConnectionPendingTaskToUba, UI_FLOWS, UBA_DEVICE_ACTIONS, } = require('../utils/ubaCommunicatorHelper');

const getUbaDevices = async () => {
	const rows = await selectQuery(ubaDeviceModel.tableName, ubaDeviceModel.selectAllQuery);
	return rows.map(row => {
		let totalStagesAmount = 0;
		if (validateArray(row?.plan)) {
			totalStagesAmount = row.plan.length;
		}

		let result = {
			...row,
			totalStagesAmount,
		};
		delete result.plan;

		return result;
	});
};

const createUbaDevice = async (data, connection) => {
	await createModel(ubaDeviceModel, data, connection);
}

const updateUbaDevice = async (ubaSN, data) => {
	const ubaDevice = await getUbaDeviceByUbaSN(ubaSN);
	if(!ubaDevice) {
		throw new Error(`UbaDevice with serial ${ubaSN} does not exist.`);
	}
	await updateModel(ubaDeviceModel, ubaSN, data);

	if((data.machineMac && ubaDevice.machineMac !== data.machineMac) || (data.address && ubaDevice.address !== data.address) || (data.comPort && ubaDevice.comPort !== data.comPort)) {
		//if machineMac or address or comPort changed, need to send remove pending task to uba
		logger.info(`ubaDevice machineMac, address or comPort changed, sending remove pending task to uba for machineMac ${ubaDevice.machineMac}, address ${ubaDevice.address}, comPort ${ubaDevice.comPort}`);
		sendConnectionPendingTaskToUba( ubaDevice.machineMac, ubaDevice.address, ubaDevice.comPort, undefined, undefined, undefined, UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST, UI_FLOWS.EDIT_UBA_DEVICE );
	}
	const machineMacToUpdate = data.machineMac || ubaDevice.machineMac;
	const addressToUpdate = data.address || ubaDevice.address;
	const comPortToUpdate = data.comPort || ubaDevice.comPort;
	const nameToUpdate = data.name || ubaDevice.name;
	logger.info(`sending add pending task to uba for machineMac ${machineMacToUpdate}, address ${addressToUpdate}, comPort ${comPortToUpdate}, name ${nameToUpdate}`);
	sendConnectionPendingTaskToUba( machineMacToUpdate, addressToUpdate, comPortToUpdate, undefined, undefined, nameToUpdate, UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST, UI_FLOWS.EDIT_UBA_DEVICE );
}

const deleteUbaDevice = async (ubaSN, connection) => {
	await deleteModel(ubaDeviceModel, ubaSN, connection);
};

const getUbaDeviceByUbaSN = async (ubaSN, connection) => {
	
	if (!validateString(ubaSN) || !validateString(ubaSN.trim())) {
		throw new Error(`Invalid ubaSN.`);
	}
	const query = `SELECT * FROM \`${ubaDeviceModel.tableName}\` WHERE \`ubaSN\` = ?;`;
	const result = await selectQuery(ubaDeviceModel.tableName, query, [ubaSN.trim(),], connection);
	logger.info(`getUbaDevice Executing ubaSN: ${ubaSN}`);
	return result[0];
	
};

const getCountUbaDeviceByMachineMac = async (machineMac, connection) => {
	
	const query = `
		SELECT COUNT(*) AS \`ubaCount\` 
		FROM \`${ubaDeviceModel.tableName}\`
		WHERE machineMac = ?;
	`;
	const rows = await selectQuery(ubaDeviceModel.tableName, query, [machineMac]);
	return rows[0]?.ubaCount;
	
};

const getUbaDeviceByConstraint = async (machineMac, address, comPort, connection) => {
	
	if (!validateString(machineMac) || !validateString(machineMac.trim()) ||
		!validateString(address) || !validateString(address.trim()) ||
		!validateString(comPort) || !validateString(comPort.trim())) {
		throw new Error(`Invalid search.`);
	}
	const query = `SELECT * FROM \`${ubaDeviceModel.tableName}\` WHERE \`machineMac\` = ? and \`address\` = ? and \`comPort\` = ?;`;
	const result = await selectQuery(ubaDeviceModel.tableName, query, [machineMac, address, comPort], connection);
	logger.info(`getUbaDeviceByConstraint Executing machineMac: ${machineMac}, address: ${address}, comPort: ${comPort}`);
	return result[0];
	
};

module.exports = {
	getUbaDevices,
	getUbaDeviceByUbaSN,
	createUbaDevice,
	updateUbaDevice,
	deleteUbaDevice,
	getUbaDeviceByConstraint,
	getCountUbaDeviceByMachineMac,
};
