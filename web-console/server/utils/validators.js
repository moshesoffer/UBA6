const { testRoutineModel, reportsDataModel } = require('../models');

const validateIsDefined = value => (typeof value !== 'undefined');

const validateBoolean = booleanValue => (typeof booleanValue === 'boolean');

const validateString = (stringValue, isEmptyCheck = true) => {
	const result = typeof stringValue === 'string';

	if (isEmptyCheck) {
		return (result && Boolean(stringValue.length));
	}

	return result;
};

const validateNumber = numberValue => (typeof numberValue === 'number' && !isNaN(numberValue));

const validateInteger = integerValue => Number.isInteger(integerValue);

const validateObject = (objectValue, isEmptyCheck = false) => {
	const result = (typeof objectValue === 'object' && objectValue !== null);

	if (isEmptyCheck) {
		return (result && Boolean(Object.keys(objectValue).length));
	}

	return result;
};

const validateArray = (arrayValue, isEmptyCheck = true) => {
	const result = validateObject(arrayValue) && Array.isArray(arrayValue);

	if (isEmptyCheck) {
		return (result && Boolean(arrayValue.length));
	}

	return result;
};

const validateFunction = checkedFunction => (typeof checkedFunction === 'function');

const validateTimestamp = timestampValue => ((validateInteger(timestampValue) && timestampValue > 1571048279));

const validateDate = dateValue => {
	const result = Date.parse(dateValue)
	return !isNaN(result) && validateTimestamp(result);
}

const validateByRegExp = (stringValue, regExpObject) => {
	if (!validateIsDefined(stringValue)) {
		return false;
	}

	const stringParam = stringValue.toString();

	if (!stringParam.length) {
		return true;
	}

	if (!validateString(stringParam, false)) {
		return false;
	}

	return regExpObject.test(stringParam);
};

const validatePhone = phoneValue => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[\\d\\s\\-]+$|^$', 'u');
	return validateByRegExp(phoneValue, regExpObject);
};

const validateUbaSN = ubaSNValue => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[\\w-]+$|^$', 'u');
	return validateByRegExp(ubaSNValue, regExpObject);
};

const validateMacAddress = macAddressValue => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[\\w:-]+$', 'u');
	return validateByRegExp(macAddressValue, regExpObject);
};

const validateHumanName = name => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[a-z]{3,}$', 'ui');
	return validateByRegExp(name, regExpObject);
};

const validateEmail = email => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$', 'u');
	return validateByRegExp(email, regExpObject);
};

const validatePassword = password => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)\\S{8,}$', 'u');
	return validateByRegExp(password, regExpObject);
};

/*
	We need to handle the plan field separately, because it is an array of objects.
	It is stored as a JSON string in the DB.
	We check that the array is not empty.
	We check that each object has only the defined keys.
	See the testRoutineModel.planProperties constant.
*/
const validatePlan = (plan, mandatory) => {
	if (!validateArray(plan)) {
		if(mandatory) throw new Error(`Invalid plan.`);
		return;
	}

	let dataPlan = [];
	for (const testState of plan) {
		let state = testRoutineModel.planProperties.reduce((accumulator, key) => {
			if (key in testState) {
				accumulator[key] = testState[key];
			}
			return accumulator;
		}, {});

		if (validateObject(state, true)) {
			dataPlan.push(state);
		}
	}

	if (!validateArray(dataPlan)) {
		throw new Error(`Invalid plan.`);
	}

	return dataPlan;

};

const validateTestResults = (testResults, mandatory) => {
	if (!validateArray(testResults)) {
		if(mandatory) throw new Error(`Invalid TestResults.`);
		return;
	}

	let dataTestResults = [];
	for (const testState of testResults) {
		let state = reportsDataModel.testResultsProperties.reduce((accumulator, key) => {
			if (key in testState) {
				accumulator[key] = testState[key];
			}
			return accumulator;
		}, {});

		if (validateObject(state, true)) {
			dataTestResults.push(state);
		}
	}

	if (!validateArray(dataTestResults)) {
		throw new Error(`Invalid TestResults..`);
	}

	return dataTestResults;

};

module.exports = {
	validateIsDefined,
	validateBoolean,
	validateString,
	validateNumber,
	validateInteger,
	validateObject,
	validateArray,
	validateFunction,
	validateTimestamp,
	validateDate,
	validatePlan,
	validateTestResults,
};
