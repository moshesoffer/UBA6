import { category, statusCodes, isTestRunning } from 'src/constants/unsystematic';
import { simpleFilterData, } from 'src/utils/filtering';
import { validateString, } from 'src/utils/validators';

export const prepareValue = value => value;

export const doFiltering = (data, filters) => {
	const {ubaSN, batteryPN, status, machineName,} = filters;
	let newData = [...data];

	if (validateString(ubaSN)) {
		newData = simpleFilterData(newData, 'ubaSN', ubaSN);
	}

	if (validateString(batteryPN)) {
		newData = simpleFilterData(newData, 'batteryPN', batteryPN);
	}

	if (validateString(status)) {
		newData = newData.filter(item => {
			switch (status) {
				case category.RUNNING: {
					return isTestRunning(item.status);
				}
				case category.STANDBY: {
					return item.status!==statusCodes.RUNNING;
				}
				default:
					// eslint-disable-next-line no-console
					console.info('Unknown status value');
					return true;
			}
		});
	}

	if (validateString(machineName)) {
		newData = newData.filter(item => item?.machineName?.toLowerCase() === machineName?.toLowerCase());
	}

	return newData;
};
