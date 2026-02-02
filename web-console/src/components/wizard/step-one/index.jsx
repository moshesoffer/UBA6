import {useState, useEffect, useRef,} from 'react';
import {useLocation,} from 'react-router-dom';

import Container from '@mui/material/Container';
import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import CardActions from '@mui/material/CardActions';
import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';
import {getText,} from 'src/services/string-definitions';
import {useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {useSettings} from 'src/store/SettingsProvider';
import {setTestData,} from 'src/actions/TestRoutines';
import {pageStateList,getSelectedCardsOrTableView} from 'src/constants/unsystematic';
import {validateInteger, validateNumber, validateString,} from 'src/utils/validators';
import {handleInputChange, resetTestParameters,} from 'src/utils/helper';

import ConfigurationsMandatory from './ConfigurationsMandatory';
import {changePageState,} from '../utils';

export default function StepOne(props) {

	const {pathname} = useLocation();
	const initialTestRoutine = useRef(null);
	const configurationsMandatoryRef = useRef(null);

	const {testData,} = useTestRoutines();
	const {cells,} = useSettings();

	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const [isError, setIsError] = useState(false);

	useEffect(() => {
		initialTestRoutine.current = testData;
	}, []);

	useEffect(() => {
		const noCellSerial = parseInt(testData?.noCellSerial?.toString());
		if (validateString(testData?.cellPN) && validateInteger(noCellSerial)) {
			const chosenCellPN = cells.find(cell => cell.itemPN === testData.cellPN);
			const maxVoltage = Number(chosenCellPN?.maxVoltage);

			if (validateNumber(maxVoltage) && maxVoltage > 0) {
				handleInputChange(testRoutinesDispatch, setTestData, 'maxVoltage', maxVoltage);
				const maxPerBattery = noCellSerial * maxVoltage;
				handleInputChange(testRoutinesDispatch, setTestData, 'maxPerBattery', maxPerBattery);
			}
		}
	}, [testData?.cellPN, testData?.noCellSerial,]);

	useEffect(() => {
		const noCellParallel = parseInt(testData?.noCellParallel?.toString());
		if (validateString(testData?.cellPN) && validateInteger(noCellParallel)) {
			const chosenCellPN = cells.find(cell => cell.itemPN === testData.cellPN);
			const chosenNomCapacity = Number(chosenCellPN?.nomCapacity);

			if (validateNumber(chosenNomCapacity) && chosenNomCapacity > 0) {
				const ratedBatteryCapacity = noCellParallel * chosenNomCapacity;
				handleInputChange(testRoutinesDispatch, setTestData, 'ratedBatteryCapacity', ratedBatteryCapacity);
			}
		}
	}, [testData?.cellPN, testData?.noCellParallel,]);

	const handleNextClick = () => {
		const resultMandatory = configurationsMandatoryRef.current.doValidation();

		if (!resultMandatory) {
			setIsError(true);
			return;
		}

		setIsError(false);
		handleStateChange(pageStateList.WIZARD_TWO);
	};

	const handleStateChange = state => {
		if (state === pageStateList.TABLE_VIEW || state === pageStateList.CARDS_VIEW) {
			resetTestParameters(testRoutinesDispatch);
		}

		changePageState(state, pathname, ubaDevicesDispatch, testRoutinesDispatch);
	}

	return (
		<Container maxWidth="false">

			<Box sx={{
						height: 'calc( 100vh - 240px)',
					}}>
				<ConfigurationsMandatory initialTestRoutine={{...initialTestRoutine.current}} ref={configurationsMandatoryRef}/>
			</Box>
			<Card sx={{mt: 3, px: 4, py: 1,}}>
				<CardActions sx={{justifyContent: 'space-between'}}>
					<Button variant="contained" color="primary" sx={{width: 140}} onClick={() => handleStateChange(getSelectedCardsOrTableView())}>
						{getText('common.CANCEL')}
					</Button>

					<Stack alignItems="center">
						<Button variant="contained" sx={{width: 140}} onClick={handleNextClick}>
							{getText('common.NEXT')}
						</Button>
						<Typography align="center" sx={{color: 'red', display: isError ? 'block' : 'none',}}>
							{getText('mainPage.wizardTwo.FIX_ALL_ERRORS')}
						</Typography>
					</Stack>
				</CardActions>
			</Card>
		</Container>
	);
}
