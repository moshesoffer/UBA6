import React from 'react';
import PropTypes from 'prop-types';

import Grid from '@mui/material/Grid';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import Select from '@mui/material/Select';
import OutlinedInput from '@mui/material/OutlinedInput';
import MenuItem from '@mui/material/MenuItem';

import {getText,} from 'src/services/string-definitions';
import {handleFilters,} from 'src/utils/filtering';
import {LOCK_STATUS,} from 'src/constants/unsystematic';

export default function CustomTableToolbar(props) {

	const {filters, setFilters, setPage,} = props;
	const {testName, customer, batteryPN, cellPN, noCellSerial, isLocked,} = filters;

	const handleTestNameFilter = event => handleFilters('testName', event.target.value, setFilters, setPage);
	const handleCustomerFilter = event => handleFilters('customer', event.target.value, setFilters, setPage);
	const handleBatteryPNFilter = event => handleFilters('batteryPN', event.target.value, setFilters, setPage);
	const handleCellPNFilter = event => handleFilters('cellPN', event.target.value, setFilters, setPage);
	const handleCellsInSerialFilter = event => handleFilters('noCellSerial', event.target.value, setFilters, setPage);
	const handleIsLockedFilter = event => handleFilters('isLocked', event.target.value, setFilters, setPage);

	return (
		<Grid container spacing={2} sx={{px: 6, py: 2}}>
			<Grid item lg={2} sx={{display: 'grid',}}>
				<Typography variant="subtitle1" sx={{my: 'auto',}}>
					{getText('mainPage.wizardOne.TEST_ROUTINES_FILTER')}
				</Typography>
			</Grid>

			<Grid item lg={4}>
				<TextField
					fullWidth
					size="small"
					label={getText('common.TEST_NAME')}
					value={testName}
					onChange={handleTestNameFilter}
				/>
			</Grid>

			<Grid item lg={2} sx={{display: 'grid',}}>
				<Typography variant="subtitle1" align="center" sx={{my: 'auto',}}>
					{getText('mainPage.wizardOne.AND_PARENTHESES')}
				</Typography>
			</Grid>

			<Grid item lg={4}>
				<TextField
					fullWidth
					size="small"
					label={getText('mainPage.wizardOne.CUSTOMER')}
					value={customer}
					onChange={handleCustomerFilter}
				/>
			</Grid>

			<Grid item lg={3}>
				<TextField
					fullWidth
					size="small"
					label={getText('testEditor.BATTERY_P_N')}
					value={batteryPN}
					onChange={handleBatteryPNFilter}
				/>
			</Grid>

			<Grid item lg={3}>
				<TextField
					fullWidth
					size="small"
					label={getText('testEditor.CELL_P_N')}
					value={cellPN}
					onChange={handleCellPNFilter}
				/>
			</Grid>

			<Grid item lg={3}>
				<TextField
					fullWidth
					size="small"
					label={getText('testEditor.NO_CELLS_IN_SERIAL')}
					value={noCellSerial}
					onChange={handleCellsInSerialFilter}
				/>
			</Grid>

			<Grid item lg={3}>
				<FormControl fullWidth size="small">
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
			</Grid>
		</Grid>
	);
}

CustomTableToolbar.propTypes = {
	filters: PropTypes.object,
	setFilters: PropTypes.func,
	setPage: PropTypes.func,
};
