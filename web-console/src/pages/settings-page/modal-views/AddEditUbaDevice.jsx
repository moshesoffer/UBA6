import {useState,} from 'react';

import Box from '@mui/material/Box';
import Stack from '@mui/material/Stack';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import FormHelperText from '@mui/material/FormHelperText';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';

import {useAuth, useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevices, useUbaDevicesDispatch} from 'src/store/UbaDevicesProvider';
import {useSettings,} from 'src/store/SettingsProvider';
import withModalView from 'src/components/withModalView';
import {getText,} from 'src/services/string-definitions';
import {validateString,} from 'src/utils/validators';
import {checkString,} from 'src/utils/checker';
import {getInputValue, handleInputChange,} from 'src/utils/helper';
import {createUbaDevice, updateUbaDevice,} from 'src/action-creators/UbaDevices';
import {setModal,} from 'src/actions/Auth';
import {updateCurrentUba,} from 'src/actions/UbaDevices';
import {addEditSettings, ubaChannel as channelPossibleValues,} from 'src/constants/unsystematic';

function AddEditUbaDevice() {

	const [serialError, setSerialError] = useState('');
	const [nameError, setNameError] = useState('');
	const [ubaChannelError, setUbaChannelError] = useState('');
	const [machineError, setMachineError] = useState('');
	const [portError, setPortError] = useState('');
	const [addressError, setAddressError] = useState('');
	const [testRunStage, setTestRunStage] = useState(0);

	const {openedModalType,} = useAuth();
	const {currentUba,} = useUbaDevices();
	const {machines,} = useSettings();

	let buttonText = getText('common.ADD');
	let title = getText('settingsPage.uba.ADD_UBA');
	if (openedModalType === addEditSettings.EDIT_UBA_DEVICE) {
		buttonText = getText('common.UPDATE');
		title = getText('settingsPage.uba.EDIT_UBA');
	}

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	const validateValue = (dataKey, dataValue) => {
		switch (dataKey) {
			case 'ubaSN': {
				return checkString(dataValue, 'common.UBA_S_N', setSerialError);
			}
			case 'name': {
				return checkString(dataValue, 'common.NAME', setNameError);
			}
			case 'ubaChannel': {
				return checkString(dataValue, 'common.CHANNEL', setUbaChannelError);
			}
			case 'machineMac': {
				return checkString(dataValue, 'common.LAB', setMachineError);
			}
			case 'comPort': {
				return checkString(dataValue, 'settingsPage.uba.PORT', setPortError);
			}
			case 'address': {
				return checkString(dataValue, 'settingsPage.uba.ADDRESS', setAddressError);
			}
			default:
				break;
		}
	}

	const handleChange = (dataKey, dataValue) => {
		validateValue(dataKey, dataValue);
		handleInputChange(ubaDevicesDispatch, updateCurrentUba, dataKey, dataValue);
	}

	const handleTestClick = () => {
		const isSerialValid = validateValue('ubaSN', currentUba.ubaSN);
		const isNameValid = validateValue('name', currentUba.name);
		const isUbaChannelValid = validateValue('ubaChannel', currentUba.ubaChannel);
		const isMachineValid = validateValue('machineMac', currentUba.machineMac);
		const isPortValid = validateValue('comPort', currentUba.comPort);
		const isAddressValid = validateValue('address', currentUba.address);

		if ((!isSerialValid && openedModalType === addEditSettings.ADD_UBA_DEVICE) ||
			!isMachineValid || !isPortValid || !isAddressValid) {
			return;
		}

		setTestRunStage(1);
		// TODO: Implement test run, instead of setTimeout mock.
		setTimeout(() => setTestRunStage(2), 2000);
	}

	const handleSubmitClick = () => {
		if (openedModalType === addEditSettings.EDIT_UBA_DEVICE) {
			updateUbaDevice(authDispatch, ubaDevicesDispatch, currentUba);
		} else {
			createUbaDevice(authDispatch, ubaDevicesDispatch, currentUba);
			
		}
		authDispatch(setModal(''));
	}

	return (
		<Box sx={{background: theme => theme.palette.grey[200], borderRadius: 2, width: '20vw',}}>
			<Typography level="title-lg" sx={{px: 3.5, py: 2,}}>
				{title}
			</Typography>

			<Stack alignItems="center" spacing={2} sx={{background: theme => theme.palette.grey[0], p: 2,}}>
				{(openedModalType === addEditSettings.ADD_UBA_DEVICE) &&
					<TextField
						fullWidth
						size="small"
						error={validateString(serialError)}
						label={getText('common.UBA_S_N')}
						value={getInputValue(currentUba, 'ubaSN')}
						onChange={event => handleChange('ubaSN', event.target.value)}
						helperText={serialError}
					/>
				}

				<TextField
					fullWidth
					size="small"
					error={validateString(nameError)}
					label={getText('common.NAME')}
					value={getInputValue(currentUba, 'name')}
					onChange={event => handleChange('name', event.target.value)}
					helperText={nameError}
				/>

				<FormControl fullWidth size="small" error={validateString(ubaChannelError)}>
					<InputLabel>
						{getText('common.CHANNEL')}
					</InputLabel>

					<Select
						disabled={openedModalType === addEditSettings.EDIT_UBA_DEVICE}
						label={getText('common.CHANNEL')}
						value={getInputValue(currentUba, 'ubaChannel')}
						onChange={event => handleChange('ubaChannel', event.target.value)}
						MenuProps={{
							PaperProps: {
								sx: {maxHeight: 200,}
							}
						}}
					>
						{Object.values(channelPossibleValues).map((channelValue, key) => (
							<MenuItem key={key} value={channelValue}>
								{channelValue}
							</MenuItem>
						))}
					</Select>

					<FormHelperText>
						{ubaChannelError}
					</FormHelperText>
				</FormControl>

				<FormControl fullWidth size="small" error={validateString(machineError)}>
					<InputLabel>
						{getText('common.LAB')}
					</InputLabel>

					<Select
						label={getText('common.LAB')}
						value={getInputValue(currentUba, 'machineMac')}
						onChange={event => handleChange('machineMac', event.target.value)}
						MenuProps={{
							PaperProps: {
								sx: {maxHeight: 200,}
							}
						}}
					>
						{machines?.map((machine, key) => (
							<MenuItem key={key} value={machine?.mac}>
								{machine?.name}
							</MenuItem>
						))}
					</Select>

					<FormHelperText>
						{machineError}
					</FormHelperText>
				</FormControl>

				<TextField
					fullWidth
					size="small"
					error={validateString(portError)}
					label={getText('settingsPage.uba.PORT')}
					value={getInputValue(currentUba, 'comPort')}
					onChange={event => handleChange('comPort', event.target.value)}
					helperText={portError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(addressError)}
					label={getText('settingsPage.uba.ADDRESS')}
					value={getInputValue(currentUba, 'address')}
					onChange={event => handleChange('address', event.target.value)}
					helperText={addressError}
				/>

				<Typography align="center" sx={{display: (testRunStage === 1) ? 'block' : 'none',}}>
					{getText('settingsPage.TEST_RUNNING')}
				</Typography>

				<Typography align="center" sx={{display: (testRunStage === 2) ? 'block' : 'none',}}>
					{getText('settingsPage.TEST_COMPLETED')}
				</Typography>
			</Stack>

			<Stack direction="row" justifyContent="space-between" sx={{
				mt: -2,
				p: 2,
				background: theme => theme.palette.grey[0],
				borderRadius: 2,
			}}>
				<Button variant="contained" onClick={handleTestClick} sx={{width: 100,}}>
					{getText('common.TEST')}
				</Button>

				<Button variant="contained" disabled={(testRunStage !== 2)} onClick={handleSubmitClick} sx={{width: 100,}}>
					{buttonText}
				</Button>
			</Stack>
		</Box>
	);
}

export default withModalView(AddEditUbaDevice);
