const logger = require('../utils/logger');
const { validateString, validateArray, } = require('../utils/validators');
const pool = require('../db');
const { ubaDeviceModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');
const { sendConnectionPendingTaskToUba, UI_FLOWS, UBA_DEVICE_ACTIONS, } = require('../utils/ubaCommunicatorHelper');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const getUbaDevices = async () => {
	try {

		const rows = await withTimeout(selectQuery(ubaDeviceModel.tableName, ubaDeviceModel.selectAllQuery), AWAIT_TIMEOUT);
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
		
	} catch (err) {
		//res.status(500).json({ status: 'error', db: 'down' });
	} finally {
		//res.status(500).json({ status: 'error', db: 'down' });
	}
};

const createUbaDevice = async (data, connection) => {
	await withTimeout(createModel(ubaDeviceModel, data, connection), AWAIT_TIMEOUT);
}

const updateUbaDevice = async (ubaSN, data) => {
	const ubaDevice = await withTimeout(getUbaDeviceByUbaSN(ubaSN), AWAIT_TIMEOUT);
	if (!ubaDevice) {
		throw new Error(`UbaDevice with serial ${ubaSN} does not exist.`);
	}
	await withTimeout(updateModel(ubaDeviceModel, ubaSN, data), AWAIT_TIMEOUT);

	if ((data.machineMac && ubaDevice.machineMac !== data.machineMac) || (data.address && ubaDevice.address !== data.address) || (data.comPort && ubaDevice.comPort !== data.comPort)) {
		//if machineMac or address or comPort changed, need to send remove pending task to uba
		logger.info(`ubaDevice machineMac, address or comPort changed, sending remove pending task to uba for machineMac ${ubaDevice.machineMac}, address ${ubaDevice.address}, comPort ${ubaDevice.comPort}`);
		sendConnectionPendingTaskToUba(ubaDevice.machineMac, ubaDevice.address, ubaDevice.comPort, undefined, undefined, undefined, UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST, UI_FLOWS.EDIT_UBA_DEVICE);
	}
	const machineMacToUpdate = data.machineMac || ubaDevice.machineMac;
	const addressToUpdate = data.address || ubaDevice.address;
	const comPortToUpdate = data.comPort || ubaDevice.comPort;
	const nameToUpdate = data.name || ubaDevice.name;
	logger.info(`sending add pending task to uba for machineMac ${machineMacToUpdate}, address ${addressToUpdate}, comPort ${comPortToUpdate}, name ${nameToUpdate}`);
	sendConnectionPendingTaskToUba(machineMacToUpdate, addressToUpdate, comPortToUpdate, undefined, undefined, nameToUpdate, UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST, UI_FLOWS.EDIT_UBA_DEVICE);
}

const deleteUbaDevice = async (ubaSN, connection) => {
	await withTimeout(deleteModel(ubaDeviceModel, ubaSN, connection), AWAIT_TIMEOUT);
};

const getUbaDeviceByUbaSN = async (ubaSN, connection) => {

	if (!validateString(ubaSN) || !validateString(ubaSN.trim())) {
		throw new Error(`Invalid ubaSN.`);
	}
	const query = `SELECT * FROM \`${ubaDeviceModel.tableName}\` WHERE \`ubaSN\` = ?;`;
	const result = await withTimeout(selectQuery(ubaDeviceModel.tableName, query, [ubaSN.trim(),], connection), AWAIT_TIMEOUT);
	logger.info(`getUbaDevice Executing ubaSN: ${ubaSN}`);
	return result[0];

};

const getCountUbaDeviceByMachineMac = async (machineMac, connection) => {

	const query = `
		SELECT COUNT(*) AS \`ubaCount\` 
		FROM \`${ubaDeviceModel.tableName}\`
		WHERE machineMac = ?;
	`;
	const rows = await withTimeout(selectQuery(ubaDeviceModel.tableName, query, [machineMac]), AWAIT_TIMEOUT);
	return rows[0]?.ubaCount;

};

const getUbaDeviceByConstraint = async (machineMac, address, comPort, connection) => {

	if (!validateString(machineMac) || !validateString(machineMac.trim()) ||
		!validateString(address) || !validateString(address.trim()) ||
		!validateString(comPort) || !validateString(comPort.trim())) {
		throw new Error(`Invalid search.`);
	}
	const query = `SELECT * FROM \`${ubaDeviceModel.tableName}\` WHERE \`machineMac\` = ? and \`address\` = ? and \`comPort\` = ?;`;
	const result = await withTimeout(selectQuery(ubaDeviceModel.tableName, query, [machineMac, address, comPort], connection), AWAIT_TIMEOUT);
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
