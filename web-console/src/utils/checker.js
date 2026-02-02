import {getText,} from 'src/services/string-definitions';
import {validateInteger, validateNumber, validateString, validateFunction,} from 'src/utils/validators';

export const checkNumber = (incomingValue, mayNegative, incomingValueTitle, setIncomingValueError) => {
	const errorMessage = `${getText(incomingValueTitle)} ${getText('mainPage.wizardTwo.MUST_BE_NUMBER')}`;

	if (!validateString(incomingValue?.toString()) || !validateString(incomingValue?.toString().trim())) {
		if (validateFunction(setIncomingValueError)) {
			setIncomingValueError(errorMessage);
		}
		return false;
	}

	const numberValue = Number(incomingValue);

	if (!validateNumber(numberValue) || (!mayNegative && numberValue < 0)) {
		if (validateFunction(setIncomingValueError)) {
			setIncomingValueError(errorMessage);
		}
		return false;
	}

	if (validateFunction(setIncomingValueError)) {
		setIncomingValueError('');
	}
	return true;
};

export const checkInteger = (incomingValue, incomingValueTitle, setIncomingValueError) => {
	const errorMessage = `${getText(incomingValueTitle)} ${getText('mainPage.wizardTwo.MUST_BE_INTEGER')}`;

	const integerValue = parseInt(incomingValue, 10);

	if (
		!validateInteger(integerValue) ||
		integerValue < 0 ||
		(incomingValue?.toString() !== integerValue.toString())
	) {
		if (validateFunction(setIncomingValueError)) {
			setIncomingValueError(errorMessage);
		}
		return false;
	}

	if (validateFunction(setIncomingValueError)) {
		setIncomingValueError('');
	}
	return true;
};

export const checkString = (incomingValue, incomingValueTitle, setIncomingValueError) => {
	const errorMessage = `${getText(incomingValueTitle)} ${getText('mainPage.wizardOne.NOT_BE_EMPTY')}`;

	if (!validateString(incomingValue) || !validateString(incomingValue.trim())) {
		setIncomingValueError(errorMessage);
		return false;
	}

	setIncomingValueError('');
	return true;
};
