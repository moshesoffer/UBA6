import {setNotification,} from 'src/actions/Auth';
import {setReports, setTestsGraph,} from 'src/actions/Reports';
import {postData,} from 'src/utils/httpRequests';
import {handleRequestError, } from 'src/utils/helper';
import {validateObject, validateArray,} from 'src/utils/validators';

import {getMachines} from './Settings';

export const getReports = async (authDispatch, reportsDispatch, settingsDispatch, metadata) => {
	try {
		getMachines(authDispatch, settingsDispatch);
		const response = await postData(authDispatch, 'reports/search', 'POST', metadata);
		if (!validateArray(response?.rows, false)) {
			throw new Error('Invalid response. reports is missing.');
		}

		reportsDispatch(setReports(response));
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const updateReport = async (authDispatch, reportsDispatch, settingsDispatch, metadata, report) => {
	try {
		await postData(authDispatch, `reports/${report.id}`, 'PATCH', {
			...report,
		});
		getReports(authDispatch, reportsDispatch, settingsDispatch, metadata);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}

export const getReportData = async (authDispatch, reportsDispatch, ids) => {
	try {
		const response = await postData(authDispatch, 'test-results/search', 'POST', ids);
		if (!validateArray(response, false)) {
			throw new Error('Invalid response. Graph Data is missing.');
		}
		reportsDispatch(setTestsGraph(response));
	} catch
		(error) {
		const preparedMessage = handleRequestError(error);
		authDispatch(setNotification({message: preparedMessage,}));
	}
}
