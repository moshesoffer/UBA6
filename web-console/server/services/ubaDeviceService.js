const logger = require('../utils/logger');
const {validateString, validateArray,} = require('../utils/validators');
const pool = require('../db');
const { ubaDeviceModel } = require('../models');
const {selectQuery, createModel, updateModel, deleteModel} = require('../db/genericCRUD');

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

const getConnectedSum = async () => {
	const query = `
	SELECT COUNT(*) AS \`connected\` 
	FROM \`${ubaDeviceModel.tableName}\` 
	WHERE \`isConnected\` = 1;
	`;
	const rows = await selectQuery(ubaDeviceModel.tableName, query);
	return rows[0]?.connected;
}

const createUbaDevice = async (data, connection) => {
	await createModel(ubaDeviceModel, data, connection);
}

const updateUbaDevice = async (ubaSN, data) => {
	await updateModel(ubaDeviceModel, ubaSN, data);
}

const deleteUbaDevice = async (ubaSN, connection) => {
	await deleteModel(ubaDeviceModel, ubaSN, connection);
};

const getUbaDevice = async (connection, ubaSN) => {
	
	if (!validateString(ubaSN) || !validateString(ubaSN.trim())) {
		throw new Error(`Invalid ubaSN.`);
	}
	query = `SELECT * FROM \`${ubaDeviceModel.tableName}\` WHERE \`ubaSN\` = ?;`;
	const result = await selectQuery(ubaDeviceModel.tableName, query, [ubaSN.trim(),], connection);
	logger.info(`getUbaDevice Executing ubaSN: ${ubaSN}`);
	return result;
	
};

module.exports = {
	getUbaDevices,
	getUbaDevice,
	getConnectedSum,
	createUbaDevice,
	updateUbaDevice,
	deleteUbaDevice,
};
