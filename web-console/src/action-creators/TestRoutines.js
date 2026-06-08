import { setNotification, } from 'src/actions/Auth';
import { setGraphData, setTestRoutines, } from 'src/actions/TestRoutines';
import { handleRequestError, prepareGraphData, } from 'src/utils/helper';
import { postData, } from 'src/utils/httpRequests';
import { validateArray, } from 'src/utils/validators';
import { statusCodes, } from 'src/constants/unsystematic';
import { getUbaDevices, } from './UbaDevices';

function incrementFileName(name) {
    const match = name.match(/^(?<base>.*?)(?:\.(?<num>\d+))?$/);

    if (!match) return `${name}.1`;

    const { base, num } = match.groups;

    return num
        ? `${base}.${Number(num) + 1}`
        : `${base}.1`;
}

/* async with timeout *
const AWAIT_TIMEOUT = 5000;
function promise, ms) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      reject(new Error(`Timeout after ${ms}ms`));
    }, ms);

    promise
      .then(result => {
        clearTimeout(timer);
        resolve(result);
      })



      .catch(err => {
		//console.log(`timeout, err: ${err}`);
        clearTimeout(timer);
        reject(err);
      });
  });
}
*/

export const getTestRoutines = async (authDispatch, testRoutinesDispatch) => {
	try {
		const response = await postData(authDispatch, 'test-routines', 'GET');
		if (!validateArray(response, false)) {
			throw new Error('Invalid response. testRoutines is missing.');
		}

		testRoutinesDispatch(setTestRoutines(response));
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const createTestRoutine = async (authDispatch, testRoutinesDispatch, data) => {
	/* eslint-disable no-await-in-loop */

	let maxRetries = 16;
	while (maxRetries > 0) {
	    try {
	        const result = await postData(authDispatch, 'test-routines', 'POST', data);

	        if (result.ok) {
	            return {
	                success: true,
	                data: result.data
	            };
	        }

	        if (result.status === 409) {
	            data.testName = incrementFileName(data.testName);
	        } else {
	            return {
	                success: false,
	                status: result.status,
	                error: result.message || "Server error"
	            };
	        }

	    } catch (error) {
	        data.testName = incrementFileName(data.testName);
	    }

	    maxRetries -= 1;
	}
}

export const updateTestRoutine = async (authDispatch, testRoutinesDispatch, data) => {
	try {
		await postData(authDispatch, `test-routines/${data.id}`, 'PATCH', data);
		getTestRoutines(authDispatch, testRoutinesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const deleteTestRoutine = async (authDispatch, testRoutinesDispatch, id) => {
	try {
		await postData(authDispatch, `test-routines/${id}`, 'DELETE');
		getTestRoutines(authDispatch, testRoutinesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const getGraphData = async (authDispatch, testRoutinesDispatch, runningTestID) => {
	try {
		const response = await postData(authDispatch, `instant-test-results/${runningTestID}`, 'GET'); //No timeout!
		if (!validateArray(response, false)) {
			throw new Error('Invalid response. Graph Data is missing.');
		}

		const graphData = prepareGraphData(response);
		testRoutinesDispatch(setGraphData(graphData));
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const createRunningTest = async (authDispatch, ubaDevicesDispatch, ubaSNs, data) => {
	const enrichedData = {
		...data,
		ubaSNs,
	};

	try {
		await postData(null, 'running-test', 'POST', enrichedData);
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const stopRunningTest = async (authDispatch, ubaDevicesDispatch, runningTestID, ubaSN, testRoutineChannels) => {
	const data = {
		runningTestID,
		testRoutineChannels,
		ubaSN,
		newTestStatus: statusCodes.PENDING_STOP
	};

	try {
		await postData(null, 'change-running-test-status', 'PATCH', data); //No timeout!
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
//console.log('==> stopRunningTest afrer post');
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const forceStopRunningTest = async (authDispatch, ubaDevicesDispatch, runningTestID, ubaSN, testRoutineChannels) => {
	const data = {
		runningTestID,
		testRoutineChannels,
		ubaSN,
		newTestStatus: statusCodes.STANDBY
	};

	try {
		await postData(null, 'change-running-test-status', 'PATCH', data); //No timeout!
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);

	} catch (error) {
	    const preparedMessage = handleRequestError(error);
	    authDispatch(setNotification({ message: preparedMessage }));
	}
}

export const pauseRunningTest = async (authDispatch, ubaDevicesDispatch, runningTestID, ubaSN, testRoutineChannels) => {
	const data = {
		runningTestID,
		testRoutineChannels,
		ubaSN,
		newTestStatus: statusCodes.PENDING_PAUSE
	};

	try {
		await postData(null, 'change-running-test-status', 'PATCH', data); //No timeout!
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const resumeRunningTest = async (authDispatch, ubaDevicesDispatch, runningTestID, ubaSN, testRoutineChannels) => {
	const data = {
		runningTestID,
		testRoutineChannels,
		ubaSN,
		newTestStatus: statusCodes.PENDING_RUNNING
	};

	try {
		await postData(null, 'change-running-test-status', 'PATCH', data); //No timeout!
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const confirmRunningTest = async (authDispatch, ubaDevicesDispatch, runningTestID, ubaSN, testRoutineChannels) => {
	const data = {
		runningTestID,
		testRoutineChannels,
		ubaSN,
		newTestStatus: statusCodes.PENDING_STANDBY
	};

	try {
		await postData(null, 'change-running-test-status', 'PATCH', data); //No timeout!
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);

		setTimeout(() => {
    		location.reload();
		}, 3000); // 1 seconds	} catch (error) {

	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const nextStepRunningTest = async (authDispatch, ubaDevicesDispatch, runningTestID, ubaSN, testRoutineChannels) => {
	const data = {
		runningTestID,
		testRoutineChannels,
		ubaSN,
		newTestStatus: statusCodes.PENDING_NEXTSTEP
	};

	try {
		await postData(null, 'change-running-test-status', 'PATCH', data); //No timeout!
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
