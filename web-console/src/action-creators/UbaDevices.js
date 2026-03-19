import {setNotification,} from 'src/actions/Auth';
import {setUbaDevices, setUbaTotal,} from 'src/actions/UbaDevices';
import {postData,postData2,} from 'src/utils/httpRequests';
import {handleRequestError,} from 'src/utils/helper';
import {validateArray, validateObject,} from 'src/utils/validators';

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

export const getUbaDevices = async (authDispatch, ubaDevicesDispatch, hideLoading) => {
	try {
		const response = await withTimeout( postData(hideLoading ? null : authDispatch, 'uba-devices', 'GET'), AWAIT_TIMEOUT);
		if (!validateArray(response?.ubaDevices, false)) {
			throw new Error('Invalid response. ubaDevices is missing.');
		}
		if (!validateObject(response?.ubaTotal)) {
			throw new Error('Invalid response. ubaTotal is missing.');
		}

		ubaDevicesDispatch(setUbaDevices(response.ubaDevices));
		ubaDevicesDispatch(setUbaTotal(response.ubaTotal));
		authDispatch(setNotification({message: '',}));
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const createUbaDevice = async (authDispatch, ubaDevicesDispatch, data) => {
	try {
		await withTimeout( postData(authDispatch, 'uba-devices', 'POST', data), AWAIT_TIMEOUT);
		getUbaDevices(authDispatch, ubaDevicesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const queryUbaDevice = async (authDispatch, data) => {
	try {
		return await withTimeout( postData2(authDispatch, 'query-uba-devices', 'POST', data), AWAIT_TIMEOUT);
	} catch (error) {
		return handleRequestError(error);
	}
}

export const updateUbaDevice = async (authDispatch, ubaDevicesDispatch, data) => {
	try {
		await withTimeout( postData(authDispatch, `uba-devices/${data.ubaSN}`, 'PATCH', data), AWAIT_TIMEOUT);
		getUbaDevices(authDispatch, ubaDevicesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const deleteUbaDevice = async (authDispatch, ubaDevicesDispatch, ubaSN) => {
	try {
		await withTimeout( postData(authDispatch, `uba-devices/${ubaSN}`, 'DELETE'), AWAIT_TIMEOUT);
		getUbaDevices(authDispatch, ubaDevicesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
