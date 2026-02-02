const logger = require('../utils/logger');
const {validateString, validateObject, validateArray,validatePlan, validateTestResults} = require('../utils/validators');
const {checkOrderParameter,} = require('../utils/helper');
const {DATE_RANGE,} = require('../utils/constants');
const pool = require('../db');
const { selectQuery, updateModel, createModel} = require('../db/genericCRUD');
const { reportsDataModel, reportModel } = require('../models'); 
const { v4: uuidv4 } = require('uuid');

const getReports = async metadata => {
	let connection;
	let query = '';
	let queryCount = '';
	let whereClause = '';
	let sqlPredicate = [];
	let updateValues = [];
	let updateQueryCount = [];

	try {
		checkOrderParameter(metadata);
		const offset = metadata.page * metadata.rowsPerPage;
		connection = await pool.getConnection();

		if (validateObject(metadata.filters, true)) {
			const filterKeys = Object.keys(metadata.filters);

			for (const key of filterKeys) {
				if (!validateString(metadata.filters[key])) {
					continue;
				}

				if (key === 'machineName') {
					sqlPredicate.push(` \`machineName\` = ?`);
					updateValues.push(metadata.filters.machineName);
					continue;
				}

				if (key === 'dateRange') {
					if (!Object.keys(DATE_RANGE).includes(metadata.filters.dateRange)) {
						throw new Error(`Invalid date range ${metadata.filters.dateRange}`);
					}

					sqlPredicate.push(`	\`timestampStart\` >= (NOW() - INTERVAL ${DATE_RANGE[metadata.filters.dateRange]})`);
					continue;
				}

				sqlPredicate.push(` \`${key}\` like ?`);
				updateValues.push(`%${metadata.filters[key]}%`);
			}

			if (validateArray(sqlPredicate, true)) {
				whereClause = `WHERE ${sqlPredicate.join(' AND ')}`;
			}
		}

		updateQueryCount = [...updateValues];
		updateValues.push(metadata.rowsPerPage.toString());
		updateValues.push(offset.toString());

		//TODO!!! need to add index ubaSN and also timestampStart. and also maybe combinations of indexes togeter, most common.
		query = `
		SELECT *
		FROM \`${reportModel.tableName}\`
		${whereClause}
		ORDER BY ${connection.escapeId(metadata.orderBy)} ${metadata.order.toUpperCase()}
		LIMIT ? 
		OFFSET ?;
		`;

		queryCount = `
		SELECT COUNT(*) AS \`count\` 
		FROM \`${reportModel.tableName}\`
		${whereClause};
		`;

		logger.info(`getReports Executing query: [${query}] [${updateValues}]`);
		const [rows,] = await connection.execute(query, updateValues);
		logger.info(`getReports Executing queryCount: [${queryCount}] [${updateQueryCount}]`);
		const [countResults,] = await connection.execute(queryCount, updateQueryCount);
		const count = countResults[0].count;

		return {
			rows,
			count,
		};
	} catch (error) {
		logger.error(`Error getReports executing`, error);
		throw error;
	} finally {
		if (connection) {
			connection.release();
		}
	}
}

const createReport = async (connection, data) => {
	let dataPlan = validatePlan(data?.plan, true);
	data.plan = JSON.stringify(dataPlan);
	const id = uuidv4(); // Generate a UUID
	data.id = id;
	await createModel(reportModel, data, connection);
	return id;
}

const createReportData = async (connection, id, data) => {
	let dataTestResults = validateTestResults(data.testResults, true);
	const reportData = { reportID: id, testResults: JSON.stringify(dataTestResults) };
	await createModel(reportsDataModel, reportData, connection);
}

const updateReport = async (id, data) => {
	let dataPlan = validatePlan(data?.plan, false);
	if(dataPlan) data.plan = JSON.stringify(dataPlan);
	await updateModel(reportModel, id, data);
}

const getReportData = async ids => {
	if (!validateArray(ids, true)) {
		logger.error('Invalid ids:', ids);
		throw new Error('Invalid ids.');
	}
	const placeholders = ids.map(() => '?').join(', ');
	const query = `SELECT * FROM \`${reportsDataModel.tableName}\` WHERE \`reportID\` IN (${placeholders});`;
	return await selectQuery(reportsDataModel.tableName, query, ids);
}

const getReportWithTestResults = async id => {
	const query = `SELECT r.*, rd.* FROM \`${reportModel.tableName}\` as r JOIN \`${reportsDataModel.tableName}\` AS rd ON r.\`id\` = rd.\`reportID\` WHERE r.\`id\` = ?;`;
	return await selectQuery(reportsDataModel.tableName, query, [id]);
}

module.exports = {
	getReports,
	getReportData,
	getReportWithTestResults,
	updateReport,
	createReport,
	createReportData
};
