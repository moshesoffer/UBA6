import React from 'react';
import PropTypes from 'prop-types';

import Toolbar from '@mui/material/Toolbar';
import TextField from '@mui/material/TextField';
import FormControl from '@mui/material/FormControl';
import Select from "@mui/material/Select";
import MenuItem from "@mui/material/MenuItem";
import InputLabel from "@mui/material/InputLabel";

import {useReports,} from 'src/store/ReportsProvider';
import {useSettings,} from 'src/store/SettingsProvider';
import {getText,} from 'src/services/string-definitions';
import {handleFilters,} from 'src/utils/filtering';
import {DATE_RANGE,} from 'src/constants/unsystematic';

const width = {width: '14%',}

export default function CustomTableToolbar(props) {

	const {filters, setFilters, setPage,} = props;
	const {
		batteryPN,
		batterySN,
		testName,
		ubaSN,
		machineName,
		dateRange,
	} = filters;

	const {reports,} = useReports();
	const {machines,} = useSettings();

	let machineList = machines.map(item => item.name);
	machineList = [...new Set(machineList)];

	const handleBatteryPNFilter = event => handleFilters('batteryPN', event.target.value, setFilters, setPage);
	const handleBatterySNFilter = event => handleFilters('batterySN', event.target.value, setFilters, setPage);
	const handleTestNameFilter = event => handleFilters('testName', event.target.value, setFilters, setPage);
	const handleUbaFilter = event => handleFilters('ubaSN', event.target.value, setFilters, setPage);
	const handleMachineFilter = event => handleFilters('machineName', event.target.value, setFilters, setPage);
	const handleDateRangeFilter = event => handleFilters('dateRange', event.target.value, setFilters, setPage);

	return (
		<Toolbar style={{minHeight: '50px'}} sx={{
			display: 'flex',
			justifyContent: 'space-between',
		}}>
			<TextField
				size="small"
				label={getText('testEditor.BATTERY_P_N')}
				value={batteryPN}
				onChange={handleBatteryPNFilter}
				sx={width}
			/>

			<TextField
				size="small"
				label={getText('reportsPage.BATTERY_S_N')}
				value={batterySN}
				onChange={handleBatterySNFilter}
				sx={width}
			/>

			<TextField
				size="small"
				label={getText('common.TEST_NAME')}
				value={testName}
				onChange={handleTestNameFilter}
				sx={width}
			/>

			<TextField
				size="small"
				label={getText('reportsPage.UBA')}
				value={ubaSN}
				onChange={handleUbaFilter}
				sx={width}
			/>

			<FormControl size="small" sx={width}>
				<InputLabel>
					{getText('common.LAB')}
				</InputLabel>

				<Select
					label={getText('common.LAB')}
					value={machineName}
					onChange={handleMachineFilter}
					MenuProps={{
						PaperProps: {
							sx: {maxHeight: 200,}
						}
					}}
				>
					<MenuItem value="">
						{getText('common.DESELECT')}
					</MenuItem>
					{machineList.map((option, key) => (
						<MenuItem key={key} value={option}>
							{option}
						</MenuItem>
					))}
				</Select>
			</FormControl>

			<FormControl size="small" sx={width}>
				<InputLabel>
					{getText('reportsPage.DATE_RANGE')}
				</InputLabel>

				<Select
					label={getText('reportsPage.DATE_RANGE')}
					value={dateRange}
					onChange={handleDateRangeFilter}
				>
					<MenuItem value="">
						{getText('common.ALL')}
					</MenuItem>
					{DATE_RANGE.map((option, key) => (
						<MenuItem key={key} value={option}>
							{getText(`reportsPage.DATE_RANGE_VALUES.${option}`)}
						</MenuItem>
					))}
				</Select>
			</FormControl>
		</Toolbar>
	);
}

CustomTableToolbar.propTypes = {
	filters: PropTypes.object,
	setFilters: PropTypes.func,
	setPage: PropTypes.func,
};
