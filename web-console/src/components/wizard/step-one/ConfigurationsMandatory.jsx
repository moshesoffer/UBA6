import { forwardRef, useImperativeHandle, useRef, useState, } from 'react';

import LockIcon from '@mui/icons-material/Lock';
import LockOpenIcon from '@mui/icons-material/LockOpen';
import Autocomplete from '@mui/material/Autocomplete';
import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Divider from '@mui/material/Divider';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';
import Stack from '@mui/material/Stack';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import { useTheme } from '@mui/material/styles';
import { setTestData, } from 'src/actions/TestRoutines';
import { LOCK_STATUS, UBA_CHANNEL_LIST, ubaChannel } from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useSettings, } from 'src/store/SettingsProvider';
import { useTestRoutines, useTestRoutinesDispatch } from 'src/store/TestRoutinesProvider';
import { useUbaDevices, } from 'src/store/UbaDevicesProvider';
import { getInputValue, getNumberValue, handleInputChange, isOtherChannelFree } from 'src/utils/helper';
import { validateObject, validateString } from 'src/utils/validators';

import {
	validateBatteryPN,
	validateBatterySN,
	validateCellPN,
	validateCellsInParallel,
	validateCellsInSerial,
	validateTestName,
} from '../validators';
import { getInputColor, } from './utils';

