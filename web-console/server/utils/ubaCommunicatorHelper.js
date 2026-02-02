const logger = require('../utils/logger');

//this map holds waiters for pending tasks per machineMac and per PENDING_TASKS_TYPES waitersPerMachineMac[machineMac][PENDING_TASKS_TYPES][key]
const waitersPerMachineMac = new Map();

const UI_FLOWS = {
  QUERY_ADD_UBA_DEVICE: 'query_add_uba_device',
  QUERY_EDIT_UBA_DEVICE: 'query_edit_uba_device',
  ADD_UBA_DEVICE: 'add_uba_device',
  EDIT_UBA_DEVICE: 'edit_uba_device',
  DELETE_UBA_DEVICE: 'delete_uba_device',
};

const UBA_DEVICE_ACTIONS = {
  QUERY: 'query',
  ADD_TO_WATCH_LIST: 'addToWatchList',
  REMOVE_FROM_WATCH_LIST: 'removeFromWatchList',
};

const PENDING_TASKS_TYPES = {
  CONNECTION_UBA_DEVICE: 'connectionUbaDevice',
};

const ACTION_RESULT = {
  SUCCESS: { code: 'success', message: 'Operation was successful' },
  UBA_DEVICE_NOT_FOUND: { code: 'ubaDeviceNotFound', message: 'UBA Device not found' },
};

const getActionResultByCode = (code) => {
  for (const key in ACTION_RESULT) {
    if (ACTION_RESULT[key].code === code) {
      return ACTION_RESULT[key].message;
    }
  }
  return 'Unknown error';
}

const generateConnectionUbaDeviceKey = (address, comPort, action) => {
    return `${address}-${comPort}-${action}`;
};

const sendConnectionPendingTaskToUba = (machineMac, address, comPort, ubaSN, ubaChannel, name, action, uiFlow) => {
  const body = { machineMac, address, comPort, ubaSN, ubaChannel, name, action, uiFlow };
  const key = generateConnectionUbaDeviceKey(address, comPort, action);
  return sendPendingTaskToUba(machineMac, PENDING_TASKS_TYPES.CONNECTION_UBA_DEVICE, key, body);
};

const sendPendingTaskToUba = (machineMac, pendingTaskType, key, body, timeoutMs = 20_000, deleteOldRequest = true) => {

  if(deleteOldRequest) {
    const oldWaiter = getWaiter(machineMac, pendingTaskType, key);
		if (oldWaiter) {
      logger.debug(`Waiter for pending task ${pendingTaskType} for machine ${machineMac} with key ${key} already exists, releasing it`);
			oldWaiter.releaseFn( { errMsg: "waiter for pending task uba device already exists, multiple submits, this is an old request?" } );
		}
  }
  if (!waitersPerMachineMac.has(machineMac)) {
    waitersPerMachineMac.set(machineMac, new Map());
  }
  if(!waitersPerMachineMac.get(machineMac).has(pendingTaskType)) {
    waitersPerMachineMac.get(machineMac).set(pendingTaskType, new Map());
  }
  return new Promise((resolve, reject) => {
    const timeout = setTimeout(() => {
      logger.info(`timeout reached for pending task ${pendingTaskType} for machine ${machineMac}`);
      waitersPerMachineMac.get(machineMac)?.get(pendingTaskType)?.delete(key);
      reject(new Error('Timeout Reached'));
    }, timeoutMs);

    waitersPerMachineMac.get(machineMac).get(pendingTaskType).set(key,
        { 
            releaseFn: (msg) => {
              logger.debug(`releasing waiter for pending task ${pendingTaskType} for machine ${machineMac} with key ${key}`);
              clearTimeout(timeout);
              waitersPerMachineMac.get(machineMac)?.get(pendingTaskType)?.delete(key);
              if(msg?.errMsg && msg.errMsg !== 'Releasing all waiters') {
                logger.error(`Error in pending task ${pendingTaskType} for machine ${machineMac}: ${msg.errMsg}`);
                reject(new Error(msg.errMsg));
              } else {
                resolve(msg);
              }
              
            },
            body: {
              ...body,
              requestStartTime: Date.now(),
            }
        }
    );
  });
};

const getWaiter = (machineMac, pendingTaskType, key) => {
  return waitersPerMachineMac.get(machineMac)?.get(pendingTaskType)?.get(key);
};

const getAllWaitersPerMac = (machineMac) => {
  return waitersPerMachineMac.get(machineMac);
};

const releaseAllWaiters = () => {
  for (const [machineMac, pendingTasks] of waitersPerMachineMac.entries()) {
    for (const [pendingTaskType, waiters] of pendingTasks.entries()) {
      for (const [key, waiter] of waiters.entries()) {
        logger.debug(`**Releasing waiter for pending task ${pendingTaskType} for machine ${machineMac} with key ${key} [${JSON.stringify(waiter.body)}]`);
        waiter.releaseFn({ errMsg: 'Releasing all waiters' });
      }
      //waiters.clear();
    }
    //pendingTasks.clear();
  }
  //waitersPerMachineMac.clear();
  logger.info('All waiters released');
};

module.exports = {
  getWaiter,
  UI_FLOWS,
  getAllWaitersPerMac,
  UBA_DEVICE_ACTIONS,
  PENDING_TASKS_TYPES,
  generateConnectionUbaDeviceKey,
  ACTION_RESULT,
  getActionResultByCode,
  sendConnectionPendingTaskToUba,
  releaseAllWaiters,
};
