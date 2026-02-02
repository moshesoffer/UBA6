import { useEffect, useState, } from 'react';

import Card from '@mui/material/Card';
import Checkbox from '@mui/material/Checkbox';
import FormControl from "@mui/material/FormControl";
import FormControlLabel from '@mui/material/FormControlLabel';
import FormHelperText from '@mui/material/FormHelperText';
import Grid from '@mui/material/Grid';
import MenuItem from "@mui/material/MenuItem";
import Select from "@mui/material/Select";
import Stack from '@mui/material/Stack';
import { useTheme } from '@mui/material/styles';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import { chargeLimitParts, SECOND_PART__WIDTH, TITLE_WIDTH } from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useTestRoutines, useTestRoutinesDispatch, } from 'src/store/TestRoutinesProvider';
import { useUbaDevices, } from 'src/store/UbaDevicesProvider';
import { getUnitVariants, } from 'src/utils/helper';
import { validateObject, validateString, } from 'src/utils/validators';

import { validateDischargeCurrent, validateMinTemp, } from '../../../validators';
import { handleInputChange } from '../../utils';

export default function UpperPart(props) {

	const {id, source, isMinTemp, minTemp, minTempDefault, dischargeCurrent,cRate} = props;

	const [minTempWarning, setMinTempWarning] = useState('');
	const [minTempError, setMinTempError] = useState('');
	const [dischargeCurrentError, setDischargeCurrentError] = useState('');

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

		validateDischargeCurrent(dischargeCurrent, setDischargeCurrentError);
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

	const handleDischargeCurrentChange = (part, dataValue) => {
		const dataPart = dataValue.trim();
		const parts = dischargeCurrent.split(':');
		let dischargeCurrentResult;

		switch (part) {
			case chargeLimitParts.DATA_VALUE: {
				validateDischargeCurrent(dataPart, setDischargeCurrentError);
				dischargeCurrentResult = `${dataPart}:${parts[1]}`;
				break;
			}
			case chargeLimitParts.DATA_UNIT: {
				dischargeCurrentResult = `${parts[0]}:${dataPart}`;
				break;
			}
			default:
				return;
		}

		handleInputChange(testRoutinesDispatch, id, 'dischargeCurrent', dischargeCurrentResult);
	};

	return (
		<Grid container component={Card} spacing={1} sx={{ py: 0.5, width: '100%'}}>
			<Grid item lg={4} >
				<Stack direction="row" alignItems="center" spacing={1}>
					<Typography sx={{width: TITLE_WIDTH}}>
						{getText('mainPage.wizardTwo.SOURCE')}
					</Typography>

					<FormControl sx={{width: 132}}>
						<Select
							value={validateString(source) ? source : ''}
							onChange={event => handleInputChange(testRoutinesDispatch, id, 'source', event.target.value)}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{fontSize:'0.9rem'}}
						>
							{getUnitVariants('five').map((option, key) => (
								<MenuItem key={key} value={option.parameter}>
									{option.label}
								</MenuItem>
							))}
						</Select>
					</FormControl>
				</Stack>
			</Grid>

			<Grid item lg={5} >
				<FormControl error={validateString(dischargeCurrentError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
					<Typography sx={{width: SECOND_PART__WIDTH,}}>
							{getText('mainPage.wizardTwo.DISCHARGE_CURRENT')}
						</Typography>

						<TextField
							error={validateString(dischargeCurrentError)}
							value={dischargeCurrent?.split(':')?.[0]}
							onChange={event => handleDischargeCurrentChange(chargeLimitParts.DATA_VALUE, event.target.value)}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{width: 82,}}
						/>

						<Select
							value={dischargeCurrent?.split(':')?.[1]}
							onChange={event => handleDischargeCurrentChange(chargeLimitParts.DATA_UNIT, event.target.value)}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{width: 190,}}
						>
							{getUnitVariants('six').map((option, key) => (
								<MenuItem key={key} value={option.parameter}>
									{option.label}
								</MenuItem>
							))}
						</Select>
					</Stack>

					<FormHelperText sx={{mx: 1,}}>
						{dischargeCurrentError}
					</FormHelperText>
				</FormControl>
			</Grid>

			<Grid item lg={3} >
				<Stack direction="row" alignItems="center" spacing={1} sx={{width: 340, mt: 1, ml: 4}}>
					{ cRate != null && 
						<Typography sx={{fontSize: '0.8rem'}}>
							{dischargeCurrent?.split(':')?.[1]==='relative' ? getText('mainPage.wizardTwo.CURRENT_MA') : getText('mainPage.wizardTwo.C_RATE')}
						</Typography>
					}
					{ cRate != null && 
						<Typography sx={{fontSize: '0.8rem'}}>
							{cRate}
						</Typography>
					}

					{((dischargeCurrent?.split(':')?.[1]!='relative' && cRate >= 2) || (dischargeCurrent?.split(':')?.[1]==='relative' && dischargeCurrent?.split(':')?.[0] >= 2)) && (
						<Typography sx={{color: theme.palette.error.main, fontSize: '0.8rem'}}>
							{getText('mainPage.wizardTwo.HIGH_CURRENT')}
						</Typography>
					)}		
				</Stack>
			</Grid>

			<Grid item lg={4} >
				<FormControl error={validateString(minTempError)}>
					<Stack direction="row" alignItems="center" spacing={1}>
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
							sx={validateString(minTempError) ? {color: theme => theme.palette.error.main,width: 200} : {width: 200, }}
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
		</Grid>
	);
}
