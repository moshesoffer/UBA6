const logger = require('../utils/logger');
const {
	getPendingRunningTests
} = require('../services/runningTestService');
const { getWaiter, sendConnectionPendingTaskToUba, UI_FLOWS, UBA_DEVICE_ACTIONS, PENDING_TASKS_TYPES, getAllWaitersPerMac, generateConnectionUbaDeviceKey, ACTION_RESULT, getActionResultByCode } = require('../utils/ubaCommunicatorHelper');
const { getUbaDeviceByUbaSN, getUbaDeviceByConstraint } = require('../services/ubaDeviceService');
const { getMachine } = require('../services/machineService');
const { getPendingReports } = require('../services/reportService');

//this is being called by the uba service
const getPendingTasks = async (machineMac) => {
    try {
        const pendingTasks = {pendingConnectionUbaDevices: [], pendingRunningTests: [], pendingReports: []};
        const pendingRunningTests = await getPendingRunningTests(machineMac);
        pendingTasks.pendingRunningTests = pendingRunningTests;
        const pendingReports = await getPendingReports(machineMac);
        pendingTasks.pendingReports = pendingReports;
        
        for (const pendingRunningTest of pendingRunningTests) {
            const pendingReport = pendingReports.find(report => report.ubaSN === pendingRunningTest.ubaSN && report.channel === pendingRunningTest.channel && new Date(report.timestampStart).getTime() == new Date(pendingRunningTest.timestampStart).getTime());
            if(pendingReport){
                pendingRunningTest.reportId = pendingReport.id;
                pendingReport.pendingRunningTestId = pendingRunningTest.id;
            } else {
                logger.info(`No matching pending report found for pending running test with ubaSN: ${pendingRunningTest.ubaSN}, channel: ${pendingRunningTest.channel}, timestampStart: ${pendingRunningTest.timestampStart}`);
            }
        }
        
        const allWaitersPerMac = getAllWaitersPerMac(machineMac);
        const connectionUbaDeviceWaiters = allWaitersPerMac?.get(PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE);
        if(connectionUbaDeviceWaiters) {
            for (const [key, value] of connectionUbaDeviceWaiters.entries()) {
                const { uiFlow, requestStartTime, ubaSN, ubaChannel, ...withoutOthers } = value.body;
                pendingTasks.pendingConnectionUbaDevices.push(withoutOthers);
            }
        }
        logger.info(`getPendingTasks for machineMac: ${machineMac}, pendingTasks: ${JSON.stringify(pendingTasks)}`);
        return pendingTasks;
    } catch (error) {
        logger.error('getPendingTasks', error);
        throw error;
    }
};

