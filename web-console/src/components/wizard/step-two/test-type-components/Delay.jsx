import {useState, useEffect,} from 'react';

import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import TextField from '@mui/material/TextField';
import FormHelperText from '@mui/material/FormHelperText';
import FormControl from "@mui/material/FormControl";
import Grid from '@mui/material/Grid';
import {getText,} from 'src/services/string-definitions';
import {useUbaDevices,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {validateString, validateObject,} from 'src/utils/validators';
import {chargeLimitParts, TITLE_WIDTH,SECOND_PART__WIDTH} from 'src/constants/unsystematic';
import GapBox from '../GapBox';
import CustomHead from './CustomHead';
import {handleInputChange,} from '../utils';
import {validateDelayTime, validateWaitTemp,} from '../../validators';

export default function Delay(props) {

	const {id, isCollapsed, delayTime, waitTemp,} = props;

	const [delayTimeError, setDelayTimeError] = useState('');
	const [waitTempError, setWaitTempError] = useState('');
	const [delayTimeOrWaitTempError, setDelayTimeOrWaitTempError] = useState('');

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
		//console.log('runCommonValidation',delayTime,waitTemp);
		if(delayTime === null && waitTemp === null) {
			setDelayTimeOrWaitTempError(getText('mainPage.wizardTwo.DELAY_TIME_OR_WAIT_TEMP_REQUIRED'));
			return;
		}
		setDelayTimeOrWaitTempError('');
		if(delayTime != null) validateDelayTime(delayTime, setDelayTimeError);
		if(waitTemp != null) validateWaitTemp(waitTemp, setWaitTempError);
	};

	const handleDelayTimeChange = dataValue => {
		setDelayTimeOrWaitTempError('');
		const delayTime = dataValue.trim();
		if(delayTime !== '') {
			validateDelayTime(delayTime, setDelayTimeError);
		} else {
			setDelayTimeError('');
		}
		handleInputChange(testRoutinesDispatch, id, 'delayTime', delayTime);
	};

	const handleWaitTempChange = dataValue => {
		setDelayTimeOrWaitTempError('');
		const waitTemp = dataValue.trim();
		if(waitTemp !== '') {
			validateWaitTemp(waitTemp, setWaitTempError);
		} else {
			setWaitTempError('');
		}
		handleInputChange(testRoutinesDispatch, id, 'waitTemp', waitTemp);
	};

	if (isCollapsed) {
		return (
			<>
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.DELAY"/>
				<GapBox {...{id}}/>
			</>
		);
	}

	return (
		<>
			<Stack spacing={1} sx={{
				width: 1,
				p: 1,
				border: 'solid 2px #d0d8e5',
				borderRadius: '16px',
			}}>
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.DELAY"/>
					
				<Grid container component={Card} spacing={1} sx={{ py: 0.5, width: '100%'}} >
					<Grid item lg={4}>
						<FormControl error={validateString(delayTimeError) || validateString(delayTimeOrWaitTempError)} >
							<Stack direction="row" alignItems="center" spacing={1}>
								<Typography sx={{width: TITLE_WIDTH}}>
									{getText('mainPage.wizardTwo.DELAY_TIME')}
								</Typography>
								<TextField
									
									value={validateString(delayTime) ? delayTime : ''}
									onChange={event => handleDelayTimeChange(event.target.value)}
									disabled={!!testData?.isLocked && validateObject(currentUba, true)}
									sx={{width: 132,}}
									label={getText('mainPage.wizardTwo.TIME_FORMAT')}
									error={validateString(delayTimeError)}
								/>
							</Stack>
							<FormHelperText sx={{mx: 1,}}>
								{ delayTimeError ? delayTimeError : '' }
								{ delayTimeOrWaitTempError ? delayTimeOrWaitTempError : '' }
							</FormHelperText>
						</FormControl>
					</Grid>
					<Grid item lg={5}>
						<FormControl  error={validateString(waitTempError)}>
							<Stack direction="row" alignItems="center" spacing={1}>
								<Typography sx={{width: SECOND_PART__WIDTH}}>
									{getText('mainPage.wizardTwo.WAIT_TEMPERATURE')}
								</Typography>

								<TextField
									
									error={validateString(waitTempError)}
									value={waitTemp ? waitTemp : ''}
									onChange={event => handleWaitTempChange(event.target.value)}
									disabled={!!testData?.isLocked && validateObject(currentUba, true)}
									sx={{width: 82,}}
									label='℃'
								/>
							</Stack>
						<FormHelperText sx={{mx: 1,}}>
							{waitTempError}
						</FormHelperText>
					</FormControl>
					</Grid>
				</Grid>
			</Stack>

			<GapBox {...{id}}/>
		</>
	);
}
