const logger = require('../utils/logger');
const {validateString, validateObject, validateArray,validatePlan, validateTestResults} = require('../utils/validators');
const {checkOrderParameter, createFolderIfNotExists, readTextFromFile} = require('../utils/helper');
const {DATE_RANGE,reportsDataPath,testResultsFileName,} = require('../utils/constants');
const pool = require('../db');
const { selectQuery, updateModel, createModel} = require('../db/genericCRUD');
const { reportModel } = require('../models'); 
const { v4: uuidv4 } = require('uuid');
const path = require('node:path');
const fs = require('fs');
const {status} = require(`../utils/constants`);

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

		//TODO need to add index ubaSN and also timestampStart. and also maybe combinations of indexes togeter, most common.
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
};

const createReport = async (data, connection) => {
	let dataPlan = validatePlan(data?.plan, true);
	data.plan = JSON.stringify(dataPlan);
	const id = uuidv4(); // Generate a UUID
	data.id = id;
	await createModel(reportModel, data, connection);
	return id;
}

const createTestResultsFile = async (id, data, doValidateTestResults = true) => {
	let dataTestResults = doValidateTestResults ? validateTestResults(data.testResults, true) : data.testResults;

	const dirPath = path.join(reportsDataPath, id);
    createFolderIfNotExists(dirPath);
	const filePath = path.join(dirPath, testResultsFileName);
	fs.writeFileSync(filePath, JSON.stringify(dataTestResults), 'utf8');
}

const updateReport = async (id, data, connection) => {
	await updateModel(reportModel, id, data, connection);
}

const getTestResults = async ids => {
	if (!validateArray(ids, true)) {
		logger.error('Invalid ids:', ids);
		throw new Error('Invalid ids.');
	}
	const results = [];

	for (const id of ids) {
		try {
			const filePath = path.join(reportsDataPath, id, testResultsFileName);
			const content = readTextFromFile(filePath);
			results.push({
				reportID: id,
				testResults: JSON.parse(content)
			});
		} catch (err) {
			logger.error(`Failed to read report for ID ${id}:`, err);
			throw new Error(`Failed to read report for ID ${id}`);
		}
	}

	return results;
}

const getReportWithTestResults = async id => {
	const query = `SELECT r.* FROM \`${reportModel.tableName}\` as r WHERE r.\`id\` = ?;`;
	const rows = await selectQuery(reportModel.tableName, query, [id]);
	if(!rows || rows.length === 0) return [];
	rows[0].reportID = id;
	const testResults = readTextFromFile(path.join(reportsDataPath, id, testResultsFileName));
	rows[0].testResults = JSON.parse(testResults);
	return rows;
};

const getPendingReports = async (machineMac) => {
	let query = `
		SELECT r.*
		FROM \`${reportModel.tableName}\` as r
		WHERE (r.\`status\` & (${status.PENDING})) != 0
	`;
	if(machineMac){
		query += ` AND r.\`machineMac\` = ?;`;
		return await selectQuery(reportModel.tableName, query, [machineMac]);
	} else {
		query += `;`;
		return await selectQuery(reportModel.tableName, query);
	}
};

module.exports = {
	getReports,
	getTestResults,
	getReportWithTestResults,
	updateReport,
	createReport,
	createTestResultsFile,
	getPendingReports
};
