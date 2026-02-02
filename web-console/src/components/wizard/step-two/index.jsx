import {useState, useRef,} from 'react';
import {useLocation,} from 'react-router-dom';

import Box from '@mui/material/Box';
import Container from '@mui/material/Container';
import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import CardActions from '@mui/material/CardActions';
import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import {getText,} from 'src/services/string-definitions';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevices, useUbaDevicesDispatch} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {setCurrentUba,setSelectedDevices} from '/src/actions/UbaDevices';
import {startSaveAction,} from 'src/actions/TestRoutines';
import {createTestRoutine, updateTestRoutine, createRunningTest,} from 'src/action-creators/TestRoutines';
import {pageStateList, testTypeNames, ubaChannel, UBA_CHANNEL_LIST,getSelectedCardsOrTableView} from 'src/constants/unsystematic';
import {validateBoolean, validateString,} from 'src/utils/validators';
import {resetTestParameters,} from 'src/utils/helper';

import Operations from './Operations';
import Tests from './Tests';
import ParametersChangedDialog from './modal-views/ParametersChangedDialog';
import {
	validateMaxTime,
	validateDelayTime,
	validateWaitTemp,
	validateMinTemp,
	validateMaxTemp,
	validateChargeLimit,
	validateDischargeLimit,
	validateChargeCurrent,
	validateDischargeCurrent,
	validateChargePerCell,
	validateCutOffCurrent,
	validateCutOffVoltage,
	validateGoToStep,
	validateRepeatStep,
	validateCheckboxGroup,
} from '../validators';
import {changePageState,} from '../utils';

