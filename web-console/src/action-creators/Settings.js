import {setNotification,} from 'src/actions/Auth';
import {setMachineSettings, setCellSettings,} from 'src/actions/Settings';
import {postData,} from 'src/utils/httpRequests';
import {handleRequestError,} from 'src/utils/helper';
import {validateArray,} from 'src/utils/validators';

export const getMachines = async (authDispatch, settingsDispatch) => {
	try {
		const response = await postData(authDispatch, 'machines', 'GET');
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
		const response = await postData(authDispatch, 'cells', 'GET');
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
		await postData(authDispatch, 'cells', 'POST', data);
		getCells(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const updateCell = async (authDispatch, settingsDispatch, data) => {
	try {
		await postData(authDispatch, `cells/${data.itemPN}`, 'PATCH', data);
		getCells(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const deleteCell = async (authDispatch, settingsDispatch, itemPN) => {
	try {
		await postData(authDispatch, `cells/${itemPN}`, 'DELETE');
		getCells(authDispatch, settingsDispatch);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
