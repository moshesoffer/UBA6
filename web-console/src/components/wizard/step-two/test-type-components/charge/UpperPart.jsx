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

import {useTheme} from '@mui/material/styles';

import {getText,} from 'src/services/string-definitions';
import {useUbaDevices,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {validateNumber, validateString, validateObject,} from 'src/utils/validators';
import {chargeLimitParts, TITLE_WIDTH,SECOND_PART__WIDTH} from 'src/constants/unsystematic';
import {getUnitVariants,getInputValue} from 'src/utils/helper';

import {handleInputChange, FIRST_COLUMN, SECOND_COLUMN,} from '../../utils';
import {validateMinTemp, validateChargeCurrent, validateChargePerCell,} from '../../../validators';

export default function UpperPart(props) {

	const {id, source, isMinTemp, minTemp, minTempDefault, chargeCurrent, cRate, chargePerCell, maxVoltage,} = props;

	const [minTempWarning, setMinTempWarning] = useState('');
	const [minTempError, setMinTempError] = useState('');
	const [chargeCurrentError, setChargeCurrentError] = useState('');
	const [chargePerCellError, setChargePerCellError] = useState('');

	const testRoutinesDispatch = useTestRoutinesDispatch();

	const theme = useTheme();
	const {currentUba,} = useUbaDevices();
	const {testData, saveIsRun,} = useTestRoutines();

	useEffect(() => {
		if (!saveIsRun) {
			return;
		}

		runCommonValidation();
	}, [saveIsRun,]);

	const runCommonValidation = () => {
		if (isMinTemp) {
			validateMinTemp(isMinTemp, minTemp, setMinTempError);
		}

		validateChargeCurrent(chargeCurrent, setChargeCurrentError);
		validateChargePerCell(chargePerCell, setChargePerCellError);

	};

	const handleMinTempChange = dataValue => {
		const minTemp = dataValue.trim();
		const validateMinTempResult = validateMinTemp(isMinTemp, minTemp, setMinTempError);
		handleInputChange(testRoutinesDispatch, id, 'minTemp', minTemp);

		if (validateMinTempResult) {
			const numberMinTemp = Number(minTemp?.toString());
			const numberMinTempDefault = Number(minTempDefault?.toString());

			if (numberMinTemp < numberMinTempDefault) {
				setMinTempWarning(getText('mainPage.wizardTwo.TOO_LOW_TEMP'));
				return;
			}
		}

		setMinTempWarning('');
	};

	const handleChargeCurrentChange = (part, dataValue) => {
		const dataPart = dataValue.trim();
		const parts = chargeCurrent.split(':');
		let chargeCurrentResult;

		switch (part) {
			case chargeLimitParts.DATA_VALUE: {
				validateChargeCurrent(dataPart, setChargeCurrentError);
				chargeCurrentResult = `${dataPart}:${parts[1]}`;
				break;
			}
			case chargeLimitParts.DATA_UNIT: {
				chargeCurrentResult = `${parts[0]}:${dataPart}`;
				break;
			}
			default:
				return;
		}

		handleInputChange(testRoutinesDispatch, id, 'chargeCurrent', chargeCurrentResult);
	};

	const handleChargePerCellChange = dataValue => {
		const chargePerCell = dataValue.trim();
		validateChargePerCell(chargePerCell, setChargePerCellError, parseFloat(maxVoltage?.toString()));
		handleInputChange(testRoutinesDispatch, id, 'chargePerCell', chargePerCell);
	};

	return (
		<Grid container component={Card} spacing={1} sx={{ py: 0.5, width: '100%'}} >
			<Grid item lg={4} >
				<Stack direction="row" alignItems="center" spacing={1}>
					<Typography sx={{width: TITLE_WIDTH}}>
						{getText('mainPage.wizardTwo.SOURCE')}
					</Typography>

					<FormControl disabled={!!testData?.isLocked && validateObject(currentUba, true)} sx={{width: 132,}}>
						<Select
							value={validateString(source) ? source : ''}
							onChange={event => handleInputChange(testRoutinesDispatch, id, 'source', event.target.value)}
							sx={{fontSize:'0.9rem'}}
						>
							{getUnitVariants('four').map((option, key) => (
								<MenuItem key={key} value={option.parameter}>
									{option.label}
								</MenuItem>
							))}
						</Select>
					</FormControl>
				</Stack>
			</Grid>

			<Grid item lg={5} >
				<FormControl error={validateString(chargeCurrentError)} >
					<Stack direction="row" alignItems="center" spacing={1} >
						<Typography sx={{width: SECOND_PART__WIDTH,}}>
							{getText('mainPage.wizardTwo.CHARGE_CURRENT')}
						</Typography>

						<TextField
							error={validateString(chargeCurrentError)}
							value={chargeCurrent?.split(':')?.[0]}
							onChange={event => handleChargeCurrentChange(chargeLimitParts.DATA_VALUE, event.target.value)}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{width: 82,}}
						/>

						<Select
							value={chargeCurrent?.split(':')?.[1]}
							onChange={event => handleChargeCurrentChange(chargeLimitParts.DATA_UNIT, event.target.value)}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{width: 190,}}
						>
							{getUnitVariants('two').map((option, key) => (
								<MenuItem key={key} value={option.parameter}>
									{option.label}
								</MenuItem>
							))}
						</Select>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{chargeCurrentError}
					</FormHelperText>
				</FormControl>
			</Grid>

			<Grid item lg={3} >
				<Stack direction="row" alignItems="center" spacing={1} sx={{width: 340, mt: 1, ml: 4}}>
					<Typography sx={{fontSize: '0.8rem'}}>
						{chargeCurrent?.split(':')?.[1]==='relative' ? getText('mainPage.wizardTwo.CURRENT_MA') : getText('mainPage.wizardTwo.C_RATE')}
					</Typography>

					<Typography sx={{fontSize: '0.8rem'}}>
						{(cRate === null) ? getText('common.NOT_APPLICABLE') : cRate}
					</Typography>

					{((chargeCurrent?.split(':')?.[1]!='relative' && cRate >= 0.5) || (chargeCurrent?.split(':')?.[1]==='relative' && chargeCurrent?.split(':')?.[0] >= 0.5)) && (
						<Typography sx={{color: theme.palette.error.main, fontSize: '0.8rem'}}>
							{getText('mainPage.wizardTwo.HIGH_CURRENT')}
						</Typography>
					)}		
				</Stack>
			</Grid>

			<Grid item lg={4} >
				<FormControl error={validateString(minTempError)}>
					<Stack direction="row" alignItems="center" spacing={1} >
						<FormControlLabel
							label={getText('mainPage.wizardTwo.MIN_TEMPERATURE')}
							control={
								<Checkbox
									checked={isMinTemp}
									onChange={event => handleInputChange(testRoutinesDispatch, id, 'isMinTemp', event.target.checked)}
									sx={{pl: 0,}}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={validateString(minTempError) ? {color: theme => theme.palette.error.main,width: 200, } : {width: 200, }}
						/>

						<TextField
							error={validateString(minTempError)}
							value={minTemp ? minTemp : ''}
							onChange={event => handleMinTempChange(event.target.value)}
							disabled={!isMinTemp || (!!testData?.isLocked && validateObject(currentUba, true))}
							sx={{width: 132,}}
							label='℃'
						/>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{minTempError}
						{minTempWarning}
					</FormHelperText>
				</FormControl>
			</Grid>
			<Grid item lg={5} >
				<FormControl error={validateString(chargePerCellError)}>
					<Stack direction="row" alignItems="center" spacing={1} >
						<Typography sx={{width: SECOND_PART__WIDTH,}}>
							{getText('mainPage.wizardTwo.CHARGE_PER_CELL')}
						</Typography>

						<TextField
							error={validateString(chargePerCellError)}
							value={chargePerCell ? chargePerCell : ''}
							onChange={event => handleChargePerCellChange(event.target.value)}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{width: 82,}}
							label={getText('mainPage.wizardTwo.VOLTS')}
						/>
					</Stack>
					<FormHelperText sx={{mx: 1,}}>
						{chargePerCellError}
					</FormHelperText>
				</FormControl>
			</Grid>

			<Grid item lg={3} sx={{ mt: 1}}>
				<Stack direction="row" alignItems="center" spacing={1} sx={{ ml: 4}}>
					<Typography sx={{fontSize: '0.8rem'}}>
						{getText('mainPage.wizardTwo.CHARGE_VOLTAGE')}
					</Typography>
					<Typography sx={{fontSize: '0.8rem'}}>
						{chargePerCell ? (chargePerCell * getInputValue(testData, 'noCellSerial')).toFixed(2) : ''} {getText('mainPage.wizardTwo.VOLTS')}
					</Typography>
				</Stack>
			</Grid>

		</Grid>
	);
}
