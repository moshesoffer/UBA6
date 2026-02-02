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
import {getUnitVariants,} from 'src/utils/helper';

import {handleInputChange, FIRST_COLUMN, SECOND_COLUMN,} from '../../utils';
import {
	validateMaxTime,
	validateMaxTemp,
	validateChargeLimit,
	validateCutOffCurrent,
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
		isChargeLimit,
		chargeLimit,
		isCutOffCurrent,
		cutOffCurrent,
	} = props;

	const [maxTimeError, setMaxTimeError] = useState('');
	const [maxTempWarning, setMaxTempWarning] = useState('');
	const [maxTempError, setMaxTempError] = useState('');
	const [chargeLimitError, setChargeLimitError] = useState('');
	const [cutOffCurrentError, setCutOffCurrentError] = useState('');

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
		validateCheckboxGroup(isMaxTime, isChargeLimit, isCutOffCurrent, setMaxTimeError, setChargeLimitError, setCutOffCurrentError);

		if (isMaxTime) {
			validateMaxTime(isMaxTime, maxTime, setMaxTimeError);
		}

		if (isMaxTemp) {
			validateMaxTemp(isMaxTemp, maxTemp, setMaxTempError);
		}

		if (isChargeLimit) {
			validateChargeLimit(isChargeLimit, chargeLimit, setChargeLimitError);
		}

		if (isCutOffCurrent) {
			validateCutOffCurrent(isCutOffCurrent, cutOffCurrent, setCutOffCurrentError);
		}
	}

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

	const handleChargeLimit = (part, dataValue) => {
		const dataPart = dataValue.trim();
		const parts = chargeLimit.split(':');
		let chargeLimitResult;

		switch (part) {
			case chargeLimitParts.DATA_VALUE: {
				const ratedBatteryCapacity = parseFloat(testData?.ratedBatteryCapacity?.toString());
				validateChargeLimit(isChargeLimit, dataPart, setChargeLimitError, ratedBatteryCapacity);
				chargeLimitResult = `${dataPart}:${parts[1]}`;
				break;
			}
			case chargeLimitParts.DATA_UNIT: {
				chargeLimitResult = `${parts[0]}:${dataPart}`;
				break;
			}
			default:
				return;
		}

		handleInputChange(testRoutinesDispatch, id, 'chargeLimit', chargeLimitResult);
	};

	const handleCutOffCurrent = (part, dataValue) => {
		const dataPart = dataValue.trim();
		const parts = cutOffCurrent.split(':');
		let cutOffCurrentResult;

		switch (part) {
			case chargeLimitParts.DATA_VALUE: {
				validateCutOffCurrent(isCutOffCurrent, dataPart, setCutOffCurrentError);
				cutOffCurrentResult = `${dataPart}:${parts[1]}`;
				break;
			}
			case chargeLimitParts.DATA_UNIT: {
				cutOffCurrentResult = `${parts[0]}:${dataPart}`;
				break;
			}
			default:
				return;
		}

		handleInputChange(testRoutinesDispatch, id, 'cutOffCurrent', cutOffCurrentResult);
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
				<FormControl error={validateString(cutOffCurrentError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
						<FormControlLabel
							label={getText('mainPage.wizardTwo.CUT_OFF_CURRENT')}
							control={
								<Checkbox
									checked={isCutOffCurrent}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isCutOffCurrent', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(cutOffCurrentError) ? {color: theme => theme.palette.error.main, width: SECOND_PART__WIDTH} : {width: SECOND_PART__WIDTH,}}
						/>

						<TextField
							error={validateString(cutOffCurrentError)}
							value={cutOffCurrent?.split(':')?.[0]}
							onChange={event => handleCutOffCurrent(chargeLimitParts.DATA_VALUE, event.target.value)}
							disabled={!isCutOffCurrent || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 82,}}
						/>

						<Select
							value={cutOffCurrent?.split(':')?.[1]}
							onChange={event => handleCutOffCurrent(chargeLimitParts.DATA_UNIT, event.target.value)}
							disabled={!isCutOffCurrent || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 190,}}
						>
							{getUnitVariants('one').map((option, key) => (
								<MenuItem key={key} value={option.parameter}>
									{option.label}
								</MenuItem>
							))}
						</Select>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{cutOffCurrentError}
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
							sx={validateString(maxTimeError) ? {color: theme => theme.palette.error.main,width: TITLE_WIDTH,} : {width: TITLE_WIDTH,}}
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
				<FormControl error={validateString(chargeLimitError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
						<FormControlLabel
							label={getText('mainPage.wizardTwo.CHARGE_LIMIT')}
							control={
								<Checkbox
									checked={isChargeLimit}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isChargeLimit', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(chargeLimitError) ? {color: theme => theme.palette.error.main,width: SECOND_PART__WIDTH} : {width: SECOND_PART__WIDTH,}}
						/>

						<TextField
							error={validateString(chargeLimitError)}
							value={chargeLimit?.split(':')?.[0]}
							onChange={event => handleChargeLimit(chargeLimitParts.DATA_VALUE, event.target.value)}
							disabled={!isChargeLimit || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 82,}}
						/>

						<Select
							value={chargeLimit?.split(':')?.[1]}
							onChange={event => handleChargeLimit(chargeLimitParts.DATA_UNIT, event.target.value)}
							disabled={!isChargeLimit || (!!testData?.isLocked && validateObject(currentUba, true))}
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
						{chargeLimitError}
					</FormHelperText>
				</FormControl>
			</Grid>
			
		</Grid>
	);
}
