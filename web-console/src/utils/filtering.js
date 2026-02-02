import { isTestNeedToBeConfirmed, isTestRunning, statusCodes } from "../constants/unsystematic";
import { validateFunction, validateObject, validateString, } from "./validators";

const descendingComparator = (elementA, elementB, orderBy, prepareValue) => {
	const valueA = prepareValue(elementA[orderBy], orderBy);
	const valueB = prepareValue(elementB[orderBy], orderBy);

	if (valueA === null) {
		return 1;
	}

	if (valueB === null) {
		return -1;
	}

	if (valueB < valueA) {
		return -1;
	}

	if (valueB > valueA) {
		return 1;
	}

	return 0;
};

const getComparator = (order, orderBy, prepareValue) => {
	if (order === 'desc') {
		return (elementA, elementB) => descendingComparator(elementA, elementB, orderBy, prepareValue);
	}

	return (elementA, elementB) => -descendingComparator(elementA, elementB, orderBy, prepareValue);
};

export default function filtering(inputData, order, orderBy, filters, prepareValue, doFiltering) {
	const stabilizedThis = inputData.map((el, index) => [el, index]);

	if (validateString(order) && validateString(orderBy) && validateFunction(prepareValue)) {
		const comparator = getComparator(order, orderBy, prepareValue);
		stabilizedThis.sort((elementA, elementB) => {
			//showing first those with errors
			const hasErrorA = typeof elementA[0].error !== 'undefined' && elementA[0].error !== null && elementA[0].error !== 0 && elementA[0].status === statusCodes.ABORTED;
			const hasErrorB = typeof elementB[0].error !== 'undefined' && elementB[0].error !== null && elementB[0].error !== 0 && elementB[0].status === statusCodes.ABORTED;
			if (hasErrorA && !hasErrorB) {
			  	return -1;
			} else if (!hasErrorA && hasErrorB) {
			  	return 1;
			}
			//then showing those that are running only if order is by runtime
			if (orderBy === 'runtime') {
				const needTobeConfirmedA = isTestNeedToBeConfirmed(elementA[0].status);
				const needTobeConfirmedB = isTestNeedToBeConfirmed(elementB[0].status);
				if (needTobeConfirmedA && !needTobeConfirmedB) {
					return -1;
				} else if (!needTobeConfirmedA && needTobeConfirmedB) {
					return 1;
				}
				
				const isRunningA = isTestRunning(elementA[0].status);
				const isRunningB = isTestRunning(elementB[0].status);
				if (isRunningA && !isRunningB) {
					return -1;
				} else if (!isRunningA && isRunningB) {
					return 1;
				}
			}

			const result = comparator(elementA[0], elementB[0]);
			if (result !== 0) {
				return result;
			}
			return elementA[1] - elementB[1];
		});
	}

	let dataFiltered = stabilizedThis.map((el) => el[0]);
	dataFiltered = doFiltering(dataFiltered, filters);

	return dataFiltered;
}

export const simpleFilterData = (data, fieldName, fieldValue) => data.filter(item => item?.[fieldName] && item[fieldName].toLowerCase().indexOf(fieldValue?.toLowerCase()) !== -1);

export const doSimpleFiltering = (data, filters) => {
	//console.log('data', data,filters);
	let newData = [...data];

	if (!validateObject(filters)) {
		return newData;
	}

	for (const [key, value] of Object.entries(filters)) {
		if (validateString(value?.toString())) {
			newData = newData.filter(item => {
				let itemValue = item?.[key]?.toString().toLowerCase();
				if (!validateString(itemValue)) {
					itemValue = '';
				}

				const pattern = [value]?.toString().toLowerCase();

				return itemValue.indexOf(pattern) !== -1
			});
		}
	}

	return newData;
};

export const handleFilters = (name, value, setFilters, setPage) => {
	setPage(0);

	setFilters((prevState) => ({
		...prevState,
		[name]: value,
	}));
};

export const handleChangePage = (page, setPage) => setPage(page);

export const handleChangeRowsPerPage = (rows, setPage, setRowsPerPage) => {
	setPage(0);
	setRowsPerPage(parseInt(rows, 10));
};
