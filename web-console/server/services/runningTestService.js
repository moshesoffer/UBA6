const logger = require('../utils/logger');
const { v4: uuidv4 } = require('uuid');
const {isTestRunning, TEST_ROUTINE_CHANNELS, status: statuses} = require('../utils/constants');
const {
	validateIsDefined,
	validateArray,
    validatePlan,
} = require('../utils/validators');
const {checkRunningTestKeys,} = require('../utils/helper');
const pool = require('../db');
const { instantTestResultsModel, runningTestsModel, ubaDeviceModel, machineModel, cellModel } = require('../models');
const { selectQuery, updateModel, createModel } = require('../db/genericCRUD');
const {status} = require(`../utils/constants`);
const { setLastInstantTestResult } = require('../utils/testResultsHelper');

const addInstantTestResults = async (instantTestResults) => {
	let connection;
	const insertArr = [];
	const instantTestResultsExistsMap = new Map();
    try {
		for (const item of instantTestResults) {
			if(item.isLogData === 1) {
				instantTestResultsExistsMap.set(item.runningTestID, true);
				insertArr.push(item);
			} else {
				logger.info(`addInstantTestResults item with isLogData false ${item.runningTestID}`);
				if(instantTestResultsExistsMap.get(item.runningTestID) !== true) {
					const amount = await getInstantTestResultsAmount(item.runningTestID);
					logger.info(`addInstantTestResults amount for runningTestID ${item.runningTestID} is ${amount}`);
					if(amount === 0) {
						logger.info(`addInstantTestResults inserting first item with isLogData false ${item.runningTestID}`);
						insertArr.push(item);
					}
					instantTestResultsExistsMap.set(item.runningTestID, true);
				}
			}
			setLastInstantTestResult(item.runningTestID, item);//even when inserting to db, we set in memory as well, that way we know whats connected in memory
		}
		if(insertArr.length === 0) {
			logger.info(`addInstantTestResults no items with isLogData true to insert to db`);
			return;
		}

        connection = await pool.getConnection();
        await connection.beginTransaction();
		logger.info(`addInstantTestResults going to insert [${insertArr.length}] instant test results`);
		for (const item of insertArr) {
			await createModel(instantTestResultsModel, item, connection);
		}
		logger.info(`addInstantTestResults finished to add`);
        
        await connection.commit();
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('addInstantTestResults Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

//this returns the latest test results for each running test
const getAllLatestInstantTestResults = async () => {
	return await selectQuery(instantTestResultsModel.tableName, instantTestResultsModel.selectAllQuery);
};

const getInstantTestResultsAmount = async runningTestID => {
	const query = `
		SELECT COUNT(*) AS \`amount\` 
		FROM \`${instantTestResultsModel.tableName}\`
		WHERE \`runningTestID\` = ?;
	`;
	const rows = await selectQuery(instantTestResultsModel.tableName, query, [runningTestID]);
	return rows[0]?.amount;
}

//not in use, but can be used to get most latest results for specific running test
const getLatestInstantTestResults = async runningTestID => {
	const query = `
	SELECT i.*
	FROM \`${instantTestResultsModel.tableName}\` AS i
	WHERE i.\`runningTestID\` = ? ORDER BY \`timestamp\` DESC LIMIT 1;
	`;
	return await selectQuery(instantTestResultsModel.tableName, query, [runningTestID]);
};

const getInstantTestResults = async runningTestID => {
	const query = `
	SELECT i.\`timestamp\`, i.\`voltage\`, i.\`current\`, i.\`capacity\`, i.\`temp\`
	FROM \`${instantTestResultsModel.tableName}\` AS i
	WHERE i.\`runningTestID\` = ?;
	`;
	return await selectQuery(instantTestResultsModel.tableName, query, [runningTestID]);
};

const getPendingRunningTests = async (machineMac) => {
	let query = `
		SELECT r.\`id\`, r.\`ubaSN\`, r.\`channel\`, r.\`status\`, r.\`testRoutineChannels\`, ud.\`machineMac\` ,r.\`noCellSerial\`, r.\`testName\`, r.\`plan\`, r.\`timestampStart\`
		FROM \`${runningTestsModel.tableName}\` AS r
		JOIN \`${ubaDeviceModel.tableName}\` AS ud ON r.\`ubaSN\` = ud.\`ubaSN\`
		WHERE (r.\`status\` & (${status.PENDING})) != 0
	`;
	if(machineMac){
		query += ` AND ud.\`machineMac\` = ?;`;
		return await selectQuery(instantTestResultsModel.tableName, query, [machineMac]);
	} else {
		query += `;`;
		return await selectQuery(instantTestResultsModel.tableName, query);
	}
};

const getRunningAmount = async () => {
	const query = `
		SELECT COUNT(*) AS \`running\` 
		FROM \`${runningTestsModel.tableName}\`
		WHERE (\`status\` & (${status.IS_TEST_RUNNING})) != 0;
	`;
	const rows = await selectQuery(runningTestsModel.tableName, query);
	return rows[0]?.running;
}

const createRunningTest = async (connection, ubaSNs, data, status) => {
	let query;
	let updateFields = [];
	let updatePlaceholders = [];
	let updateValues = [];
	let values = [];
	let updateValuesCompleted = [];

	try {
		checkRunningTestKeys(ubaSNs);

		/*
		We insert to DB only those values, which have defined keys.
		See the FIELDS constant.
		*/
		const preparedFields = runningTestsModel.createProperties;
		for (const field of preparedFields) {
			if (field === 'plan') {
				if(data.plan){//isnt mandatory
					let dataPlan = validatePlan(data.plan, true);
					const plan = JSON.stringify(dataPlan);
					updateFields.push('`plan`');
					updatePlaceholders.push(`?`);
					updateValues.push(plan);
				}
			} else if (validateIsDefined(data[field])) {
				updateFields.push(`\`${field}\``);
				updatePlaceholders.push(`?`);
				updateValues.push(data[field]);
			}
		}
		if (status !== statuses.STANDBY) {//in standby no need to set testRoutineChannels, this is when creating ubaDevice and test
			updateFields.push(`\`testRoutineChannels\``);
			updatePlaceholders.push(`?`);

			if(data.channel && data.channel === TEST_ROUTINE_CHANNELS.A_AND_B) {
				logger.info('this is running a test for both channels');
				updateValues.push(TEST_ROUTINE_CHANNELS.A_AND_B);
			} else if (data.channel){
				logger.info('this is single channel test');
				updateValues.push(TEST_ROUTINE_CHANNELS.A_OR_B);
			} else {
				logger.info('this is batch run');
				updateValues.push(TEST_ROUTINE_CHANNELS.A_OR_B);
			}
		}
		const ids = [];
		ubaSNs.forEach(item => {
			const id = uuidv4(); // Generate a UUID
			ids.push(id);
			if(updatePlaceholders.length > 0) {
				values.push(`('${id}', ?, '${item.channel}', CURRENT_TIMESTAMP(), '${status}', ${updatePlaceholders.join(', ')})`);
			} else {
				values.push(`('${id}', ?, '${item.channel}', CURRENT_TIMESTAMP(), '${status}')`);
			}
			
			updateValuesCompleted = updateValuesCompleted.concat(item.ubaSN, updateValues);
		});

		if(updateFields.length > 0) {
			query = `
			INSERT INTO \`${runningTestsModel.tableName}\`
			(\`id\`, \`ubaSN\`, \`channel\`, \`timestampStart\`, \`status\`, ${updateFields.join(', ')}) VALUES ${values.join(', ')};`;
		} else {
			query = `
			INSERT INTO \`${runningTestsModel.tableName}\`
			(\`id\`, \`ubaSN\`, \`channel\`, \`timestampStart\`, \`status\`) VALUES ${values.join(', ')};`;
		}
		

		logger.info(`createRunningTest Executing query: [${query}] [${updateValuesCompleted}]`);
		const [result,] = await connection.execute(query, updateValuesCompleted);
		if (result?.affectedRows < 1) {
			throw new Error(`Error creating RunningTests.`);
		}
		return ids;
	} catch (error) {
		logger.error(`Error createRunningTest [${query}] [${updateValuesCompleted}]`, error);
		throw error;
	}
}

const changeTestStatus = async (runningTestID, newStatus, openedConnection) => {
    await updateModel(runningTestsModel, runningTestID, {status: newStatus}, openedConnection);
}

const deleteRunningTest = async (connection, ubaSNs) => {
	let query;
	let selectQuery;
	let updatePlaceholders = [];
	let updateValues = [];

	try {
		checkRunningTestKeys(ubaSNs);

		for (let i = 0; i < ubaSNs.length; i++) {
			updatePlaceholders.push('(?, ?)');
		}
		ubaSNs.forEach(values => updateValues.push(values.ubaSN, values.channel));
		selectQuery = `
			SELECT * FROM \`${runningTestsModel.tableName}\`
			WHERE
				(\`ubaSN\`, \`channel\`) IN (${updatePlaceholders.join(', ')});
			`;
		logger.info(`SELECT RunningTests`);
		const [selectResult,] = await connection.execute(selectQuery, updateValues);
		let resultArray = Object.keys(selectResult).map(key => selectResult[key]);
		logger.info(`resultArray.length ${resultArray.length}`);
		const runningTests = resultArray.filter(item => { return isTestRunning(item.status); });
		logger.info('resultArray after filter - only running', {runningTests});
		if(runningTests.length > 0) {
			throw new Error(`Error deleteRunningTest. Test is running`);
		}

		query = `
			DELETE FROM \`${runningTestsModel.tableName}\`
			WHERE
				(\`status\` & (${status.IS_TEST_RUNNING})) = 0 AND
				(\`ubaSN\`, \`channel\`) IN (${updatePlaceholders.join(', ')});
			`;

		logger.info(`deleteRunningTest Executing query: [${query}] [${updateValues}]`);
		const [result,] = await connection.execute(query, updateValues);
		logger.info(`result?.affectedRows ${result?.affectedRows} ${resultArray.length}`)
		if (result?.affectedRows !== resultArray.length) {
			throw new Error(`Error deleteRunningTest.`);
		}
		logger.debug(`Deleted ${JSON.stringify(result?.affectedRows)} rows for "${JSON.stringify(ubaSNs)}" parameters.`);
	} catch (error) {
		logger.error(`Error deleteRunningTest executing [${query}] [${updateValues}]`, error);
		throw error;
	}
}

const getRunningTestsByUbaSN = async (ubaSN, connection) => {
	const query = `
	SELECT *
	FROM \`${runningTestsModel.tableName}\` AS rt
	WHERE rt.\`ubaSN\` = ?;
	`;
    const rows = await selectQuery(runningTestsModel.tableName, query, [ubaSN,], connection);
    if(rows.length > 2) {
        throw new Error(`Error getRunningTestsByUbaSN cant be more than 2 channels for same ubaSN.`);
    }
    return rows;
};

const getRunningTestByIdWithJoins = async (runningTestID, connection) => {
	const query = `
	SELECT rt.*, m.name AS machineName, m.mac AS machineMac, cm.chemistry
	FROM \`${runningTestsModel.tableName}\` AS rt
	JOIN \`${ubaDeviceModel.tableName}\` AS ud ON rt.\`ubaSN\` = ud.\`ubaSN\`
	JOIN \`${machineModel.tableName}\` AS m ON ud.\`machineMac\` = m.\`mac\`
	JOIN \`${cellModel.tableName}\` AS cm ON rt.\`cellPN\` = cm.\`itemPN\`
	WHERE rt.\`id\` = ?;
	`;
	const rows = await selectQuery(runningTestsModel.tableName, query, [runningTestID], connection);
	if(rows.length !== 1) {
		throw new Error(`Error getRunningTestByIdWithJoins cant be more than 1 running test for same id.`);
	}
	return rows[0];
};

module.exports = {
	getAllLatestInstantTestResults,
	getLatestInstantTestResults,
	getInstantTestResultsAmount,
	getInstantTestResults,
	getPendingRunningTests,
	getRunningTestByIdWithJoins,
	getRunningAmount,
	createRunningTest,
	deleteRunningTest,
	changeTestStatus,
	getRunningTestsByUbaSN,
	addInstantTestResults,
};
