const Joi = require('joi');

const { testRoutineModel,  } = require('../models');
const logger = require('./logger');

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
	We check that each object has only the defined keys and that the values are of the correct type.
	it will also convert the values to the correct type for example '123' to 123.
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
			} else {
				logger.info(`Key ${key} isn't in plan step value = ${accumulator[key]}`);
			}
			return accumulator;
		}, {});

		if (validateObject(state, true)) {
			let convertedValue = validateSchema(Joi.object(testRoutineModel.planSchema), state, `validatePlan`);
			dataPlan.push(convertedValue);
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
	let testResultsProperties = [
        'timestamp',
        'voltage',
        'temperature',
        'current',
		'planIndex',
		'stepIndex',
        //'currentStep',
	];
	for (const testState of testResults) {
		let state = testResultsProperties.reduce((accumulator, key) => {
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

function validateSchema(joiSchema, data, context = '') {
	/*const joiOptions = {
		convert: true,      // default true
		abortEarly: true,   // stop after first error (default true)
		allowUnknown: false,// disallow unknown keys in objects
		stripUnknown: false,// remove unknown keys instead of erroring
		presence: 'optional', // can be 'required', 'optional', 'forbidden'
		noDefaults: false,  // skip applying defaults
		errors: { label: 'path' } // customize error messages
	};
	joiSchema.validate(data, { convert: false }) to avoid auto conversion. default joi will convert strings to numbers '42' → 42
	Also booleans 'true' → true, 'false' → false, '1' → true, '0' → false
	Also dates — '2025-09-21' → Date('2025-09-21')
	*/
	//currently convert is true by default and many calls to apis are sending strings instead of numbers. in the end the db will convert to the right type. 
	//but still need to change all the calls to send the right type. and use the value of joiSchema or to add convert:false and to be strict!
    const { value, error } = joiSchema.validate(data);
	
    if (error) {
        logger.warn(`validateSchema failed${context ? ' for ' + context : ''}`, error.details);
        throw new Error(error.details.map(e => e.message).join(', '));
    }
	return value;
}

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
    validateSchema,
};
