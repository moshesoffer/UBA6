const logger = require('../utils/logger');
const pool = require('../db');
const {
	createRunningTest,
	insertMockDataInstantTestResults,
	deleteRunningTest,
    getRunningTestsByUbaSN,
    changeTestStatus,
} = require('./runningTestService');
const {status, ubaChannels, RUNNING_TEST_ACTIONS, TEST_ROUTINE_CHANNELS} = require('../utils/constants');
const { createUbaDevice, deleteUbaDevice, } = require('./ubaDeviceService');
const { createReport, createReportData } = require('./reportService');

const createReportAndData = async (body) => {
    let connection;
    try {
        connection = await pool.getConnection();
        await connection.beginTransaction();
        logger.info(`createReportAndData`);
        const id = await createReport(connection, body);
        logger.info(`createReportAndData finished createReport`, {id: id});
        await createReportData(connection, id, body);
        logger.info(`createReportAndData finished createRunningTest`);
        await connection.commit();// Commit if all functions succeed
        return id;
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('createReportAndData Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

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
        //TODO!!! this is only for mock and need to be removed
        const errorEnum = await insertMockDataInstantTestResults(connection, ids);
        await connection.commit();// Commit if all functions succeed
        return {ids, errorEnum};
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
        //const uba = await getUbaDevice(connection, serial);
        //logger.info(`getUbaDevice`,{uba});

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
    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('deleteUbaDeviceAndTest Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

const actionOnRunningTest = async (runningTestID, testRoutineChannels, ubaSN, runningTestAction) => {
    let connection;
    const statusToSet = runningTestAction === RUNNING_TEST_ACTIONS.PAUSE ? status.PENDING_PAUSE : (runningTestAction === RUNNING_TEST_ACTIONS.RESUME) ? status.PENDING_RUNNING : (runningTestAction === RUNNING_TEST_ACTIONS.STOP) ? status.PENDING_STOP : (runningTestAction === RUNNING_TEST_ACTIONS.CONFIRM) ? status.PENDING_STANDBY : undefined;

    if(!statusToSet){
        logger.error(`Invalid runningTestAction ${runningTestAction}`);
        throw new Error(`Invalid action ${runningTestAction}`);
    }
    if(!runningTestID || !ubaSN) {
        logger.error(`mandatory fields runningTestID ${runningTestID}, ubaSN ${ubaSN}`);
        throw new Error(`mandatory fields runningTestID ${runningTestID}, ubaSN ${ubaSN}`);
    }
    logger.info(`actionOnRunningTest`, {runningTestID, testRoutineChannels, ubaSN, runningTestAction});
    
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

        //TODO!!! remove the setTimeout, this is only for presentation
        if(process.env.TEST_ENV) return;
        setTimeout(async () => {
			try {
				const statusToSetPhase2 = runningTestAction === RUNNING_TEST_ACTIONS.PAUSE ? status.PAUSED : (runningTestAction === RUNNING_TEST_ACTIONS.RESUME) ? status.RUNNING : (runningTestAction === RUNNING_TEST_ACTIONS.STOP) ? status.STOPPED : (runningTestAction === RUNNING_TEST_ACTIONS.CONFIRM) ? status.STANDBY : undefined;
                promises = [];
                for (let index = 0; index < runningTestIDs.length; index++) {
                    promises.push(changeTestStatus(runningTestIDs[index], statusToSetPhase2));
                }
                await Promise.all(promises);
			} catch (error) {
				logger.error('Error changing status to ' + statusToSetPhase2, error);
			}
		}, 7000);

    } catch (error) {
        if (connection) await connection.rollback(); // Rollback on error
        logger.error('resumeRunningTest Transaction error:', error);
        throw error;
    } finally {
        if (connection) connection.release(); // Release connection back to the pool
    }
};

module.exports = {
    runTest,
    createUbaAndTest,
    deleteUbaDeviceAndTest,
    actionOnRunningTest,
    createReportAndData,
};