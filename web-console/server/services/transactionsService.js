const logger = require('../utils/logger');
const pool = require('../db');
const {
	createRunningTest,
	deleteRunningTest,
    getRunningTestsByUbaSN,
    changeTestStatus,
    getRunningTestByIdWithJoins
} = require('./runningTestService');
const {status, ubaChannels, TEST_ROUTINE_CHANNELS} = require('../utils/constants');
const { createUbaDevice, deleteUbaDevice, getUbaDeviceByUbaSN, } = require('./ubaDeviceService');
const { createReport, createTestResultsFile, updateReport} = require('./reportService');
const { sendConnectionPendingTaskToUba, UI_FLOWS, UBA_DEVICE_ACTIONS, } = require('../utils/ubaCommunicatorHelper');
const { formatSecondsToHHMMSS } = require('../utils/helper');

const createReportAndTestResult = async (body) => {
    let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
        logger.info(`createReportAndTestResult`);
        addTimeOfTest(body);
        const id = await createReport(body, connection);
        logger.info(`createReportAndTestResult finished createReport`, {id: id});
        await createTestResultsFile(id, body);
        logger.info(`createReportAndTestResult finished createTestResultsFile`);
        await connection.commit();// Commit if all functions succeed
        return id;
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('createReportAndTestResult Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

const updateReportAndTestResult = async (id, body) => {
    let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
        logger.info(`updateReportAndTestResult`);
        addTimeOfTest(body);
        await updateReport(id, body, connection);
        logger.info(`updateReportAndTestResult finished updateReport`, id);
        if(body.testResults) await createTestResultsFile(id, body);
        logger.info(`updateReportAndTestResult finished createTestResultsFile`);
        await connection.commit();// Commit if all functions succeed
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('updateReportAndTestResult Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

//this can run a batch of tests, and create a report for each test
const runTest = async (body) => {
    let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
        logger.info(`runTest`, {body});
        await deleteRunningTest(connection, body?.ubaSNs);
        logger.info(`runTest finished deleteRunningTest`, {ubaSNs: body?.ubaSNs});
        const ids = await createRunningTest(connection, body?.ubaSNs, body, status.PENDING_RUNNING);
        logger.info(`runTest finished createRunningTest`, {ids: ids});
        
        for (const value of ids) {
            const runningTest = await getRunningTestByIdWithJoins(value, connection);
            const { id, ...withoutId } = runningTest;
            const reportId = await createReport({ ...withoutId }, connection);
            logger.info(`runTest finished createReport`, {reportId: reportId});
            await createTestResultsFile(reportId, { testResults: [] }, false);//will create an empty file in file system
            logger.info(`runTest finished createTestResultsFile`, {reportId: reportId});
        }
        
        await connection.commit();// Commit if all functions succeed
        return {ids};
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('runTest Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

//will create ubaDevice and a runningTest
const createUbaAndTest = async (body) => {
    let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
        
        await createUbaDevice(body, connection);
            
        let ubaSNs = [{
            ubaSN: body.ubaSN,
            channel: body.ubaChannel,
        }];
        if (body.ubaChannel === ubaChannels.AB) {
            ubaSNs = [
                {
                    ubaSN: body.ubaSN,
                    channel: ubaChannels.A,
                },
                {
                    ubaSN: body.ubaSN,
                    channel: ubaChannels.B,
                },
            ];
        }
        logger.info(`uba-devices going to createRunningTest`);
        const ids = await createRunningTest(connection, ubaSNs, { ...body, }, status.STANDBY);
        logger.info(`uba-devices finished to createRunningTest`);
        await connection.commit();// Commit if all functions succeed

        sendConnectionPendingTaskToUba( body.machineMac, body.address, body.comPort, undefined, undefined, body.name, UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST, UI_FLOWS.ADD_UBA_DEVICE );

        return ids;
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('createUbaAndTest Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

const deleteUbaDeviceAndTest = async (serial) => {
    let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
        const ubaDevice = await getUbaDeviceByUbaSN(serial, connection);
        ubaSNs = [
			{
				ubaSN: serial,
				channel: ubaChannels.A,
			},
			{
				ubaSN: serial,
				channel: ubaChannels.B,
			},
		];
		logger.info(`uba-devices going to deleteRunningTest`, {ubaSNs});
		await deleteRunningTest(connection, ubaSNs);

        logger.info(`uba-devices going to deleteUbaDevice ${serial}`);
		await deleteUbaDevice(serial, connection);

        await connection.commit();// Commit if all functions succeed

        sendConnectionPendingTaskToUba( ubaDevice.machineMac, ubaDevice.address, ubaDevice.comPort, undefined, undefined, undefined, UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST, UI_FLOWS.DELETE_UBA_DEVICE );

    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('deleteUbaDeviceAndTest Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

const changeRunningTestStatus = async (runningTestID, testRoutineChannels, ubaSN, statusToSet) => {
    let connection;
    
    if(statusToSet === undefined){
        logger.error(`Invalid statusToSet ${statusToSet}`);
        throw new Error(`Invalid statusToSet ${statusToSet}`);
    }
    if(!runningTestID || !ubaSN) {
        logger.error(`mandatory fields runningTestID ${runningTestID}, ubaSN ${ubaSN}`);
        throw new Error(`mandatory fields runningTestID ${runningTestID}, ubaSN ${ubaSN}`);
    }
    logger.info(`changeRunningTestStatus`, {runningTestID, testRoutineChannels, ubaSN, statusToSet});
    
    try {
        connection = await pool.getConnection();
        const runningTestIDs = [runningTestID];
        if(testRoutineChannels && testRoutineChannels === TEST_ROUTINE_CHANNELS.A_AND_B){
            logger.info(`this is a test on both channels, going to find the other channel running test`);
            const runningsTests = await getRunningTestsByUbaSN(ubaSN, connection);
            //logger.info(`****1`, {runningsTests});
            const runningTestOnDifferentChannel = runningsTests.find(runningTest => runningTest.id !== runningTestID);
            logger.info(`runningTestOnDifferentChannel`, {runningTestOnDifferentChannel});
            if(runningTestOnDifferentChannel) runningTestIDs.push(runningTestOnDifferentChannel.id);
        }
        logger.info('runningTestIDs', { runningTestIDs });
        let promises = [];
        for (let index = 0; index < runningTestIDs.length; index++) {
            promises.push(changeTestStatus(runningTestIDs[index], statusToSet, connection));
        }
        await Promise.all(promises);

        await connection.commit();// Commit if all functions succeed

    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('resumeRunningTest Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

const addTimeOfTest = (body) => {
    const timeOfTest = body?.testResults && body?.testResults.length > 0 ? formatSecondsToHHMMSS(body.testResults[body.testResults.length - 1].timestamp) : undefined;
    body.timeOfTest = timeOfTest;
};

module.exports = {
    runTest,
    createUbaAndTest,
    deleteUbaDeviceAndTest,
    changeRunningTestStatus,
    createReportAndTestResult,
    updateReportAndTestResult,
};