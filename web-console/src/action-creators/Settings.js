import {setNotification,} from 'src/actions/Auth';
import {setMachineSettings, setCellSettings,} from 'src/actions/Settings';
import {postData,postData2} from 'src/utils/httpRequests';
import {handleRequestError,} from 'src/utils/helper';
import {validateArray,} from 'src/utils/validators';

/* async with timeout */
const AWAIT_TIMEOUT = 5000;
function withTimeout(promise, ms) {
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
		console.log(`timeout, err: ${err}`);
        clearTimeout(timer);
        reject(err);
      });
  });
}

export const getMachines = async (authDispatch, settingsDispatch) => {
	try {
		const response = await withTimeout( postData(authDispatch, 'machines', 'GET'), AWAIT_TIMEOUT);
		if (!validateArray(response, false)) {
			throw new Error('Invalid response. Machine Settings is missing.');
		}

		settingsDispatch(setMachineSettings(response));
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const getCells = async (authDispatch, settingsDispatch) => {
	try {
		const response = await withTimeout( postData(authDispatch, 'cells', 'GET'), AWAIT_TIMEOUT);
		if (!validateArray(response, false)) {
			throw new Error('Invalid response. Cell Settings is missing.');
		}

		settingsDispatch(setCellSettings(response));
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const createCell = async (authDispatch, settingsDispatch, data) => {
	try {
		await withTimeout( postData(authDispatch, 'cells', 'POST', data), AWAIT_TIMEOUT);
		getCells(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const updateCell = async (authDispatch, settingsDispatch, data) => {
	try {
		await withTimeout( postData(authDispatch, `cells/${data.itemPN}`, 'PATCH', data), AWAIT_TIMEOUT);
		getCells(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const deleteCell = async (authDispatch, settingsDispatch, itemPN) => {
	try {
		await withTimeout( postData(authDispatch, `cells/${itemPN}`, 'DELETE'), AWAIT_TIMEOUT);
		getCells(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const deleteMachine = async (authDispatch, settingsDispatch, machineMac) => {
	try {
		const res = await withTimeout( postData2(authDispatch, `machines/${machineMac}`, 'DELETE'), AWAIT_TIMEOUT);
		if(res?.error) {
			authDispatch(setNotification({message: res.error,}));
			return;
		}
		getMachines(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
