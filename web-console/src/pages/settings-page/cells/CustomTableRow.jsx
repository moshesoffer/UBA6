import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';
import IconButton from '@mui/material/IconButton';
import DeleteIcon from '@mui/icons-material/Delete';
import BorderColorIcon from '@mui/icons-material/BorderColor';

import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useSettingsDispatch,} from 'src/store/SettingsProvider';
import {setModal,} from 'src/actions/Auth';
import {setCurrentCell,} from 'src/actions/Settings';
import {deleteCell,} from 'src/action-creators/Settings';
import {addEditSettings,} from 'src/constants/unsystematic';
import {validateNumber,} from 'src/utils/validators';

export default function CustomTableRow(props) {

	const {row,} = props;
	const authDispatch = useAuthDispatch();
	const settingsDispatch = useSettingsDispatch();

	const handleEditClick = () => {
		settingsDispatch(setCurrentCell(row));
		authDispatch(setModal(addEditSettings.EDIT_CELL));
	};

	const handleDeleteClick = () => {
		let choice = confirm('Do you want to delete this Cell P/N?');
		if(choice === true) {
			deleteCell(authDispatch, settingsDispatch, row?.itemPN);
			return true;
		}
		return false;
	};

	return (
		<TableRow hover tabIndex={-1}>

			<TableCell>
				{row?.chemistry}
			</TableCell>

			<TableCell>
				{row?.manufacturer}
			</TableCell>

			<TableCell>
				{row?.itemPN}
			</TableCell>

			<TableCell>
				{validateNumber(Number(row?.minVoltage)) ? Number(row.minVoltage).toFixed(2) : 'N/A'}
			</TableCell>

			<TableCell>
				{validateNumber(Number(row?.nomVoltage)) ? Number(row.nomVoltage).toFixed(2) : 'N/A'}
			</TableCell>

			<TableCell>
				{validateNumber(Number(row?.maxVoltage)) ? Number(row.maxVoltage).toFixed(2) : 'N/A'}
			</TableCell>

			<TableCell>
				{parseInt(row?.minCapacity)}
			</TableCell>

			<TableCell>
				{parseInt(row?.nomCapacity)}
			</TableCell>

			<TableCell>
				{validateNumber(Number(row?.minTemp)) ? Number(row.minTemp).toFixed(1) : 'N/A'}
			</TableCell>

			<TableCell>
				{validateNumber(Number(row?.maxTemp)) ? Number(row.maxTemp).toFixed(1) : 'N/A'}
			</TableCell>

			<TableCell>
				{row?.chargeOption}
			</TableCell>

			<TableCell>
				<IconButton size="small" color="primary" onClick={handleEditClick}>
					<BorderColorIcon/>
				</IconButton>

				<IconButton size="small" color="error" onClick={handleDeleteClick}>
					<DeleteIcon/>
				</IconButton>
			</TableCell>
		</TableRow>
	);
}