export default function WizardTwo() {

	const {pathname} = useLocation();
	let isRunAction = useRef(false);

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const {currentUba,} = useUbaDevices();
	const {testData, plan, existingTest,} = useTestRoutines();

	const [isDialogOpen, setIsDialogOpen] = useState(false);
	const [isError, setIsError] = useState(false);

	const validateChargeData = data => {
		const validateCheckboxGroupResult = validateCheckboxGroup(data.isMaxTime, data.isCutOffCurrent, data.isChargeLimit);
		const validateMinTempResult = validateMinTemp(data.isMinTemp, data.minTemp, null);
		const validateChargeCurrentResult = validateChargeCurrent(data.chargeCurrent, null);
		const validateChargePerCellResult = validateChargePerCell(data.chargePerCell, null);
		const validateMaxTimeResult = validateMaxTime(data.isMaxTime, data.maxTime, null);
		const validateMaxTempResult = validateMaxTemp(data.isMaxTemp, data.maxTemp, null);
		const validateChargeLimitResult = validateChargeLimit(data.isChargeLimit, data.chargeLimit, null);
		const validateCutOffCurrentResult = validateCutOffCurrent(data.isCutOffCurrent, data.cutOffCurrent, null);

		return validateCheckboxGroupResult &&
			validateMinTempResult &&
			validateChargeCurrentResult &&
			validateChargePerCellResult &&
			validateMaxTimeResult &&
			validateMaxTempResult &&
			validateChargeLimitResult &&
			validateCutOffCurrentResult;
	};

	const validateDischargeData = data => {
		const validateCheckboxGroupResult = validateCheckboxGroup(data.isMaxTime, data.isCutOffVoltage, data.isDischargeLimit);
		const validateMinTempResult = validateMinTemp(data.isMinTemp, data.minTemp, null);
		const validateDischargeCurrentResult = validateDischargeCurrent(data.dischargeCurrent, null);
		const validateMaxTimeResult = validateMaxTime(data.isMaxTime, data.maxTime, null);
		const validateMaxTempResult = validateMaxTemp(data.isMaxTemp, data.maxTemp, null);
		const validateDischargeLimitResult = validateDischargeLimit(data.isDischargeLimit, data.dischargeLimit, null);
		const validateCutOffVoltageResult = validateCutOffVoltage(data.isCutOffVoltage, data.cutOffVoltage, null);

		return validateCheckboxGroupResult &&
			validateMinTempResult &&
			validateDischargeCurrentResult &&
			validateMaxTimeResult &&
			validateMaxTempResult &&
			validateDischargeLimitResult &&
			validateCutOffVoltageResult;
	};

	const validateDelayData = data => {
		if(data.delayTime === null && data.waitTemp === null) {
			return false;
		}
		let validateDelayTimeResult = true;
		let validateWaitTempResult = true;

		if(data.delayTime != null) validateDelayTimeResult = validateDelayTime(data.delayTime, null);
		if(data.waitTemp != null) validateWaitTempResult = validateWaitTemp(data.waitTemp, null);

		return validateDelayTimeResult && validateWaitTempResult;
	};

	const validateLoopData = data => {
		const validateGoToStepResult = validateGoToStep(data.id, data.goToStep, null);
		const validateRepeatStepResult = validateRepeatStep(data.repeatStep, null);

		return validateGoToStepResult && validateRepeatStepResult;
	};

	const validateTestTypeData = testType => {
		switch (testType.type) {
			case testTypeNames.CHARGE:
				return validateChargeData(testType);
			case testTypeNames.DISCHARGE:
				return validateDischargeData(testType);
			case testTypeNames.DELAY:
				return validateDelayData(testType);
			case testTypeNames.LOOP:
				return validateLoopData(testType);
			default:
				return (
					<Box>
						Unknown test type
					</Box>
				);
		}
	}

	const doFinish = isButton => {
		testRoutinesDispatch(startSaveAction());

		if (!plan.length) {
			setIsError(true);
			return false
		}

		for (const testType of plan) {
			const validationResult = validateTestTypeData(testType);
			if (!validationResult) {
				setIsError(true);
				return false;
			}
		}

		setIsError(false);

		if (
			isButton &&
			validateBoolean(existingTest.isTrue) && existingTest.isTrue &&
			validateBoolean(existingTest.isChanged) && existingTest.isChanged
		) {
			// This flow has triggered by clicking the button and not from answering into dialog.
			// In this case, we are opening the dialog if the test changes has been recognized.
			// Also, we are interrupting the process here.
			setIsDialogOpen(true);
			return false;
		}

		const testRoutine = {
			...testData,
			testName: testData.testName.trim(),
			plan,
		};

		if (isRunAction.current) {
			// Running the test.
			let ubaSNs = [];

			if (
				(testRoutine.channel === ubaChannel.A || testRoutine.channel === ubaChannel.B) &&
				(currentUba.ubaChannel === testRoutine.channel || currentUba.ubaChannel === ubaChannel.AB)
			) {
				ubaSNs = [
					{
						ubaSN: currentUba.ubaSN,
						channel: currentUba.channel,
					},
				];
			} else if (
				testRoutine.channel === UBA_CHANNEL_LIST.A_AND_B &&
				currentUba.ubaChannel === ubaChannel.AB
			) {
				ubaSNs = [
					{
						ubaSN: currentUba.ubaSN,
						channel: ubaChannel.A,
					},
					{
						ubaSN: currentUba.ubaSN,
						channel: ubaChannel.B,
					},
				];
			} else {
				throw new Error(`Insufficient testRoutine channel ${testRoutine.channel} with currentUba channel ${currentUba.ubaChannel}`);
			}

			createRunningTest(authDispatch, ubaDevicesDispatch, ubaSNs, testRoutine);
		} else {
			// Saving the test routine.
			if (testRoutine.id === null) {
				createTestRoutine(authDispatch, testRoutinesDispatch, testRoutine);
			} else {
				updateTestRoutine(authDispatch, testRoutinesDispatch, testRoutine);
			}
		}

		return true;
	}

	const finishProcess = isButton => {
		const isProcessFinished = doFinish(isButton);
		if (!isProcessFinished) {
			// The process had not been finished because of the dialog had been opened.
			return;
		}

		resetTestParameters(testRoutinesDispatch);
		ubaDevicesDispatch(setCurrentUba({}));
		//ubaDevicesDispatch(setSelectedDevices([]));//not sure if this is needed
		handleStateChange(getSelectedCardsOrTableView());
	}

	const handleRunClick = () => {
		isRunAction.current = true;
		finishProcess(true);
	}

	const handleSaveClick = () => {
		isRunAction.current = false;
		finishProcess(true);
	}

	const handleDialogClose = answer => {
		setIsDialogOpen(false);

		if (!answer) {
			// The dialog answer is 'No';
			return;
		}

		finishProcess(false);
	}

	const handleStateChange = state => changePageState(state, pathname, ubaDevicesDispatch, testRoutinesDispatch);

	return (
		<Container maxWidth="false">
			<Stack direction="row" spacing={2}>
				<Operations/>
				<Tests/>
			</Stack>

			<Card sx={{mt: 1, px: 4, py: 1,}}>
				<CardActions sx={{justifyContent: 'space-between'}}>
					<Button variant="contained" color="primary" sx={{width: 140}} onClick={() => handleStateChange(pageStateList.WIZARD_ONE)}>
						{getText('common.BACK')}
					</Button>

					<Stack alignItems="center">
						{validateString(currentUba?.channel) ? (
							<Button variant="contained" sx={{width: 140}} onClick={handleRunClick}>
								{getText('mainPage.wizardTwo.RUN')}
							</Button>
						) : (
							<Button variant="contained" sx={{width: 140}} onClick={handleSaveClick}>
								{getText('common.SAVE')}
							</Button>
						)}
						<Typography align="center" sx={{color: 'red', display: isError ? 'block' : 'none',}}>
							{getText('mainPage.wizardTwo.FIX_ALL_ERRORS')}
						</Typography>
					</Stack>
				</CardActions>
			</Card>

			<ParametersChangedDialog open={isDialogOpen} onClose={handleDialogClose}/>
		</Container>
	);
}
