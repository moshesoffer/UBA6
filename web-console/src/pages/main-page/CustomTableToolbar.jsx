import React from 'react';
import PropTypes from 'prop-types';

import Toolbar from '@mui/material/Toolbar';
import OutlinedInput from '@mui/material/OutlinedInput';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import InputLabel from '@mui/material/InputLabel';
import FormControl from '@mui/material/FormControl';

import {useUbaDevices,} from 'src/store/UbaDevicesProvider';
import {getText,} from 'src/services/string-definitions';
import {getMachines,} from 'src/utils/helper';
import {category,} from 'src/constants/unsystematic';
import {handleFilters,} from 'src/utils/filtering';

export const STATE_OPTIONS = [
	{
		label: 'Standby',
		value: category.STANDBY,
	},
	{
		label: 'Running',
		value: category.RUNNING,
	},
];

export default function CustomTableToolbar(props) {

	const {filters, setFilters, setPage,} = props;

	const {ubaDevices,} = useUbaDevices();
	const machineNames = getMachines(ubaDevices);

	const {ubaSN, batteryPN, status, machineName,} = filters;

	const handleSerialFilter = event => handleFilters('ubaSN', event.target.value, setFilters, setPage);
	const handleBatteryPNFilter = event => handleFilters('batteryPN', event.target.value, setFilters, setPage);
	const handleStatusFilter = event => handleFilters('status', event.target.value, setFilters, setPage);
	const handleMachineFilter = event => handleFilters('machineName', event.target.value, setFilters, setPage);

	return (
		<Toolbar style={{minHeight: '50px'}} sx={{
			display: 'flex',
			justifyContent: 'space-around',
			p: theme => theme.spacing(0, 1, 0, 3),
		}}>
			<OutlinedInput placeholder={getText('common.UBA_S_N')} value={ubaSN} onChange={handleSerialFilter}/>

			<OutlinedInput placeholder={getText('mainPage.BATTERY_P_N_FILTER')} value={batteryPN} onChange={handleBatteryPNFilter}/>

			<FormControl size="small" sx={{flexShrink: 0, width: {xs: 1, md: 200},}}>
				<InputLabel>
					{getText('common.STATUS')}
				</InputLabel>

				<Select
					input={<OutlinedInput label={getText('common.STATUS')}/>}
					value={status}
					onChange={handleStatusFilter}
				>
					<MenuItem value="">
						<em>
							{getText('common.ALL')}
						</em>
					</MenuItem>
					{STATE_OPTIONS.map((option, key) => (
						<MenuItem key={key} value={option.value}>
							{option.label}
						</MenuItem>
					))}
				</Select>
			</FormControl>

			<FormControl size="small" sx={{flexShrink: 0, width: {xs: 1, md: 200},}}>
				<InputLabel>
					{getText('mainPage.LAB_LOCATION')}
				</InputLabel>

				<Select
					input={<OutlinedInput label={getText('mainPage.LAB_LOCATION')}/>}
					value={machineName}
					onChange={handleMachineFilter}
				>
					<MenuItem value="">
						<em>
							{getText('common.ALL')}
						</em>
					</MenuItem>
					{machineNames.map((option, key) => (
						<MenuItem key={key} value={option}>
							{option}
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
