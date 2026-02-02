import { allTestData, existingTestParameters, setPlan, testEditParameters, } from 'src/actions/TestRoutines';
import { DUP, UBA_CHANNEL_LIST, unitVariants, isTestRunning} from 'src/constants/unsystematic';
import { initialTestRoutines, } from 'src/reducers/TestRoutines';
import { getText, } from 'src/services/string-definitions';
import { validateArray, validateNumber, validateObject, validateString, } from 'src/utils/validators';
import { dateFromUtc } from './dateTimeHelper';

export const areObjectsEqual = (obj1, obj2) => {
	return JSON.stringify(obj1) === JSON.stringify(obj2);
  }

export const handleRequestError = error => {
	// eslint-disable-next-line no-console
	console.info(error);

	if (validateString(error)) {
		return error;
	}

	if (validateString(error?.message)) {
		return error.message;
	}

	return 'An error occurred.';
};

export const getInteger = value => parseInt(value?.toString(), 10);

export const checkBitInPosition = (value, position) => ((value & (1 << position)) !== 0);//Deprecated

export const handleInputChange = (dispatch, action, dataKey, dataValue) => {
	dispatch(action({
		dataKey,
		dataValue,
	}));
};

export const getInputValue = (data, key) => {
	if (validateString(data?.[key]) || validateNumber(data?.[key])) {
		return data[key];
	}

	return '';
};

export const getNumberValue = (data, key) => {
	const numberValue = getInputValue(data, key);
	if (validateNumber(numberValue)) {
		return Number(numberValue.toFixed(2));
	}

	return null;
};

export const getUnitVariants = type => unitVariants[type].map(
	unit => ({
		parameter: unit.parameter,
		label: getText(unit.label),
	})
);

export const getUnit = type => {
	const typeElement = unitVariants[type];
	const [first,] = typeElement;
	return first.parameter;
};

export const printCelsius = value => {
	const degreeCelsiusSymbol = '\u2103';

	if (validateString(value)) {
		return `${value}${degreeCelsiusSymbol}`;
	}

	return '';
};

export const getMachines = ubaDevices => {
	const machines = ubaDevices
		.map((device => device.machineName))
		.filter(machineName => validateString(machineName));

	return [
		// Remove duplicates.
		...new Set(machines),
	];
};

const getUbaChannel = (channel, testRoutineNew) => {
	if (testRoutineNew?.channel === UBA_CHANNEL_LIST.A_AND_B) {
		// Existing ubaChannel is "A + B".
		return UBA_CHANNEL_LIST.A_AND_B;
	}

	if (validateString(channel)) {
		// We are in the main page. So, we are using the ubaChannel of the real UBA.
		return channel;
	}

	// We are in the test routine page. So, we are using "A or B".
	return UBA_CHANNEL_LIST.A_OR_B;
};

const loadTestRoutine = (testRoutine, testRoutinesDispatch) => {
	const testData = {
		...initialTestRoutines.testData,
	};
	if (validateObject(testRoutine)) {
		for (const key of Object.keys(testData)) {
			testData[key] = testRoutine[key];
		}
	}

	let plan = [];
	if (validateArray(testRoutine?.plan)) {
		({plan,} = testRoutine);
	}

	testRoutinesDispatch(allTestData(testData));
	testRoutinesDispatch(setPlan(plan));
};

export const resetTestParameters = testRoutinesDispatch => {
	testRoutinesDispatch(testEditParameters());
	loadTestRoutine(null, testRoutinesDispatch)
};

export const fillTestRoutine = (testRoutine, isDup, channel, testRoutinesDispatch) => {
	let testRoutineNew = {
		...testRoutine,
	};

	if (isDup) {
		testRoutineNew = {
			...testRoutine,
			id: null,
			testName: `${testRoutine.testName} ${DUP}`,
			batterySN: '',
			notes: '',
			customer: '',
			workOrderNumber: '',
			approvedBy: '',
			conductedBy: '',
			cellSupplier: '',
			cellBatch: '',
		}
	} else {
		// The purpose of this action is to inform us that we are dealing with an existing test.
		testRoutinesDispatch(existingTestParameters());
	}

	testRoutineNew.channel = getUbaChannel(channel, testRoutineNew);

	loadTestRoutine(testRoutineNew, testRoutinesDispatch);
};

export const prepareGraphData = data => data
	.map(item => ({
		timestamp: item.timestamp,
		dateTimeValue: dateFromUtc(item.timestamp),
		timePart: item.timestamp.split('T')[1].split('.')[0],
		voltage: parseFloat(item.voltage),
		current: parseFloat(item.current),
		capacity: parseFloat(item.capacity),
		temp: parseFloat(item.temp),
	}))
	.filter(item => item !== null);

export const isOtherChannelFree = (ubaDevices, currentUba) => {
	const sameUbaButWithDifferentChannel = ubaDevices.filter(uba => uba.ubaSN === currentUba.ubaSN && uba.channel !== currentUba.channel);
	const isAnotherChannelFree = (sameUbaButWithDifferentChannel && sameUbaButWithDifferentChannel.length > 0) &&
		!isTestRunning(sameUbaButWithDifferentChannel[0].status);
	return isAnotherChannelFree;
};

