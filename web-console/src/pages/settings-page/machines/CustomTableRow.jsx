import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';
import { deleteMachine } from 'src/action-creators/Settings';
import { useAuthDispatch } from 'src/store/AuthProvider';
import { useSettingsDispatch } from 'src/store/SettingsProvider';
import IconButton from '@mui/material/IconButton';
import DeleteIcon from '@mui/icons-material/Delete';
import {useUbaDevices, } from 'src/store/UbaDevicesProvider';

export default function CustomTableRow(props) {

	const {row,} = props;
	const authDispatch = useAuthDispatch();
	const settingsDispatch = useSettingsDispatch();
	const {ubaDevices,} = useUbaDevices();

	const handleDeleteClick = () => {
		let choice = confirm('Do you want to delete this Station?');
		if(choice === true) {
			deleteMachine(authDispatch, settingsDispatch, row?.mac);
			return true;
		}
		return false;
	};

	const getActions = () => {
		const ubaDeviceCount = ubaDevices.filter(uba => uba.machineMac === row.mac).length;
		if (ubaDeviceCount === 0) {
			return (
				<IconButton size="small" color="error" onClick={handleDeleteClick}>
					<DeleteIcon/>
				</IconButton>
			);
		}
	};

	return (
		<TableRow hover tabIndex={-1}>
			<TableCell>
				{row?.name}
			</TableCell>

			<TableCell>
				{row?.ip}
			</TableCell>

			<TableCell>
				{getActions()}
			</TableCell>
		</TableRow>
	);
}
