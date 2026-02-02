export const validateIsDefined = value => (typeof value !== 'undefined');

export const validateBoolean = booleanValue => (typeof booleanValue === 'boolean');

export const validateString = (stringValue, isEmptyCheck = true) => {
	const result = typeof stringValue === 'string';

	if (isEmptyCheck) {
		return (result && Boolean(stringValue.length));
	}

	return result;
};

export const validateNumber = numberValue => (typeof numberValue === 'number' && !isNaN(numberValue));

export const validateInteger = integerValue => Number.isInteger(integerValue);

export const validateObject = (objectValue, isEmptyCheck = false) => {
	const result = (typeof objectValue === 'object' && objectValue !== null);

	if (isEmptyCheck) {
		return (result && Boolean(Object.keys(objectValue).length));
	}

	return result;
};

export const validateArray = (arrayValue, isEmptyCheck = true) => {
	const result = validateObject(arrayValue) && Array.isArray(arrayValue);

	if (isEmptyCheck) {
		return (result && Boolean(arrayValue.length));
	}

	return result;
};

export const validateFunction = checkedFunction => (typeof checkedFunction === 'function');

export const validateTimestamp = timestampValue => ((validateInteger(timestampValue) && timestampValue > 1571048279));

export const validateDate = dateValue => {
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

export const validatePhone = phoneValue => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[\\d\\s\\-]+$|^$', 'u');
	return validateByRegExp(phoneValue, regExpObject);
};

export const validateUbaSN = ubaSNValue => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[\\w-]+$|^$', 'u');
	return validateByRegExp(ubaSNValue, regExpObject);
};

export const validateMacAddress = macAddressValue => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[\\w:-]+$', 'u');
	return validateByRegExp(macAddressValue, regExpObject);
};

export const validateHumanName = name => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[a-z]{3,}$', 'ui');
	return validateByRegExp(name, regExpObject);
};

export const validateEmail = email => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$', 'u');
	return validateByRegExp(email, regExpObject);
};

export const validatePassword = password => {
	// eslint-disable-next-line prefer-regex-literals
	const regExpObject = RegExp('^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)\\S{8,}$', 'u');
	return validateByRegExp(password, regExpObject);
};