function ConfigurationsMandatory(props, ref) {

	const {initialTestRoutine,} = props;
	const inputColors = useRef(null);

	const [batteryPNError, setBatteryPNError] = useState('');
	const [batterySNError, setBatterySNError] = useState('');
	const [testNameError, setTestNameError] = useState('');
	const [cellPNError, setCellPNError] = useState('');
	const [cellsInSerialError, setCellsInSerialError] = useState('');
	const [cellsInParallelError, setCellsInParallelError] = useState('');

	const theme = useTheme();
	const {currentUba, ubaDevices} = useUbaDevices();
	const {testRoutines, testData, existingTest,} = useTestRoutines();
	const {cells,} = useSettings();

	let batteryPNList = testRoutines.map(item => item.batteryPN);
	batteryPNList = [...new Set(batteryPNList)];
	let batterySNList = testRoutines.map(item => item.batterySN);
	batterySNList = [...new Set(batterySNList)];
	let cellPNList = cells.map(cell => ({
		cellPN: cell?.itemPN,
		label: `${cell?.chemistry} ${cell?.manufacturer} ${cell?.itemPN}`,
	}),);

	const testRoutinesDispatch = useTestRoutinesDispatch();

	const handleBatteryPNChange = (event, newInputValue) => {
		inputColors.current = getInputColor('batteryPN', newInputValue, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'batteryPN', newInputValue)
		validateBatteryPN(newInputValue, setBatteryPNError);
	};

	const handleUbaChannelChange = event => {
		const newInputValue = event.target.value;
		inputColors.current = getInputColor('channel', newInputValue, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'channel', newInputValue)
	};

	const handleBatterySNChange = (event, newInputValue) => {
		inputColors.current = getInputColor('batterySN', newInputValue, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'batterySN', newInputValue)
		validateBatterySN(newInputValue, setBatterySNError);
	};

	const handleCellPNChange = (event, newInputValue) => {
		inputColors.current = getInputColor('cellPN', newInputValue?.cellPN, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'cellPN', newInputValue?.cellPN);
		validateCellPN(newInputValue?.cellPN, setCellPNError);
	};

	const handleCellsInSerialChange = event => {
		const noCellSerial = event.target.value;
		inputColors.current = getInputColor('noCellSerial', noCellSerial, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'noCellSerial', noCellSerial);
		validateCellsInSerial(noCellSerial, setCellsInSerialError);
	};

	const handleCellsInParallelChange = event => {
		const noCellParallel = event.target.value;
		inputColors.current = getInputColor('noCellParallel', noCellParallel, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'noCellParallel', noCellParallel);
		validateCellsInParallel(noCellParallel, setCellsInParallelError);
	};

	const handleTestNameChange = newInputValue => {
		inputColors.current = getInputColor('testName', newInputValue, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, 'testName', newInputValue);
		validateTestName(newInputValue, testRoutines, testData?.id, setTestNameError);
	};

	const handleFieldChange = (event, fieldName) => {
		const newInputValue = event.target.value;
		inputColors.current = getInputColor(fieldName, newInputValue, initialTestRoutine, existingTest, theme.palette.primary.main, inputColors.current);
		handleInputChange(testRoutinesDispatch, setTestData, fieldName, newInputValue);
	}

	const doValidation = () => {
		const testName = getInputValue(testData, 'testName');
		const batteryPN = getInputValue(testData, 'batteryPN');
		const batterySN = getInputValue(testData, 'batterySN');
		const cellPN = getInputValue(testData, 'cellPN');
		const noCellSerial = getInputValue(testData, 'noCellSerial');
		const noCellParallel = getInputValue(testData, 'noCellParallel');

		const isBatteryPNValid = validateBatteryPN(batteryPN, setBatteryPNError);
		const isBatterySNValid = validateBatterySN(batterySN, setBatterySNError);
		const isCellPNValid = validateCellPN(cellPN, setCellPNError);
		const isCellsInSerialValid = validateCellsInSerial(noCellSerial, setCellsInSerialError);
		const isCellsInParallelValid = validateCellsInParallel(noCellParallel, setCellsInParallelError);
		const isTestNameValid = validateTestName(testName, testRoutines, testData?.id, setTestNameError);

		return isTestNameValid && isBatteryPNValid && isBatterySNValid && isCellPNValid && isCellsInSerialValid && isCellsInParallelValid;
	};

	useImperativeHandle(ref, () => ({
		doValidation: doValidation,
	}));

	const printTestPlanElement = () => {
		const label = getText(`mainPage.wizardZero.${LOCK_STATUS[testData?.isLocked]?.toUpperCase()}`);
		const color = testData?.isLocked ? 'error' : 'primary';
		const Icon = testData?.isLocked ?
			<LockIcon fontSize="large" color={color}/> :
			<LockOpenIcon fontSize="large" color={color}/>;

		if (validateObject(currentUba, true)) {
			// We are on the Main Page.
			return (
				<Stack direction="row" alignItems="center" spacing={1} sx={{width: '49%',}}>
					{Icon}

					<Typography level="title-lg">
						{label}
					</Typography>
				</Stack>
			);
		}

		// We are on the Test Routines Page.
		return (
			<Box sx={{width: '49%',}}>
				<Button
					variant="outlined"
					size="medium"
					color={color}
					startIcon={Icon}
					onClick={() => handleInputChange(testRoutinesDispatch, setTestData, 'isLocked', Number(!testData?.isLocked))}
				>
					{label}
				</Button>
			</Box>
		);
	};

	const ubaChannelText = channel => {
		return (
			<Stack direction="row" alignItems="center">
				<Typography level="title-lg" sx={{pl: 1.5,}}>
					{getText('mainPage.wizardOne.UBA_CHANNEL')}
				</Typography>

				<Typography level="title-lg" sx={{pl: 1.5,}}>
					{validateString(channel) ? channel : getText('common.NOT_APPLICABLE')}
				</Typography>
			</Stack>
		);
	};

	const getUbaChannlesElement = () => {
		//console.log('currentUba', currentUba, testData);
		if (validateString(currentUba?.ubaChannel)) {
			// We are on the Main Page.
			switch (currentUba?.ubaChannel) {
				case ubaChannel.A:
				case ubaChannel.B: {
					return ubaChannelText(currentUba.ubaChannel)
				}
				case ubaChannel.AB: {
					const ubaChannelList = [];
					if(currentUba.channel === ubaChannel.A) {
						ubaChannelList.push(ubaChannel.A);
					} else if(currentUba.channel === ubaChannel.B) {
						ubaChannelList.push(ubaChannel.B);
					}
					const isAnotherChannelFree = isOtherChannelFree(ubaDevices, currentUba);
					
					return (
						<FormControl size="small" sx={{width: 130}}>
							<InputLabel>
								{getText('mainPage.wizardOne.UBA_CHANNEL')}
							</InputLabel>

							<Select
								value={getInputValue(testData, 'channel')}
								label={getText('mainPage.wizardOne.UBA_CHANNEL')}
								onChange={handleUbaChannelChange}
								SelectDisplayProps={{style: inputColors.current?.channel}}
								disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							>
								{ubaChannelList.map((option, key) => (
										<MenuItem key={key} value={option}>
											{option}
										</MenuItem>
									)
								)}
								<MenuItem key={UBA_CHANNEL_LIST.A_AND_B} value={UBA_CHANNEL_LIST.A_AND_B} disabled={!isAnotherChannelFree}>
									{isAnotherChannelFree ? UBA_CHANNEL_LIST.A_AND_B : `${UBA_CHANNEL_LIST.A_AND_B} (${getText('mainPage.wizardOne.ANOTHER_CHANNEL_FREE')})`}
								</MenuItem>
							</Select>
						</FormControl>
					);
				}
				default: {
					console.error('Invalid UBA channel');
					return ubaChannelText();
				}
			}
		};

		// We are on the Test Routine Page.
		return (
			<FormControl size="small" sx={{width: 130}}>
				<InputLabel>
					{getText('mainPage.wizardOne.UBA_CHANNEL')}
				</InputLabel>

				<Select
					value={getInputValue(testData, 'channel')}
					label={getText('mainPage.wizardOne.UBA_CHANNEL')}
					onChange={handleUbaChannelChange}
					SelectDisplayProps={{style: inputColors.current?.ubaChannel}}
					disabled={!!testData?.isLocked && validateObject(currentUba, true)}
				>
					{Object.values(UBA_CHANNEL_LIST).map((option, key) => (
						<MenuItem key={key} value={option}>
							{option === UBA_CHANNEL_LIST.A_AND_B ? 'Dual' : 'Single'}
						</MenuItem>
					))}
				</Select>
			</FormControl>
		);
	};

	return (
		<Card sx={{p: 3, mb: 2}} >
			<Typography variant="h5" color="#637381" >
				{getText('mainPage.wizardOne.TEST_CONFIGURATIONS')}
			</Typography>

			<Divider inset="none" sx={{mb: 4}}/>
			
			<CardContent >
				<Box
					rowGap={3}
					columnGap={2}
					display="grid"
					gridTemplateColumns={{
						xs: 'repeat(1, 1fr)',
						sm: 'repeat(1, 1fr)',
						lg: 'repeat(2, 1fr)',
					}}
					
				>
					<Box >
						<TextField
							size="small"
							error={validateString(testNameError)}
							label={getText('mainPage.wizardOne.TEST_PLAN_NAME')}
							value={getInputValue(testData, 'testName')}
							onChange={event => handleTestNameChange(event.target.value)}
							helperText={testNameError}
							inputProps={{style: inputColors.current?.testName}}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							required={true}
							sx={{width: 500}}
						/>
					</Box>

					<Box >
						{printTestPlanElement()}
					</Box>

					<Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'flex-start', gap: 2 }}>
						<Autocomplete
							freeSolo
							size="small"
							value={getInputValue(testData, 'batteryPN')}
							onChange={handleBatteryPNChange}
							options={batteryPNList}
							renderInput={params =>
								<TextField
									error={validateString(batteryPNError)}
									{...params}
									label={getText('testEditor.BATTERY_P_N')}
									helperText={batteryPNError}
									required={true}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{input: inputColors.current?.batteryPN, width:350}}
						/>

						{getUbaChannlesElement()}
					</Box>

					<Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'flex-start', gap: 2 }}>
						<TextField
							size="small"
							sx={{width: 220}}
							error={validateString(cellsInSerialError)}
							label={getText('testEditor.NO_CELLS_IN_SERIAL')}
							value={getInputValue(testData, 'noCellSerial')}
							onChange={handleCellsInSerialChange}
							helperText={cellsInSerialError}
							inputProps={{style: inputColors.current?.noCellSerial}}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							required={true}
						/>
						<Stack direction="row" alignItems="center">
							<Typography level="title-lg" sx={{pl: 1.5,}}>
								{getText('mainPage.wizardOne.MAX_PER_BATTERY')}
							</Typography>

							<Typography level="title-lg" sx={{pl: 1.5,}}>
								{getNumberValue(testData, 'maxPerBattery')}V
							</Typography>
						</Stack>
					</Box>

					<Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'flex-start', gap: 2 }}>
						<Autocomplete
							freeSolo
							size="small"
							value={getInputValue(testData, 'cellPN')}
							onChange={handleCellPNChange}
							options={cellPNList.sort((a, b) => -b.label.localeCompare(a.label))}
							renderInput={params =>
								<TextField
									error={validateString(cellPNError)}
									{...params}
									label={getText('testEditor.CELL_P_N')}
									helperText={cellPNError}
									required={true}
								/>
							}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							sx={{input: inputColors.current?.cellPN, width:350}}
						/>

						<Autocomplete
							freeSolo
							size="small"
							value={getInputValue(testData, 'batterySN')}
							onChange={handleBatterySNChange}
							options={batterySNList}
							renderInput={params =>
								<TextField
									error={validateString(batterySNError)}
									{...params}
									label={getText('reportsPage.BATTERY_S_N')}
									helperText={batterySNError}
									required={true}
								/>
							}
							sx={{input: inputColors.current?.batterySN, width:250}}
						/>
					</Box>

					<Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'flex-start', gap: 2 }}>
						<TextField
							size="small"
							sx={{width: 220}}
							error={validateString(cellsInParallelError)}
							label={getText('testEditor.NO_CELLS_IN_PARALLEL')}
							value={getInputValue(testData, 'noCellParallel')}
							onChange={handleCellsInParallelChange}
							helperText={cellsInParallelError}
							inputProps={{style: inputColors.current?.noCellParallel}}
							disabled={!!testData?.isLocked && validateObject(currentUba, true)}
							required={true}
						/>

						<Stack direction="row" alignItems="center">
							<Typography level="title-lg" sx={{pl: 1.5,width:227}}>
								{getText('mainPage.wizardOne.RATED_BATTERY_CAPACITY')}
							</Typography>

							<Typography level="title-lg" sx={{pl: 1.5}}>
								{getNumberValue(testData, 'ratedBatteryCapacity') ? getNumberValue(testData, 'ratedBatteryCapacity').toLocaleString() : null}mAh
							</Typography>
						</Stack>
					</Box>
				</Box>
			</CardContent>

			<Divider inset="none" sx={{mb: 4, mt: 2, borderBottomWidth: "3px"}}/>

			<CardContent>
				<TextField
						size="small"
						label={getText('mainPage.wizardOne.NOTES')}
						value={getInputValue(testData, 'notes')}
						onChange={event => handleFieldChange(event, 'notes')}
						inputProps={{style: inputColors.current?.notes}}
						sx={{width: '100%', mb: 3}}
					/>
				<Box
					rowGap={3}
					columnGap={2}
					display="grid"
					gridTemplateColumns={{
						xs: 'repeat(1, 1fr)',
						sm: 'repeat(2, 1fr)',
						lg: 'repeat(3, 1fr)',
					}}
				>
					<TextField
						size="small"
						label={getText('mainPage.wizardOne.CUSTOMER')}
						value={getInputValue(testData, 'customer')}
						onChange={event => handleFieldChange(event, 'customer')}
						inputProps={{style: inputColors.current?.customer}}
					/>

					<TextField
						size="small"
						label={getText('mainPage.wizardOne.WORK_ORDER_NUMBER')}
						value={getInputValue(testData, 'workOrderNumber')}
						onChange={event => handleFieldChange(event, 'workOrderNumber')}
						inputProps={{style: inputColors.current?.workOrderNumber}}
					/>

					<TextField
						size="small"
						label={getText('mainPage.wizardOne.APPROVED_BY')}
						value={getInputValue(testData, 'approvedBy')}
						onChange={event => handleFieldChange(event, 'approvedBy')}
						inputProps={{style: inputColors.current?.approvedBy}}
					/>

					<TextField
						size="small"
						label={getText('mainPage.wizardOne.CONDUCTED_BY')}
						value={getInputValue(testData, 'conductedBy')}
						onChange={event => handleFieldChange(event, 'conductedBy')}
						inputProps={{style: inputColors.current?.conductedBy}}
					/>

					<TextField
						size="small"
						label={getText('mainPage.wizardOne.CELL_SUPPLIER')}
						value={getInputValue(testData, 'cellSupplier')}
						onChange={event => handleFieldChange(event, 'cellSupplier')}
						inputProps={{style: inputColors.current?.cellSupplier}}
					/>

					<TextField
						size="small"
						label={getText('mainPage.wizardOne.CELL_DATE')}
						value={getInputValue(testData, 'cellBatch')}
						onChange={event => handleFieldChange(event, 'cellBatch')}
						inputProps={{style: inputColors.current?.cellBatch}}
					/>
				</Box>
			</CardContent>
		</Card>
	);
}

export default forwardRef(ConfigurationsMandatory);
