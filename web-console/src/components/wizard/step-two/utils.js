import {changeTestType, testTypeData,} from 'src/actions/TestRoutines';

export const FIRST_COLUMN = 5;
export const SECOND_COLUMN = 7;

export const handleOnDrop = (event, testRoutinesDispatch, id) => {
	event.preventDefault();

	let newId = id + 0.1;
	if (id === -1) {
		newId = -1;
	}

	const data = event.dataTransfer.getData('text/plain');
	testRoutinesDispatch(changeTestType({
		previousId: JSON.parse(data).id,
		newId,
	}));
};

export const handleInputChange = (testRoutinesDispatch, id, dataKey, dataValue, isExclude = false) => {
	testRoutinesDispatch(testTypeData({
		id,
		dataKey,
		dataValue,
		isExclude,
	}));
};
