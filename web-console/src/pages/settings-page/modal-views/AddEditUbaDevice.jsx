import { useState, } from 'react';

import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import CircularProgress from '@mui/material/CircularProgress';
import FormControl from '@mui/material/FormControl';
import FormHelperText from '@mui/material/FormHelperText';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';
import Stack from '@mui/material/Stack';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import { createUbaDevice, queryUbaDevice, updateUbaDevice, } from 'src/action-creators/UbaDevices';
import { setModal } from 'src/actions/Auth';
import { updateCurrentUba, } from 'src/actions/UbaDevices';
import withModalView from 'src/components/withModalView';
import { addEditSettings, ubaChannel as channelPossibleValues, } from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useAuth, useAuthDispatch, } from 'src/store/AuthProvider';
import { useSettings, } from 'src/store/SettingsProvider';
import { useUbaDevices, useUbaDevicesDispatch } from 'src/store/UbaDevicesProvider';
import { checkString, } from 'src/utils/checker';
import { getInputValue, handleInputChange, } from 'src/utils/helper';
import { validateString, } from 'src/utils/validators';
import Tooltip from '@mui/material/Tooltip';

function AddEditUbaDevice() {

	const [serialError, setSerialError] = useState('');
	const [nameError, setNameError] = useState('');
	const [ubaChannelError, setUbaChannelError] = useState('');
	const [machineError, setMachineError] = useState('');
	const [portError, setPortError] = useState('');
	const [addressError, setAddressError] = useState('');
	
	const [testRunStage, setTestRunStage] = useState(0);
	const [ubaDeviceQueryAnswer, setUbaDeviceQueryAnswer] = useState({});
	const [queryError, setQueryError] = useState('');
	const [ubaName, setUbaName] = useState('');

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

	const handleTestClick = async () => {
		setTestRunStage(1);
		setQueryError('');
		const isMachineValid = validateValue('machineMac', currentUba.machineMac);
		const isPortValid = validateValue('comPort', currentUba.comPort);
		const isAddressValid = validateValue('address', currentUba.address);
		if (!isMachineValid || !isPortValid || !isAddressValid) {
			setTestRunStage(0);
			return;
		}

		const queryObj = {machineMac: currentUba.machineMac, comPort: currentUba.comPort, address: currentUba.address,};
		if( openedModalType === addEditSettings.EDIT_UBA_DEVICE) {
			queryObj.ubaSN = currentUba.ubaSN;
			queryObj.ubaChannel = currentUba.ubaChannel;
		}
		const res = await queryUbaDevice(false, queryObj);
		if(res?.error) {
			//authDispatch(setNotification({message: res.error,}));
			setQueryError(res.error);
			setUbaDeviceQueryAnswer({});
			setTestRunStage(0);
		} else {
			setQueryError('');
			setUbaDeviceQueryAnswer(res);
			setTestRunStage(2);
			setUbaName(res?.name || '');
		}
		
	}

	const handleSubmitClick = () => {
		const { action, actionResult, ...ubaToAddEdit} = { ...ubaDeviceQueryAnswer}
		ubaToAddEdit.name = ubaName;
		if (openedModalType === addEditSettings.EDIT_UBA_DEVICE) {
			updateUbaDevice(authDispatch, ubaDevicesDispatch, ubaToAddEdit);
		} else {
			createUbaDevice(authDispatch, ubaDevicesDispatch, ubaToAddEdit);
		}
		authDispatch(setModal(''));
	}

	return (
		<Box sx={{background: theme => theme.palette.grey[200], borderRadius: 2, width: '20vw',position: "relative",}}>
			<Typography level="title-lg" sx={{px: 3.5, py: 2,}}>
				{title}
			</Typography>

			<Stack alignItems="center" spacing={2} sx={{background: theme => theme.palette.grey[0], p: 2,}}>
				<TextField
					fullWidth
					disabled={true}
					size="small"
					error={validateString(serialError)}
					label={getText('common.UBA_S_N')}
					value={ubaDeviceQueryAnswer.ubaSN || getInputValue(currentUba, 'ubaSN')}
					helperText={serialError}
				/>

				<FormControl fullWidth size="small" error={validateString(ubaChannelError)}>
					<InputLabel>
						{getText('common.CHANNEL')}
					</InputLabel>

					<Select
						disabled={true}
						label={getText('common.CHANNEL')}
						value={ubaDeviceQueryAnswer.ubaChannel || getInputValue(currentUba, 'ubaChannel')}
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

				<Tooltip title={testRunStage === 1 || !ubaDeviceQueryAnswer.ubaSN || !ubaDeviceQueryAnswer.name || !ubaDeviceQueryAnswer.ubaChannel ? "This value can be edited only after a successful test" : ""} arrow>
					<TextField
						fullWidth
						size="small"
						disabled={testRunStage === 1 || !ubaDeviceQueryAnswer.ubaSN || !ubaDeviceQueryAnswer.name || !ubaDeviceQueryAnswer.ubaChannel}
						error={validateString(nameError)}
						label={getText('common.NAME')}
						value={ubaName || getInputValue(currentUba, 'name')}
						onChange={event => { setUbaName(event.target.value); setNameError(''); } }
						helperText={nameError}
					/>
				</Tooltip>
				
				<FormControl fullWidth size="small" error={validateString(machineError)}>
					<InputLabel>
						{getText('common.LAB')}
					</InputLabel>

					<Select
						label={getText('common.LAB')}
						value={getInputValue(currentUba, 'machineMac')}
						onChange={event => {setTestRunStage(0); setQueryError(''); handleChange('machineMac', event.target.value) } }
						disabled={testRunStage === 1}
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
					disabled={testRunStage === 1}
					error={validateString(portError)}
					label={getText('settingsPage.uba.PORT')}
					value={getInputValue(currentUba, 'comPort')}
					onChange={event => {setTestRunStage(0); setQueryError(''); handleChange('comPort', event.target.value) } }
					helperText={portError}
				/>

				<TextField
					fullWidth
					size="small"
					disabled={testRunStage === 1}
					error={validateString(addressError)}
					label={getText('settingsPage.uba.ADDRESS')}
					value={getInputValue(currentUba, 'address')}
					onChange={event => { setTestRunStage(0); setQueryError(''); handleChange('address', event.target.value) } }
					helperText={addressError}
				/>

				{ queryError !== '' &&
					<Typography
						fullWidth
						size="small"
						color="error"
						sx={{paddingLeft: 1,paddingRight: 1,}}
					>
						{queryError === 'Timeout Reached' ? getText('settingsPage.uba.UBA_DEVICE_NO_RESPONSE') : queryError}
					</Typography>
				}

			</Stack>

			<Stack direction="row" justifyContent="space-between" sx={{
				mt: -2,
				p: 2,
				background: theme => theme.palette.grey[0],
				borderRadius: 2,
			}}>
				<Button variant="contained" disabled={testRunStage !== 0} onClick={handleTestClick} sx={{width: 100,height: 60,}}>
					<Typography>{ getText('common.TEST') }</Typography>
					{
						testRunStage === 1 &&
						<Box sx={{ paddingLeft: '10px', marginTop: '5px' }}>
							<CircularProgress
								sx={{
									color: "blue",
									animationDuration: "5s", 
									marginTop: 0.5,
									
								}}
								size={25}
							/>
						</Box>
					}
					
				</Button>

				<Button variant="contained" disabled={(testRunStage !== 2)} onClick={handleSubmitClick} sx={{width: 100,}}>
					{buttonText}
				</Button>
			</Stack>

			{testRunStage === 1 && (
				<Box
					sx={{
						position: "absolute",
						inset: 0, // top:0, right:0, bottom:0, left:0
						display: "flex",
						justifyContent: "center",
						alignItems: "center",
						backgroundColor: "rgba(255,255,255,0.6)", // dim background
						borderRadius: 2,
						zIndex: 10,
					}}
				>
					<CircularProgress
						sx={{ color: "blue", animationDuration: "5s" }}
						size={50}
					/>
				</Box>
			)}
		</Box>
	);
}

export default withModalView(AddEditUbaDevice);
