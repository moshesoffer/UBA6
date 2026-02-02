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
const { instantTestResultsModel, runningTestsModel } = require('../models');
const { selectQuery, updateModel, createModel } = require('../db/genericCRUD');

const addInstantTestResults = async (instantTestResults) => {
	let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
		logger.info(`addInstantTestResults [${instantTestResults.length}] going to add`);
		for (const item of instantTestResults) {
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
const getInstantTestResults = async () => {
	return await selectQuery(instantTestResultsModel.tableName, instantTestResultsModel.selectAllQuery);
};

const getInstantTestResultsGraphData = async runningTestID => {
	const query = `
	SELECT i.\`timestamp\`, i.\`voltage\`, i.\`current\`, i.\`capacity\`, i.\`temp\`
	FROM \`${instantTestResultsModel.tableName}\` AS i
	WHERE i.\`runningTestID\` = ?;
	`;
	return await selectQuery(instantTestResultsModel.tableName, query, [runningTestID]);
};

//TODO!!! this is only for mock and need to be removed
const mockInsert = 'INSERT INTO `InstantTestResults` (`id`,`runningTestID`,`timestamp`,`testState`,`testCurrentStep`,`voltage`,`current`,`temp`,`capacity`,`error`) VALUES ';
const mockInsertData = `(UUID(),'uba_SN','2024-05-22 13:44:43.000','charging',1,50.3858,0.0000,25.0000,0.00000,0),
(UUID(),'uba_SN','2024-05-22 13:44:44.000','charging',1,50.3860,0.0000,25.0000,0.00000,0),
(UUID(),'uba_SN','2024-05-22 13:44:45.000','charging',1,50.3860,0.0000,25.0000,0.00000,0),
(UUID(),'uba_SN','2024-05-22 13:44:46.000','charging',1,50.3858,0.0000,25.0000,0.00046,0),
(UUID(),'uba_SN','2024-05-22 13:44:47.000','charging',1,47.7349,25.0062,25.0000,0.00627,0),
(UUID(),'uba_SN','2024-05-22 13:44:48.000','charging',1,47.5561,25.0069,25.0000,0.01342,0),
(UUID(),'uba_SN','2024-05-22 13:44:49.000','charging',1,47.4405,25.0067,25.0000,0.02058,0),
(UUID(),'uba_SN','2024-05-22 13:44:50.000','charging',1,47.3332,25.0066,25.0000,0.02773,0),
(UUID(),'uba_SN','2024-05-22 13:44:51.000','charging',1,47.2441,25.0068,25.0000,0.03488,0),
(UUID(),'uba_SN','2024-05-22 13:44:52.000','charging',1,47.1662,25.0069,25.0000,0.04204,0),
(UUID(),'uba_SN','2024-05-22 13:44:53.000','charging',1,47.1054,25.0070,25.0000,0.04830,0),
(UUID(),'uba_SN','2024-05-22 13:44:54.000','charging',1,47.0437,25.0069,25.0000,0.05545,0),
(UUID(),'uba_SN','2024-05-22 13:44:55.000','charging',1,46.9886,25.0069,25.0000,0.06261,0),
(UUID(),'uba_SN','2024-05-22 13:44:56.000','charging',1,46.9501,25.0068,25.0000,0.06976,0),
(UUID(),'uba_SN','2024-05-22 13:44:57.000','charging',1,47.3786,20.0050,25.0000,0.07533,0),
(UUID(),'uba_SN','2024-05-22 13:44:58.000','charging',1,47.3660,20.0051,25.0000,0.08105,0),
(UUID(),'uba_SN','2024-05-22 13:44:59.000','charging',1,47.3447,20.0050,25.0000,0.08606,0),
(UUID(),'uba_SN','2024-05-22 13:45:00.000','charging',1,47.3271,20.0051,25.0000,0.09178,0),
(UUID(),'uba_SN','2024-05-22 13:45:01.000','charging',1,47.3012,20.0053,25.0000,0.09822,0),
(UUID(),'uba_SN','2024-05-22 13:45:02.000','charging',1,47.2754,20.0054,25.0000,0.10324,0),
(UUID(),'uba_SN','2024-05-22 13:45:03.000','charging',1,47.2561,20.0054,25.0000,0.10896,0),
(UUID(),'uba_SN','2024-05-22 13:45:04.000','charging',1,47.2341,20.0053,25.0000,0.11397,0),
(UUID(),'uba_SN','2024-05-22 13:45:05.000','charging',1,47.2136,20.0054,25.0000,0.11970,0),
(UUID(),'uba_SN','2024-05-22 13:45:06.000','charging',1,47.1942,20.0054,25.0000,0.12470,0),
(UUID(),'uba_SN','2024-05-22 13:45:07.000','charging',1,47.1739,20.0055,25.0000,0.13043,0),
(UUID(),'uba_SN','2024-05-22 13:45:08.000','charging',1,47.1642,20.0054,25.0000,0.13615,0),
(UUID(),'uba_SN','2024-05-22 13:45:09.000','charging',1,47.1461,20.0053,25.0000,0.14188,0),
(UUID(),'uba_SN','2024-05-22 13:45:10.000','charging',1,47.1288,20.0053,25.0000,0.14761,0),
(UUID(),'uba_SN','2024-05-22 13:45:11.000','charging',1,47.1105,20.0053,25.0000,0.15261,0),
(UUID(),'uba_SN','2024-05-22 13:45:12.000','charging',1,47.0993,20.0052,25.0000,0.15834,0),
(UUID(),'uba_SN','2024-05-22 13:45:13.000','charging',1,47.0807,20.0050,25.0000,0.16406,0),
(UUID(),'uba_SN','2024-05-22 13:45:14.000','charging',1,47.0630,20.0050,25.0000,0.16979,0),
(UUID(),'uba_SN','2024-05-22 13:45:15.000','charging',1,47.0533,20.0050,25.0000,0.17480,0),
(UUID(),'uba_SN','2024-05-22 13:45:16.000','charging',1,47.0440,20.0050,25.0000,0.18052,0),
(UUID(),'uba_SN','2024-05-22 13:45:17.000','charging',1,47.0271,20.0050,25.0000,0.18625,0),
(UUID(),'uba_SN','2024-05-22 13:45:18.000','charging',1,47.0169,20.0052,25.0000,0.19126,0),
(UUID(),'uba_SN','2024-05-22 13:45:19.000','charging',1,46.9993,20.0053,25.0000,0.19770,0),
(UUID(),'uba_SN','2024-05-22 13:45:20.000','charging',1,46.9904,20.0053,25.0000,0.20271,0),
(UUID(),'uba_SN','2024-05-22 13:45:21.000','charging',1,46.9817,20.0052,25.0000,0.20915,0),
(UUID(),'uba_SN','2024-05-22 13:45:22.000','charging',1,46.9723,20.0052,25.0000,0.21416,0),
(UUID(),'uba_SN','2024-05-22 13:45:23.000','charging',1,46.9637,20.0052,25.0000,0.21989,0),
(UUID(),'uba_SN','2024-05-22 13:45:24.000','charging',1,46.9476,20.0053,25.0000,0.22490,0),
(UUID(),'uba_SN','2024-05-22 13:45:25.000','charging',1,46.9391,20.0053,25.0000,0.23063,0),
(UUID(),'uba_SN','2024-05-22 13:45:26.000','charging',1,46.9304,20.0053,25.0000,0.23635,0),
(UUID(),'uba_SN','2024-05-22 13:45:27.000','charging',1,46.9219,20.0053,25.0000,0.24136,0),
(UUID(),'uba_SN','2024-05-22 13:45:28.000','charging',1,46.9131,20.0054,25.0000,0.24708,0),
(UUID(),'uba_SN','2024-05-22 13:45:29.000','charging',1,46.9043,20.0053,25.0000,0.25281,0),
(UUID(),'uba_SN','2024-05-22 13:45:30.000','charging',1,46.8952,20.0053,25.0000,0.25853,0),
(UUID(),'uba_SN','2024-05-22 13:45:31.000','charging',1,46.8929,20.0054,25.0000,0.26353,0),
(UUID(),'uba_SN','2024-05-22 13:45:32.000','charging',1,46.8838,20.0054,25.0000,0.26997,0),
(UUID(),'uba_SN','2024-05-22 13:45:33.000','charging',1,46.8758,20.0053,25.0000,0.27498,0),
(UUID(),'uba_SN','2024-05-22 13:45:34.000','charging',1,46.8673,20.0054,25.0000,0.28071,0),
(UUID(),'uba_SN','2024-05-22 13:45:35.000','charging',1,46.8591,20.0054,25.0000,0.28572,0),
(UUID(),'uba_SN','2024-05-22 13:45:36.000','charging',1,46.8502,20.0052,25.0000,0.29144,0),
(UUID(),'uba_SN','2024-05-22 13:45:37.000','charging',1,46.8488,20.0052,25.0000,0.29717,0),
(UUID(),'uba_SN','2024-05-22 13:45:38.000','charging',1,46.8408,20.0053,25.0000,0.30289,0),
(UUID(),'uba_SN','2024-05-22 13:45:39.000','charging',1,46.8321,20.0054,25.0000,0.30790,0),
(UUID(),'uba_SN','2024-05-22 13:45:40.000','charging',1,46.8304,20.0054,25.0000,0.31362,0),
(UUID(),'uba_SN','2024-05-22 13:45:41.000','charging',1,46.8230,20.0054,25.0000,0.31934,0),
(UUID(),'uba_SN','2024-05-22 13:45:42.000','charging',1,46.8150,20.0056,25.0000,0.32507,0),
(UUID(),'uba_SN','2024-05-22 13:45:43.000','charging',1,46.8131,20.0056,25.0000,0.33008,0),
(UUID(),'uba_SN','2024-05-22 13:45:44.000','charging',1,46.8060,20.0054,25.0000,0.33580,0),
(UUID(),'uba_SN','2024-05-22 13:45:45.000','charging',1,46.7983,20.0053,25.0000,0.34153,0),
(UUID(),'uba_SN','2024-05-22 13:45:46.000','charging',1,46.7967,20.0053,25.0000,0.34725,0),
(UUID(),'uba_SN','2024-05-22 13:45:47.000','charging',1,46.7896,20.0052,25.0000,0.35297,0),
(UUID(),'uba_SN','2024-05-22 13:45:48.000','charging',1,46.7872,20.0052,25.0000,0.35798,0),
(UUID(),'uba_SN','2024-05-22 13:45:49.000','charging',1,46.7804,20.0052,25.0000,0.36370,0),
(UUID(),'uba_SN','2024-05-22 13:45:50.000','charging',1,46.7779,20.0053,25.0000,0.36942,0),
(UUID(),'uba_SN','2024-05-22 13:45:51.000','charging',1,46.7717,20.0053,25.0000,0.37515,0),
(UUID(),'uba_SN','2024-05-22 13:45:52.000','charging',1,46.7693,20.0053,25.0000,0.38016,0),
(UUID(),'uba_SN','2024-05-22 13:45:53.000','charging',1,46.7630,20.0053,25.0000,0.38588,0),
(UUID(),'uba_SN','2024-05-22 13:45:54.000','charging',1,46.7602,20.0050,25.0000,0.39160,0),
(UUID(),'uba_SN','2024-05-22 13:45:55.000','charging',1,46.7545,20.0052,25.0000,0.39731,0),
(UUID(),'uba_SN','2024-05-22 13:45:56.000','charging',1,46.7515,20.0053,25.0000,0.40303,0),
(UUID(),'uba_SN','2024-05-22 13:45:57.000','charging',1,48.5168,0.0041,25.0000,0.40457,0),
(UUID(),'uba_SN','2024-05-22 13:45:58.000','charging',1,48.6233,0.0041,25.0000,0.40457,0),
(UUID(),'uba_SN','2024-05-22 13:45:59.000','charging',1,48.6802,0.0042,25.0000,0.40457,random_error);`;

//TODO!!! this is only for mock and need to be removed
const insertMockDataInstantTestResults = async (connection, ids) => {
	let query;
	try {
		if (!validateArray(ids)) {
			throw new Error(`Invalid ids.`);
		}
		const errorEnum = Math.floor(Math.random() * 2);
		for (let i = 0; i < ids.length; i++) {
			const regex = new RegExp('uba_SN', 'g');
    		const mockData = mockInsertData.replace(regex, ids[i]);
			const regex2 = new RegExp('random_error', 'g');
    		const mockData2 = mockData.replace(regex2, errorEnum);
			query = mockInsert + mockData2;
			logger.info(`insertMockDataInstantTestResults Executing query`);
			const [result,] = await connection.execute(query);
			if (result?.affectedRows < 1) {
				throw new Error(`Error inserting Mock Data InstantTestResults.`);
			}
		}
		return errorEnum;
	} catch (error) {
		logger.error(`Error insertMockDataInstantTestResults`, error);
		throw error;
	}
};

const getRunningAmount = async () => {
	const query = `
		SELECT COUNT(*) AS \`running\` 
		FROM \`${runningTestsModel.tableName}\`
		WHERE \`status\` & (1 << 5) != 0;
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
				(\`status\` & (1 << 5)) = 0 AND
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

module.exports = {
	getInstantTestResults,
	getInstantTestResultsGraphData,
	getRunningAmount,
	createRunningTest,
	deleteRunningTest,
	changeTestStatus,
	insertMockDataInstantTestResults,
	getRunningTestsByUbaSN,
	addInstantTestResults,
};
