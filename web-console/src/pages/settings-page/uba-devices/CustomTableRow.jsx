import BorderColorIcon from '@mui/icons-material/BorderColor';
import DeleteIcon from '@mui/icons-material/Delete';
import IconButton from '@mui/material/IconButton';
import TableCell from '@mui/material/TableCell';
import TableRow from '@mui/material/TableRow';
import Typography from '@mui/material/Typography';
import { deleteUbaDevice, } from 'src/action-creators/UbaDevices';
import { setModal } from 'src/actions/Auth';
import { setCurrentUba, } from 'src/actions/UbaDevices';
import { addEditSettings, isTestRunning, ubaChannel } from 'src/constants/unsystematic';
import { useAuthDispatch, } from 'src/store/AuthProvider';
import { useUbaDevices, useUbaDevicesDispatch } from 'src/store/UbaDevicesProvider';
import { isOtherChannelFree } from 'src/utils/helper';
import { getText, } from 'src/services/string-definitions';

export default function CustomTableRow(props) {

	const {row,} = props;
	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const {ubaDevices} = useUbaDevices();

	const handleEditClick = () => {
		ubaDevicesDispatch(setCurrentUba(row));
		authDispatch(setModal(addEditSettings.EDIT_UBA_DEVICE));
	};

	const handleDeleteClick = () => {
		let choice = confirm('Do you want to delete this UBA device?');
		if(choice === true) {
			deleteUbaDevice(authDispatch, ubaDevicesDispatch, row?.ubaSN);
			return true;
		}
		return false;
	};

	const isAllowedDeletionAndEdit = () => {
		const currentUbaNotRunning = !isTestRunning(row?.status);
		if(row.ubaChannel === ubaChannel.AB){
			const isAnotherChannelFree = isOtherChannelFree(ubaDevices, row);
			return currentUbaNotRunning && isAnotherChannelFree;
		}
		return currentUbaNotRunning;
	}
	const getActions = () => {
		if(isAllowedDeletionAndEdit()){
			return (
				<>
					<IconButton size="small" color="primary" onClick={handleEditClick}>
						<BorderColorIcon/>
					</IconButton>
					<IconButton size="small" color="error" onClick={handleDeleteClick}>
						<DeleteIcon/>
					</IconButton>
				</>
			);
		} else {
			return (
				<Typography fontSize={12} color="primary">
					{getText('common.TEST_CURRENTLY_RUNNING')}
				</Typography>
			);
		}
	};

	return (
		<TableRow hover tabIndex={-1}>
			<TableCell>
				{row?.ubaSN}
			</TableCell>

			<TableCell>
				{row?.ubaChannel}
			</TableCell>

			<TableCell>
				{row?.name}
			</TableCell>

			<TableCell>
				{row?.machineName}
			</TableCell>

			<TableCell>
				{row?.comPort}
			</TableCell>

			<TableCell>
				{row?.address}
			</TableCell>

			<TableCell>
				{getActions()}
			</TableCell>
		</TableRow>
	);
}
