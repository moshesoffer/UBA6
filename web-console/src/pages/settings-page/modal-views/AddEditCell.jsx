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
import {useSettings, useSettingsDispatch,} from 'src/store/SettingsProvider';
import withModalView from 'src/components/withModalView';
import {getText,} from 'src/services/string-definitions';
import {validateString,} from 'src/utils/validators';
import {checkString, checkNumber,} from 'src/utils/checker';
import {getInputValue, handleInputChange,} from 'src/utils/helper';
import {setModal,} from 'src/actions/Auth';
import {createCell, updateCell,} from 'src/action-creators/Settings';
import {updateCurrentCell,} from 'src/actions/Settings';
import {addEditSettings,} from 'src/constants/unsystematic';

const chargeOptions = ['Primary', 'Secondary',];

function AddEditCell() {

	const [chemistryError, setChemistryError] = useState('');
	const [manufacturerError, setManufacturerError] = useState('');
	const [itemPNError, setItemPNError] = useState('');
	const [minVoltageError, setMinVoltageError] = useState('');
	const [nomVoltageError, setNomVoltageError] = useState('');
	const [maxVoltageError, setMaxVoltageError] = useState('');
	const [minCapacityError, setMinCapacityError] = useState('');
	const [nomCapacityError, setNomCapacityError] = useState('');
	const [minTempError, setMinTempError] = useState('');
	const [maxTempError, setMaxTempError] = useState('');
	const [chargeOptionError, setChargeOptionError] = useState('');

	const {openedModalType,} = useAuth();
	const {currentCell,} = useSettings();

	let buttonText = getText('common.ADD');
	let title = getText('settingsPage.cell.ADD_CELL');
	if (openedModalType === addEditSettings.EDIT_CELL) {
		buttonText = getText('common.UPDATE');
		title = getText('settingsPage.cell.EDIT_CELL');
	}

	const authDispatch = useAuthDispatch();
	const settingsDispatch = useSettingsDispatch();

	const validateValue = (dataKey, dataValue) => {
		switch (dataKey) {
			case 'chemistry': {
				return checkString(dataValue, 'settingsPage.cell.CHEMISTRY', setChemistryError);
			}
			case 'manufacturer': {
				return checkString(dataValue, 'settingsPage.cell.MANUFACTURER', setManufacturerError);
			}
			case 'itemPN': {
				return checkString(dataValue, 'settingsPage.cell.ITEM_P_N', setItemPNError);
			}
			case 'minVoltage': {
				return checkNumber(dataValue, false, 'settingsPage.cell.MIN_VOLTAGE', setMinVoltageError);
			}
			case 'nomVoltage': {
				return checkNumber(dataValue, false, 'settingsPage.cell.NOM_VOLTAGE', setNomVoltageError);
			}
			case 'maxVoltage': {
				return checkNumber(dataValue, false, 'settingsPage.cell.MAX_VOLTAGE', setMaxVoltageError);
			}
			case 'minCapacity': {
				return checkNumber(dataValue, false, 'settingsPage.cell.MIN_CAPACITY', setMinCapacityError);
			}
			case 'nomCapacity': {
				return checkNumber(dataValue, false, 'settingsPage.cell.NOM_CAPACITY', setNomCapacityError);
			}
			case 'minTemp': {
				return checkNumber(dataValue, true, 'settingsPage.cell.MIN_TEMP_2', setMinTempError);
			}
			case 'maxTemp': {
				return checkNumber(dataValue, false, 'settingsPage.cell.MAX_TEMP_2', setMaxTempError);
			}
			case 'chargeOption': {
				return checkString(dataValue, 'settingsPage.cell.CHARGE_OPTION', setChargeOptionError);
			}
			default:
				break;
		}
	}

	const handleChange = (dataKey, dataValue) => {
		validateValue(dataKey, dataValue);
		handleInputChange(settingsDispatch, updateCurrentCell, dataKey, dataValue);
	}

	const handleSubmitClick = () => {
		const isChemistryValid = validateValue('chemistry', currentCell.chemistry);
		const isManufacturerValid = validateValue('manufacturer', currentCell.manufacturer);
		const isItemPNValid = validateValue('itemPN', currentCell.itemPN);
		const isMinVoltageValid = validateValue('minVoltage', currentCell.minVoltage);
		const isNomVoltageValid = validateValue('nomVoltage', currentCell.nomVoltage);
		const isMaxVoltageValid = validateValue('maxVoltage', currentCell.maxVoltage);
		const isMinCapacityValid = validateValue('minCapacity', currentCell.minCapacity);
		const isNomCapacityValid = validateValue('nomCapacity', currentCell.nomCapacity);
		const isMinTempValid = validateValue('minTemp', currentCell.minTemp);
		const isMaxTempValid = validateValue('maxTemp', currentCell.maxTemp);
		const isChargeOptionValid = validateValue('chargeOption', currentCell.chargeOption);

		if (
			!isChemistryValid ||
			!isManufacturerValid ||
			(!isItemPNValid && openedModalType === addEditSettings.ADD_CELL) ||
			!isMinVoltageValid ||
			!isNomVoltageValid ||
			!isMaxVoltageValid ||
			!isMinCapacityValid ||
			!isNomCapacityValid ||
			!isMinTempValid ||
			!isMaxTempValid ||
			!isChargeOptionValid
		) {
			return;
		}

		if (openedModalType === addEditSettings.EDIT_CELL) {
			updateCell(authDispatch, settingsDispatch, currentCell);
			authDispatch(setModal(''));
			return;
		}

		createCell(authDispatch, settingsDispatch, currentCell);
		authDispatch(setModal(''));
	}

	return (
		<Box sx={{background: theme => theme.palette.grey[200], borderRadius: 2, width: '22vw',}}>
			<Typography level="title-lg" sx={{px: 3.5, py: 2,}}>
				{title}
			</Typography>

			<Stack alignItems="center" spacing={2} sx={{
				background: theme => theme.palette.grey[0],
				p: 2,
				maxHeight: 'calc(98vh - 108px)',
				overflowY: 'auto',
			}}>
				<TextField
					fullWidth
					size="small"
					error={validateString(chemistryError)}
					label={getText('settingsPage.cell.CHEMISTRY')}
					value={getInputValue(currentCell, 'chemistry')}
					onChange={event => handleChange('chemistry', event.target.value)}
					helperText={chemistryError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(manufacturerError)}
					label={getText('settingsPage.cell.MANUFACTURER')}
					value={getInputValue(currentCell, 'manufacturer')}
					onChange={event => handleChange('manufacturer', event.target.value)}
					helperText={manufacturerError}
				/>

				{(openedModalType === addEditSettings.ADD_CELL) &&
					<TextField
						fullWidth
						size="small"
						error={validateString(itemPNError)}
						label={getText('settingsPage.cell.ITEM_P_N')}
						value={getInputValue(currentCell, 'itemPN')}
						onChange={event => handleChange('itemPN', event.target.value)}
						helperText={itemPNError}
					/>
				}

				<TextField
					fullWidth
					size="small"
					error={validateString(minVoltageError)}
					label={getText('settingsPage.cell.MIN_VOLT')}
					value={getInputValue(currentCell, 'minVoltage')}
					onChange={event => handleChange('minVoltage', event.target.value)}
					helperText={minVoltageError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(nomVoltageError)}
					label={getText('settingsPage.cell.NOM_VOLT')}
					value={getInputValue(currentCell, 'nomVoltage')}
					onChange={event => handleChange('nomVoltage', event.target.value)}
					helperText={nomVoltageError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(maxVoltageError)}
					label={getText('settingsPage.cell.MAX_VOLT')}
					value={getInputValue(currentCell, 'maxVoltage')}
					onChange={event => handleChange('maxVoltage', event.target.value)}
					helperText={maxVoltageError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(minCapacityError)}
					label={getText('settingsPage.cell.MIN_CAPAC')}
					value={getInputValue(currentCell, 'minCapacity')}
					onChange={event => handleChange('minCapacity', event.target.value)}
					helperText={minCapacityError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(nomCapacityError)}
					label={getText('settingsPage.cell.NOM_CAPAC')}
					value={getInputValue(currentCell, 'nomCapacity')}
					onChange={event => handleChange('nomCapacity', event.target.value)}
					helperText={nomCapacityError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(minTempError)}
					label={getText('settingsPage.cell.MIN_TEMPERATURE')}
					value={getInputValue(currentCell, 'minTemp')}
					onChange={event => handleChange('minTemp', event.target.value)}
					helperText={minTempError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(maxTempError)}
					label={getText('settingsPage.cell.MAX_TEMPERATURE')}
					value={getInputValue(currentCell, 'maxTemp')}
					onChange={event => handleChange('maxTemp', event.target.value)}
					helperText={maxTempError}
				/>

				<FormControl fullWidth size="small" error={validateString(chargeOptionError)}>
					<InputLabel>
						{getText('settingsPage.cell.CHARGE_OPTION')}
					</InputLabel>

					<Select
						label={getText('settingsPage.cell.CHARGE_OPTION')}
						value={getInputValue(currentCell, 'chargeOption')}
						onChange={event => handleChange('chargeOption', event.target.value)}
						MenuProps={{
							PaperProps: {
								sx: {maxHeight: 200,}
							}
						}}
					>
						{chargeOptions.map((chargeOption, key) => (
							<MenuItem key={key} value={chargeOption}>
								{chargeOption}
							</MenuItem>
						))}
					</Select>

					<FormHelperText>
						{chargeOptionError}
					</FormHelperText>
				</FormControl>
			</Stack>

			<Stack direction="row" justifyContent="center" sx={{
				mt: -2,
				p: 2,
				background: theme => theme.palette.grey[0],
				borderRadius: 2,
			}}>
				<Button variant="contained" onClick={handleSubmitClick} sx={{width: 100,}}>
					{buttonText}
				</Button>
			</Stack>
		</Box>
	);
}

export default withModalView(AddEditCell);
