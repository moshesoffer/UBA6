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
import {validateInteger, validateString, validateObject,} from 'src/utils/validators';
import {chargeLimitParts, TITLE_WIDTH,SECOND_PART__WIDTH} from 'src/constants/unsystematic';
import GapBox from '../GapBox';
import CustomHead from './CustomHead';
import {handleInputChange,} from '../utils';
import {validateGoToStep, validateRepeatStep,} from '../../validators';

export default function Loop(props) {

	const {id, isCollapsed, goToStep, repeatStep,} = props;

	const [goToStepError, setGoToStepError] = useState('');
	const [repeatStepError, setRepeatStepError] = useState('');

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
		validateGoToStep(id, goToStep, setGoToStepError);
		validateRepeatStep(repeatStep, setRepeatStepError);
	};

	const getGoToStepvValue = () => {
		if (!validateInteger(goToStep) && !validateString(goToStep)) {
			return '';
		}

		let goToStepValue = parseInt(goToStep, 10);
		if (validateInteger(goToStepValue) && goToStepValue.toString() === goToStep.toString()) {
			return goToStepValue + 1;
		}

		return goToStep;
	};

	const handleGoToStepChange = dataValue => {
		const goToStepString = dataValue.trim();

		const goToStepInteger = parseInt(goToStepString, 10);
		if (validateInteger(goToStepInteger) && goToStepInteger.toString() === goToStepString.toString()) {
			const goToStepReal = goToStepInteger - 1;
			validateGoToStep(id, goToStepReal, setGoToStepError);
			handleInputChange(testRoutinesDispatch, id, 'goToStep', goToStepReal);
			return;
		}

		validateGoToStep(id, goToStepString, setGoToStepError);
		handleInputChange(testRoutinesDispatch, id, 'goToStep', goToStepString);
	};

	const handleRepeatStepChange = dataValue => {
		const repeatStep = dataValue.trim();
		validateRepeatStep(repeatStep, setRepeatStepError);
		handleInputChange(testRoutinesDispatch, id, 'repeatStep', repeatStep);
	};

	if (isCollapsed) {
		return (
			<>
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.LOOP"/>
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
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.LOOP"/>

				<Grid container component={Card} spacing={1} sx={{ py: 0.5, width: '100%'}} >
					<Grid item lg={4}>
						<FormControl error={validateString(goToStepError)} >
							<Stack direction="row" alignItems="center" spacing={1}>
								<Typography sx={{width: TITLE_WIDTH}}>
									{getText('mainPage.wizardTwo.GO_TO_STEP')}
								</Typography>

								<TextField
									error={validateString(goToStepError)}
									
									value={getGoToStepvValue()}
									onChange={event => handleGoToStepChange(event.target.value)}
									disabled={!!testData?.isLocked && validateObject(currentUba, true)}
									sx={{width: 132,}}
									label='#'
								/>
							</Stack>

							<FormHelperText sx={{mx: 1,}}>
								{goToStepError}
							</FormHelperText>
						</FormControl>
					</Grid>
					<Grid item lg={5}>
						<FormControl error={validateString(repeatStepError)}>
							<Stack direction="row" alignItems="center" spacing={1}>
								<Typography sx={{width: SECOND_PART__WIDTH}}>
									{getText('mainPage.wizardTwo.REPEAT_STEP')}
								</Typography>

								<TextField
									error={validateString(repeatStepError)}
									value={repeatStep ? repeatStep : ''}
									onChange={event => handleRepeatStepChange(event.target.value)}
									disabled={!!testData?.isLocked && validateObject(currentUba, true)}
									sx={{width: 82,}}
									label={getText('mainPage.wizardTwo.TIMES')}
								/>
							</Stack>

							<FormHelperText sx={{mx: 1,}}>
								{repeatStepError}
							</FormHelperText>
						</FormControl>
					</Grid>
					
				</Grid>
			</Stack>

			<GapBox {...{id}}/>
		</>
	);
}
