import {useState,} from 'react';

import Container from '@mui/material/Container';
import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import TextField from '@mui/material/TextField';
import Button from '@mui/material/Button';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import CardActions from '@mui/material/CardActions';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevices, useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines,useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {createRunningTest,} from 'src/action-creators/TestRoutines';
import {setSelectedDevices, setState,} from 'src/actions/UbaDevices';
import CustomTableHead from 'src/components/CustomTableHead';
import {getText,} from 'src/services/string-definitions';
import {validateString,} from 'src/utils/validators';
import {getSelectedCardsOrTableView} from 'src/constants/unsystematic';
import { fillTestRoutine, isOtherChannelFree, resetTestParameters } from 'src/utils/helper';
import headLabels from './headLabels';
import CustomTableRow from './CustomTableRow';
import Box from '@mui/material/Box';

function RunBatchTest() {

	const [isStepOne, setIsStepOne] = useState(true);
	const [batterySN, setBatterySN] = useState('');
	const [batterySNError, setBatterySNError] = useState('');

	const {selectedDevices} = useUbaDevices();
	const {testData, plan,} = useTestRoutines();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const testDetails = selectedDevices.map((device, index) => {
		let lastFourChars = batterySN.slice(-4);
		lastFourChars = parseInt(lastFourChars) + index;
		lastFourChars = lastFourChars.toString().padStart(4, '0');

		let batterySerialNumber = batterySN.slice(0, -4);
		batterySerialNumber = `${batterySerialNumber}${lastFourChars}`;

		return {
			...device,
			batterySerialNumber,
		};
	});

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	const handleBatterySNChange = event => {
		const batterySN = event.target.value;
		setBatterySN(batterySN);
		const result = /^.*-\d{3,4}$/.test(batterySN);
		if (!result) {
			setBatterySNError('Should end with a Dash and then 3 or 4 digits');
			return;
		}

		setBatterySNError('');
	}

	const handleRunClick = () => {
		const ubaSNs = selectedDevices.map(device => ({
			ubaSN: device.ubaSN,
			channel: device.channel,
		}));
		const testRoutine = {
			...testData,
			plan,
		};
		delete testRoutine.channel;

		createRunningTest(authDispatch, ubaDevicesDispatch, ubaSNs, testRoutine);

		ubaDevicesDispatch(setSelectedDevices([]));
		ubaDevicesDispatch(setState(getSelectedCardsOrTableView()));
	}

	if (isStepOne) {
		return (
			<Container maxWidth="false">
				<Card sx={{p: 3, height: 'calc( 100vh - 240px)',}}>
					<Stack alignItems="center" spacing={4}>
						<Typography variant="h4">
							{getText('mainPage.runBatchTest.SERIAL_NUMBER_BATCH')}
						</Typography>

						<TextField
							error={validateString(batterySNError)}
							label={`${getText('reportsPage.BATTERY_S_N')} ${getText('mainPage.runBatchTest.BATCH')}`}
							value={batterySN}
							onChange={handleBatterySNChange}
							helperText={batterySNError}
						/>
					</Stack>
				</Card>
				
				<Card sx={{mt: 3, px: 4, py: 1,}}>
					<CardActions sx={{justifyContent: 'space-between'}}>
						<Button variant="contained" color="primary" sx={{width: 140}} onClick={() => {resetTestParameters(testRoutinesDispatch); ubaDevicesDispatch(setState(getSelectedCardsOrTableView()));}}>
							{getText('common.CANCEL')}
						</Button>

						<Button
							variant="contained"
							size="large"
							sx={{width: 140}}
							onClick={event => setIsStepOne(false)}
							disabled={!validateString(batterySN) || validateString(batterySNError)}
						>
							{getText('common.NEXT')}
						</Button>
					</CardActions>
				</Card>
			</Container>
		);
	}

	return (
		<Container maxWidth="false">
			<Card sx={{p: 3, height: 'calc( 100vh - 240px)',}}>
				<Stack alignItems="center" spacing={2}>
					<Typography variant="h4">
						{getText('mainPage.runBatchTest.APPROVE_BATCH_TEST')}
					</Typography>

					<Typography variant="subtitle1">
						The following tests will run as followed
					</Typography>

					<Typography>
						{`${getText('common.TEST_NAME')}: ${testData?.testName}`}
					</Typography>

					<Table aria-label="simple table">
						<CustomTableHead headLabels={headLabels}/>

						<TableBody>
							{testDetails.map((row, key) => (
								<CustomTableRow id={row?.id} key={key} {...{row}} />
							))}
						</TableBody>
					</Table>

					
				</Stack>
			</Card>
			<Card sx={{mt: 3, px: 4, py: 1,}}>
				<CardActions sx={{justifyContent: 'space-between'}}>
					<Button variant="contained" color="primary" sx={{width: 140}} onClick={() => setIsStepOne(true)}>
						{getText('common.BACK')}
					</Button>
					<Button variant="contained" sx={{width: 140}} onClick={handleRunClick}>
						{getText('mainPage.wizardTwo.RUN')}
					</Button>
				</CardActions>
			</Card>
		</Container>
	);
}

export default RunBatchTest;