//this is being called by the uba service
const pendingTasksExecuted = async (pendingTask) => {
    if(pendingTask.pendingUbaDevice) {
        const { machineMac, address, comPort, ubaSN, ubaChannel, actionResult, action, fwVersion, hwVersion } = pendingTask.pendingUbaDevice;
        if (!machineMac || !address || !comPort || !action || !actionResult) {
            throw new Error(`Missing required parameters.`);
        }
        if(actionResult === ACTION_RESULT.SUCCESS.code && (!ubaSN || !ubaChannel)) {
            throw new Error(`Missing parameters for SUCCESS uba device operation.`);
        }
        const key = generateConnectionUbaDeviceKey(address, comPort, action);
        const waiter = getWaiter(machineMac, PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE, key);
        if(!waiter) {
            logger.info(`No waiter found for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
            return;
        }
        if(waiter.body.machineMac !== machineMac || waiter.body.address !== address || waiter.body.comPort !== comPort) {
            logger.warn(`THIS IS NOT EXPECTED!!! machineMac, address or comPort mismatch, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
            waiter.releaseFn( { errMsg: "machineMac, address or comPort mismatch" } );
            return;
        }

        if(actionResult !== ACTION_RESULT.SUCCESS.code) {
            logger.info(`Action result is not SUCCESS, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key} and actionResult ${actionResult}`);
            waiter.releaseFn( { errMsg: getActionResultByCode(actionResult) } );
            return;
        }

        //this is in edit mode, if the ibaSN returned isnt the same like the one that was sent to query, then it is a mismatch
        if(waiter.body.ubaSN && waiter.body.ubaSN !== ubaSN) {
            logger.info(`ubaSN mismatch, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
            waiter.releaseFn( { errMsg: "ubaSN mismatch. ubaDevice with these parameters(machineMac, address, comPort) has a different ubaSN" } );
            return;
        }
        //this is in edit mode, if the ubaChannel returned isnt the same like the one that was sent to query, then it is a mismatch
        if(waiter.body.ubaChannel && waiter.body.ubaChannel !== ubaChannel) {
            logger.warn(`THIS IS NOT EXPECTED!!! ubaChannel mismatch, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
            waiter.releaseFn( { errMsg: "ubaChannel mismatch. ubaDevice with these parameters(machineMac, address, comPort) has a different ubaChannel" } );
            return;
        }

        if(waiter.body.action === UBA_DEVICE_ACTIONS.QUERY) {
             if(!fwVersion || !hwVersion) {
                throw new Error(`Missing versions required parameters.`);
            }
            const ubaDevice = await getUbaDeviceByUbaSN(ubaSN);
            if(waiter.body.uiFlow === UI_FLOWS.QUERY_ADD_UBA_DEVICE) {
                if(ubaDevice) {
                    logger.info(`ubaSN already exists, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
                    waiter.releaseFn( { errMsg: "ubaSN already exists. Should not add a new device with the same ubaSN" } );
                    return;
                }
            } else if(waiter.body.uiFlow === UI_FLOWS.QUERY_EDIT_UBA_DEVICE) {
                if(!ubaDevice) {
                    logger.warn(`THIS IS NOT EXPECTED!!! ubaSN does not exist, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
                    waiter.releaseFn({ errMsg: "ubaSN does not exist. Should not edit a device that is not in the system" });
                    return;
                }
            } else {
                logger.warn(`THIS IS NOT EXPECTED!!! uiFlow is not recognized, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
                waiter.releaseFn({ errMsg: "uiFlow is not recognized" });
                return;
            }
            const timePassed = Date.now() - waiter.body.requestStartTime;
            logger.info(`Releasing waiter for query pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key} pendingTask was created ${timePassed} ms ago`);
            waiter.releaseFn(pendingTask.pendingUbaDevice);
        } else if (waiter.body.action === UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST || waiter.body.action === UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST) {
            const timePassed = Date.now() - waiter.body.requestStartTime;
            logger.info(`Releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key} with action ${waiter.body.action} pendingTask was created ${timePassed} ms ago`);
            waiter.releaseFn(pendingTask.pendingUbaDevice);
        } else {
            logger.warn(`THIS IS NOT EXPECTED!!! action is not recognized, releasing waiter for pending task ${PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE} for machine ${machineMac} with key ${key}`);
            waiter.releaseFn({ errMsg: "action is not recognized" });
        }
    }
};

//being called from Web UI, doing Test for connection with UbaDevice
const queryUbaDevice = async (pendingUbaDevice) => {
	const { machineMac, address, comPort, ubaSN, ubaChannel} = pendingUbaDevice;
	if (!machineMac || !address || !comPort) {
		throw new Error(`Missing parameters.`);
	}
	
    const machine = await getMachine(machineMac);
    if(!machine) {
        throw new Error(`Machine with mac ${machineMac} does not exist.`);
    }
	const ubaDeviceByConstraint = await getUbaDeviceByConstraint(machineMac, address, comPort);

	if(!ubaSN) {
		//this is add pending uba device
		if (ubaDeviceByConstraint) {
			throw new Error(`ubaDeviceByConstraint already exists.`);
		}
		return await sendConnectionPendingTaskToUba( machineMac, address, comPort, undefined, undefined, undefined, UBA_DEVICE_ACTIONS.QUERY, UI_FLOWS.QUERY_ADD_UBA_DEVICE );
	} else {
		//this is edit pending uba device
		const ubaDeviceBySN = await getUbaDeviceByUbaSN(ubaSN);
		if (!ubaDeviceBySN) {
			throw new Error(`ubaSN does not exist.`);
		}
		if(ubaDeviceByConstraint && ubaDeviceByConstraint.ubaSN !== ubaSN) {
			throw new Error(`ubaSN already exists.`);
		}
		return await sendConnectionPendingTaskToUba( machineMac, address, comPort, ubaSN, ubaChannel, undefined, UBA_DEVICE_ACTIONS.QUERY, UI_FLOWS.QUERY_EDIT_UBA_DEVICE );
	}
};

module.exports = {
    getPendingTasks,
    pendingTasksExecuted,
    queryUbaDevice,
};
