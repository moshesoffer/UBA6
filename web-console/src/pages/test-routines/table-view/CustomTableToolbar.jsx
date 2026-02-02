import React from 'react';
import PropTypes from 'prop-types';
import Button from '@mui/material/Button';
import Toolbar from '@mui/material/Toolbar';
import OutlinedInput from '@mui/material/OutlinedInput';
import InputLabel from '@mui/material/InputLabel';
import FormControl from '@mui/material/FormControl';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import {useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {getText,} from 'src/services/string-definitions';
import {handleFilters,} from 'src/utils/filtering';
import {LOCK_STATUS,pageStateList,} from 'src/constants/unsystematic';
import Typography from '@mui/material/Typography';
import {setState,} from 'src/actions/TestRoutines';

export default function CustomTableToolbar(props) {

	const {filters, setFilters, setPage,} = props;
	const {testName, batteryPN, cellPN, isLocked,} = filters;

	const testRoutinesDispatch = useTestRoutinesDispatch();

	const buttonClickHandler = () => {
		testRoutinesDispatch(setState(pageStateList.WIZARD_ONE));
	}

	const handleTestNameFilter = event => handleFilters('testName', event.target.value, setFilters, setPage);
	const handleBatteryPNFilter = event => handleFilters('batteryPN', event.target.value, setFilters, setPage);
	const handleCellPNFilter = event => handleFilters('cellPN', event.target.value, setFilters, setPage);
	const handleIsLockedFilter = event => handleFilters('isLocked', event.target.value, setFilters, setPage);

	return (
		<Toolbar style={{minHeight: '50px'}} sx={{
			display: 'flex',
			justifyContent: 'space-around',
			p: theme => theme.spacing(0, 1, 0, 3),
		}}>
			{getText('common.FILTER_BY')}

			<OutlinedInput
				placeholder={getText('common.TEST_NAME')}
				value={testName}
				onChange={handleTestNameFilter}
			/>

			<OutlinedInput
				placeholder={getText('testEditor.BATTERY_P_N')}
				value={batteryPN}
				onChange={handleBatteryPNFilter}
			/>

			<OutlinedInput
				placeholder={getText('testEditor.CELL_P_N')}
				value={cellPN}
				onChange={handleCellPNFilter}
			/>

			<FormControl size="small" sx={{width: '16%',}}>
				<InputLabel>
					{getText('mainPage.wizardZero.TEST_LOCK')}
				</InputLabel>

				<Select
					input={<OutlinedInput label={getText('mainPage.wizardZero.TEST_LOCK')}/>}
					value={isLocked}
					onChange={handleIsLockedFilter}
				>
					<MenuItem value="">
						<em>
							{getText('common.ALL')}
						</em>
					</MenuItem>
					{LOCK_STATUS.map((status, key) => (
						<MenuItem key={key} value={key}>
							{getText(`mainPage.wizardZero.${status.toUpperCase()}`)}
						</MenuItem>
					))}
				</Select>
			</FormControl>

			<Button variant="contained" onClick={buttonClickHandler}>
				<Typography>
					{getText('testEditor.NEW_TEST')}
				</Typography>
			</Button>
		</Toolbar>
	);
}

CustomTableToolbar.propTypes = {
	filters: PropTypes.object,
	setFilters: PropTypes.func,
	setPage: PropTypes.func,
};
