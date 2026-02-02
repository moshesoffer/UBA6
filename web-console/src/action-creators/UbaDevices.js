import {setNotification,} from 'src/actions/Auth';
import {setUbaDevices, setUbaTotal,} from 'src/actions/UbaDevices';
import {postData,} from 'src/utils/httpRequests';
import {handleRequestError,} from 'src/utils/helper';
import {validateArray, validateObject,} from 'src/utils/validators';

export const getUbaDevices = async (authDispatch, ubaDevicesDispatch, hideLoading) => {
	try {
		const response = await postData(hideLoading ? null : authDispatch, 'uba-devices', 'GET');
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
		await postData(authDispatch, 'uba-devices', 'POST', data);
		getUbaDevices(authDispatch, ubaDevicesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const updateUbaDevice = async (authDispatch, ubaDevicesDispatch, data) => {
	try {
		await postData(authDispatch, `uba-devices/${data.ubaSN}`, 'PATCH', data);
		getUbaDevices(authDispatch, ubaDevicesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const deleteUbaDevice = async (authDispatch, ubaDevicesDispatch, ubaSN) => {
	try {
		await postData(authDispatch, `uba-devices/${ubaSN}`, 'DELETE');
		getUbaDevices(authDispatch, ubaDevicesDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
