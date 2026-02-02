import {useState, useEffect,} from 'react';

import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import Grid from '@mui/material/Grid';
import Typography from '@mui/material/Typography';
import TextField from '@mui/material/TextField';
import FormHelperText from '@mui/material/FormHelperText';
import FormControlLabel from '@mui/material/FormControlLabel';
import FormControl from "@mui/material/FormControl";
import Select from "@mui/material/Select";
import MenuItem from "@mui/material/MenuItem";
import Checkbox from '@mui/material/Checkbox';

import {getText,} from 'src/services/string-definitions';
import {useUbaDevices,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {validateString, validateObject,} from 'src/utils/validators';
import {chargeLimitParts, TITLE_WIDTH,SECOND_PART__WIDTH} from 'src/constants/unsystematic';
import {getUnitVariants,getInputValue} from 'src/utils/helper';
import {handleInputChange, FIRST_COLUMN, SECOND_COLUMN,} from '../../utils';
import {
	validateMaxTime,
	validateMaxTemp,
	validateDischargeLimit,
	validateCutOffVoltage,
	validateCheckboxGroup,
} from '../../../validators';

export default function LowerPart(props) {

	const {
		id,
		isMaxTime,
		maxTime,
		isMaxTemp,
		maxTemp,
		maxTempDefault,
		isDischargeLimit,
		dischargeLimit,
		isCutOffVoltage,
		cutOffVoltage,
		minVoltage,
	} = props;

	const [maxTimeError, setMaxTimeError] = useState('');
	const [maxTempWarning, setMaxTempWarning] = useState('');
	const [maxTempError, setMaxTempError] = useState('');
	const [dischargeLimitError, setDischargeLimitError] = useState('');
	const [cutOffVoltageError, setCutOffVoltageError] = useState('');

	const testRoutinesDispatch = useTestRoutinesDispatch();

	const {currentUba,} = useUbaDevices();
	const {testData, saveIsRun,} = useTestRoutines();

	useEffect(() => {
		if (!saveIsRun) {
			return;
		}

		runCommonValidation();
	}, [saveIsRun,]);

	const runCommonValidation = () => {
		validateCheckboxGroup(isMaxTime, isDischargeLimit, isCutOffVoltage, setMaxTimeError, setDischargeLimitError, setCutOffVoltageError());

		if (isMaxTime) {
			validateMaxTime(isMaxTime, maxTime, setMaxTimeError);
		}

		if (isMaxTemp) {
			validateMaxTemp(isMaxTemp, maxTemp, setMaxTempError);
		}

		if (isDischargeLimit) {
			validateDischargeLimit(isDischargeLimit, dischargeLimit, setDischargeLimitError);
		}

		if (isCutOffVoltage) {
			validateCutOffVoltage(isCutOffVoltage, cutOffVoltage, setCutOffVoltageError);
		}
	};

	const handleMaxTimeChange = dataValue => {
		const maxTime = dataValue.trim();
		validateMaxTime(isMaxTime, maxTime, setMaxTimeError);
		handleInputChange(testRoutinesDispatch, id, 'maxTime', maxTime);
	};

	const handleMaxTempChange = dataValue => {
		const maxTemp = dataValue.trim();
		const validateMaxTempResult = validateMaxTemp(isMaxTemp, maxTemp, setMaxTempError);
		handleInputChange(testRoutinesDispatch, id, 'maxTemp', maxTemp);

		if (validateMaxTempResult) {
			const numberMaxTemp = Number(maxTemp?.toString());
			const numberMaxTempDefault = Number(maxTempDefault?.toString());

			if (numberMaxTemp > numberMaxTempDefault) {
				setMaxTempWarning(getText('mainPage.wizardTwo.TOO_HIGH_TEMP'));
				return;
			}
		}

		setMaxTempWarning('');
	};

	const handleDischargeLimit = (part, dataValue) => {
		const dataPart = dataValue.trim();
		const parts = dischargeLimit.split(':');
		let dischargeLimitResult;

		switch (part) {
			case chargeLimitParts.DATA_VALUE: {
				validateDischargeLimit(isDischargeLimit, dataPart, setDischargeLimitError);
				dischargeLimitResult = `${dataPart}:${parts[1]}`;
				break;
			}
			case chargeLimitParts.DATA_UNIT: {
				dischargeLimitResult = `${parts[0]}:${dataPart}`;
				break;
			}
			default:
				return;
		}

		handleInputChange(testRoutinesDispatch, id, 'dischargeLimit', dischargeLimitResult);
	};

	const handleCutOffVoltage = dataValue => {
		const cutOffVoltage = dataValue.trim();
		validateCutOffVoltage(isCutOffVoltage, cutOffVoltage, setCutOffVoltageError, parseFloat(minVoltage?.toString()));
		handleInputChange(testRoutinesDispatch, id, 'cutOffVoltage', cutOffVoltage);
	};

	return (
		<Grid container component={Card} spacing={1} sx={{ py: 0.5, width: '100%'}}>
			<Grid item lg={4}>
				<FormControl error={validateString(maxTempError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
						<FormControlLabel
							label={getText('mainPage.wizardTwo.MAX_TEMPERATURE')}
							control={
								<Checkbox
									checked={isMaxTemp}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isMaxTemp', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(maxTempError) ? {color: theme => theme.palette.error.main,width: TITLE_WIDTH} : {width: TITLE_WIDTH}}
						/>

						<TextField
							error={validateString(maxTempError)}
							value={maxTemp ? maxTemp : ''}
							onChange={event => handleMaxTempChange(event.target.value)}
							disabled={!isMaxTemp || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 132,}}
							label='℃'
						/>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{maxTempError}
						{maxTempWarning}
					</FormHelperText>
				</FormControl>
			</Grid>

			<Grid item lg={8}>
				<FormControl error={validateString(cutOffVoltageError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
						<FormControlLabel
							label={getText('mainPage.wizardTwo.CUT_OFF_VOLTAGE')}
							control={
								<Checkbox
									checked={isCutOffVoltage}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isCutOffVoltage', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(cutOffVoltageError) ? {color: theme => theme.palette.error.main, width: SECOND_PART__WIDTH} : {width: SECOND_PART__WIDTH}}
						/>

						<TextField
							error={validateString(cutOffVoltageError)}
							value={cutOffVoltage ? cutOffVoltage : ''}
							onChange={event => handleCutOffVoltage(event.target.value)}
							disabled={!isCutOffVoltage || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 82,}}
							label={getText('mainPage.wizardTwo.VOLTS')}
						/>
						
						<Typography sx={{width: 223,}} >
							{getText('mainPage.wizardTwo.CUT_OFF_VOLTAGE_BATTERY')}
						</Typography>
						<Typography sx={{width: TITLE_WIDTH,}}>
							{cutOffVoltage ? (cutOffVoltage * getInputValue(testData, 'noCellSerial')).toFixed(2) : ''} {getText('mainPage.wizardTwo.VOLTS')}
						</Typography>
					</Stack>
					
					<FormHelperText sx={{mx: 1,}}>
						{cutOffVoltageError}
					</FormHelperText>
				</FormControl>
			</Grid>

			<Grid item lg={4} >
				<FormControl error={validateString(maxTimeError)} >
					<Stack direction="row" alignItems="center" spacing={1}>
						<FormControlLabel
							label={getText('mainPage.wizardTwo.MAX_TIME')}
							control={
								<Checkbox
									checked={isMaxTime}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isMaxTime', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(maxTimeError) ? {color: theme => theme.palette.error.main,width: TITLE_WIDTH} : {width: TITLE_WIDTH,}}
						/>

						<TextField
							value={validateString(maxTime) ? maxTime : ''}
							onChange={event => handleMaxTimeChange(event.target.value)}
							disabled={!isMaxTime || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 132,}}
							label={getText('mainPage.wizardTwo.TIME_FORMAT')}
						/>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{maxTimeError}
					</FormHelperText>
				</FormControl>
			</Grid>

			<Grid item lg={8} >
				<FormControl error={validateString(dischargeLimitError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
						<FormControlLabel
							label={getText('mainPage.wizardTwo.DISCHARGE_LIMIT')}
							control={
								<Checkbox
									checked={isDischargeLimit}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isDischargeLimit', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(dischargeLimitError) ? {color: theme => theme.palette.error.main,width: SECOND_PART__WIDTH} : {width: SECOND_PART__WIDTH,}}
						/>

						<TextField
							error={validateString(dischargeLimitError)}
							value={dischargeLimit.split(':')[0]}
							onChange={event => handleDischargeLimit(chargeLimitParts.DATA_VALUE, event.target.value)}
							disabled={!isDischargeLimit || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 82,}}
						/>

						<Select
							value={dischargeLimit.split(':')[1]}
							onChange={event => handleDischargeLimit(chargeLimitParts.DATA_UNIT, event.target.value)}
							disabled={!isDischargeLimit || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 190,}}
						>
							{getUnitVariants('tree').map((option, key) => (
								<MenuItem key={key} value={option.parameter}>
									{option.label}
								</MenuItem>
							))}
						</Select>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{dischargeLimitError}
					</FormHelperText>
				</FormControl>
			</Grid>
		</Grid>
	);
}
