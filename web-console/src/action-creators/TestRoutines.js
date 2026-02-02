import { setNotification, } from 'src/actions/Auth';
import { setGraphData, setTestRoutines, } from 'src/actions/TestRoutines';
import { handleRequestError, prepareGraphData, } from 'src/utils/helper';
import { postData, } from 'src/utils/httpRequests';
import { validateArray, } from 'src/utils/validators';
import { statusCodes, } from 'src/constants/unsystematic';
import { getUbaDevices, } from './UbaDevices';

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
	try {
		await postData(authDispatch, 'test-routines', 'POST', data);
		getTestRoutines(authDispatch, testRoutinesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
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
		const response = await postData(authDispatch, `instant-test-results/${runningTestID}`, 'GET');
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
		await postData(null, 'change-running-test-status', 'PATCH', data);
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
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
		await postData(null, 'change-running-test-status', 'PATCH', data);
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
		await postData(null, 'change-running-test-status', 'PATCH', data);
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
		await postData(null, 'change-running-test-status', 'PATCH', data);
		getUbaDevices(authDispatch, ubaDevicesDispatch, true);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
