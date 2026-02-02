import {getText} from 'src/services/string-definitions';
import {validateNumber, validateString, validateFunction,} from 'src/utils/validators';
import {checkString, checkNumber, checkInteger,} from 'src/utils/checker';
import {getInteger,} from 'src/utils/helper';

const getDataValue = dataParameter => {
	const parts = dataParameter?.split(':');
	if (validateString(parts?.[0])) {
		return parts[0];
	}

	return '';
}

const validateTime = (incomingValue, incomingValueTitle, setIncomingValueError) => {
	const errorMessage = `${getText(incomingValueTitle)} ${getText('mainPage.wizardTwo.MUST_BE_TIME')}`;

	const parts = incomingValue?.split(':');
	if (parts?.length !== 3) {
		if (validateFunction(setIncomingValueError)) {
			setIncomingValueError(errorMessage);
		}
		return false;
	}

	let [hours, minutes, seconds,] = parts;
	if (!/^\d{2,4}$/u.test(hours) || !/^\d{2}$/u.test(minutes) || !/^\d{2}$/u.test(seconds)) {
		if (validateFunction(setIncomingValueError)) {
			setIncomingValueError(errorMessage);
		}
		return false;
	}

	hours = parseInt(hours, 10);
	minutes = parseInt(minutes, 10);
	seconds = parseInt(seconds, 10);
	if (hours < 0 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59) {
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

export const validateTestName = (testName, testRoutines, id, setTestNameError) => {
	const result = checkString(testName, 'mainPage.wizardOne.TEST_PLAN_NAME', setTestNameError);
	if (!result) {
		return false;
	}

	const testNameList = testRoutines.map(testRoutine => testRoutine.testName);
	if (!testNameList.includes(testName.trim())) {
		return true;
	}

	if (!validateString(id?.toString())) {
		// This is a new test, so we can't have a test with the same name.
		setTestNameError(`${getText('mainPage.wizardOne.TEST_PLAN_NAME')} ${getText('mainPage.wizardOne.ALREADY_EXISTS')}`);
		return false;
	}

	const currentTest = testRoutines.find(testRoutine => testRoutine?.id?.toString() === id?.toString());
	const currentTestName = currentTest?.testName;
	if (currentTestName === testName.trim()) {
		// This is not a new test.
		// The name is the same as the current one, so it's fine.
		return true;
	}

	// This isn't a new test.
	// The test name already exists.
	// The name is different from the current one.
	setTestNameError(`${getText('mainPage.wizardOne.TEST_PLAN_NAME')} ${getText('mainPage.wizardOne.ALREADY_EXISTS')}`);
	return false;
}

export const validateBatteryPN = (batteryPN, setBatteryPNError) => checkString(batteryPN, 'testEditor.BATTERY_P_N', setBatteryPNError);

export const validateBatterySN = (batterySN, setBatterySNError) => checkString(batterySN, 'reportsPage.BATTERY_S_N', setBatterySNError);

export const validateCellPN = (cellPN, setCellPNError) => checkString(cellPN, 'testEditor.CELL_P_N', setCellPNError);

export const validateCellsInSerial = (noCellSerial, setCellsInSerialError) => checkInteger(noCellSerial, 'testEditor.NO_CELLS_IN_SERIAL', setCellsInSerialError);

export const validateCellsInParallel = (noCellParallel, setCellsInParallelError) => checkInteger(noCellParallel, 'testEditor.NO_CELLS_IN_PARALLEL', setCellsInParallelError);

export const validateMaxTime = (isMaxTime, maxTime, setMaxTimeError) => {
	if (!isMaxTime) {
		if (validateFunction(setMaxTimeError)) {
			setMaxTimeError('');
		}
		return true;
	}

	return validateTime(maxTime, 'mainPage.wizardTwo.MAX_TIME', setMaxTimeError);
};

export const validateDelayTime = (delayTime, setDelayTimeError) => validateTime(delayTime, 'mainPage.wizardTwo.DELAY_TIME', setDelayTimeError);

export const validateWaitTemp = (waitTemp, setWaitTempError) => checkNumber(waitTemp, false, 'mainPage.wizardTwo.WAIT_TEMPERATURE', setWaitTempError);

export const validateMinTemp = (isMinTemp, minTemp, setMinTempError) => {
	if (!isMinTemp) {
		if (validateFunction(setMinTempError)) {
			setMinTempError('');
		}
		return true;
	}

	return checkNumber(minTemp, true, 'mainPage.wizardTwo.MIN_TEMPERATURE', setMinTempError);
}

export const validateMaxTemp = (isMaxTemp, maxTemp, setMaxTempError) => {
	if (!isMaxTemp) {
		if (validateFunction(setMaxTempError)) {
			setMaxTempError('');
		}
		return true;
	}

	return checkNumber(maxTemp, false, 'mainPage.wizardTwo.MAX_TEMPERATURE', setMaxTempError);
}

export const validateChargeLimit = (isChargeLimit, chargeLimitSet, setChargeLimitError, ratedBatteryCapacity) => {
	if (!isChargeLimit) {
		if (validateFunction(setChargeLimitError)) {
			setChargeLimitError('');
		}
		return true;
	}

	const chargeLimit = getDataValue(chargeLimitSet);
	const result = checkNumber(chargeLimit, false, 'mainPage.wizardTwo.CHARGE_LIMIT', setChargeLimitError);

	const chargeLimitValue = parseFloat(chargeLimit?.toString());
	if (validateNumber(ratedBatteryCapacity) && validateNumber(chargeLimitValue) && chargeLimitValue > ratedBatteryCapacity) {
		setChargeLimitError(getText('mainPage.wizardTwo.TOO_HIGH_CAPACITY'));
	}

	return result;
};

export const validateDischargeLimit = (isDischargeLimit, dischargeLimitSet, setDischargeLimitError) => {
	if (!isDischargeLimit) {
		if (validateFunction(setDischargeLimitError)) {
			setDischargeLimitError('');
		}
		return true;
	}

	const dischargeLimit = getDataValue(dischargeLimitSet);
	return checkNumber(dischargeLimit, false, 'mainPage.wizardTwo.DISCHARGE_LIMIT', setDischargeLimitError);
};

export const validateChargeCurrent = (chargeCurrentSet, setChargeCurrentError) => {
	const chargeCurrent = getDataValue(chargeCurrentSet);
	return checkNumber(chargeCurrent, false, 'mainPage.wizardTwo.CHARGE_CURRENT', setChargeCurrentError);
};

export const validateDischargeCurrent = (dischargeCurrentSet, setDischargeCurrentError) => {
	const dischargeCurrent = getDataValue(dischargeCurrentSet);
	return checkNumber(dischargeCurrent, false, 'mainPage.wizardTwo.DISCHARGE_CURRENT', setDischargeCurrentError);
};

export const validateChargePerCell = (chargePerCell, setChargePerCellError, maxVoltage) => {
	const result = checkNumber(chargePerCell, false, 'mainPage.wizardTwo.CHARGE_PER_CELL', setChargePerCellError);

	const chargePerCellValue = parseFloat(chargePerCell?.toString());
	if (validateNumber(maxVoltage) && validateNumber(chargePerCellValue) && chargePerCellValue > maxVoltage) {
		setChargePerCellError(getText('mainPage.wizardTwo.OVER_VOLTAGE'));
	}

	return result;
}

export const validateCutOffCurrent = (isCutOffCurrent, cutOffCurrentSet, setCutOffCurrentError) => {
	if (!isCutOffCurrent) {
		if (validateFunction(setCutOffCurrentError)) {
			setCutOffCurrentError('');
		}
		return true;
	}

	const cutOffCurrent = getDataValue(cutOffCurrentSet);
	return checkNumber(cutOffCurrent, false, 'mainPage.wizardTwo.CUT_OFF_CURRENT', setCutOffCurrentError);
};

export const validateCutOffVoltage = (isCutOffVoltage, cutOffVoltage, setCutOffVoltageError, minVoltage) => {
	if (!isCutOffVoltage) {
		if (validateFunction(setCutOffVoltageError)) {
			setCutOffVoltageError('');
		}
		return true;
	}

	const result = checkNumber(cutOffVoltage, false, 'mainPage.wizardTwo.CUT_OFF_VOLTAGE', setCutOffVoltageError);

	const cutOffVoltageValue = parseFloat(cutOffVoltage?.toString());
	if (validateNumber(minVoltage) && validateNumber(cutOffVoltageValue) && cutOffVoltageValue < minVoltage) {
		setCutOffVoltageError(getText('mainPage.wizardTwo.UNDER_VOLTAGE'));
	}

	return result;
};

export const validateGoToStep = (id, goToStep, setGoToStepError) => {
	const result = checkInteger(goToStep, 'mainPage.wizardTwo.GO_TO_STEP', setGoToStepError);
	if (!result) {
		return false;
	}

	const realValue = getInteger(goToStep);
	if (realValue >= id) {
		const errorMessage = `${getText('mainPage.wizardTwo.GO_TO_STEP')} ${getText('mainPage.wizardTwo.MUST_BE_STEP', id)}`;
		if (validateFunction(setGoToStepError)) {
			setGoToStepError(errorMessage);
		}
		return false;
	}

	if (validateFunction(setGoToStepError)) {
		setGoToStepError('');
	}
	return true;
}

export const validateRepeatStep = (repeatStep, setRepeatStepError) => checkInteger(repeatStep, 'mainPage.wizardTwo.REPEAT_STEP', setRepeatStepError);

export const validateCheckboxGroup = (first, second, third, setFirstError, setSecondError, setThirdError) => {
	const errorMessage = getText('mainPage.wizardTwo.AT_LEAST_ONE');

	if (!first && !second && !third) {
		if (validateFunction(setFirstError)) {
			setFirstError(errorMessage);
		}
		if (validateFunction(setSecondError)) {
			setSecondError(errorMessage);
		}
		if (validateFunction(setThirdError)) {
			setThirdError(errorMessage);
		}

		return false;
	}

	if (validateFunction(setFirstError)) {
		setFirstError('');
	}
	if (validateFunction(setSecondError)) {
		setSecondError('');
	}
	if (validateFunction(setThirdError)) {
		setThirdError('');
	}

	return true;
}
