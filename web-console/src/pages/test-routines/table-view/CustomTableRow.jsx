import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';
import IconButton from '@mui/material/IconButton';
import BorderColorIcon from '@mui/icons-material/BorderColor';
import ContentCopyIcon from '@mui/icons-material/ContentCopy';
import ButtonGroup from '@mui/material/ButtonGroup';
import DeleteIcon from '@mui/icons-material/Delete';
import LockIcon from '@mui/icons-material/Lock';
import LockOpenIcon from '@mui/icons-material/LockOpen';

import {pageStateList,} from 'src/constants/unsystematic';
import {useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {deleteTestRoutine,} from 'src/action-creators/TestRoutines';
import {setState,} from 'src/actions/TestRoutines';
import {fillTestRoutine,} from 'src/utils/helper';

export default function CustomTableRow({row}) {

	const authDispatch = useAuthDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const handleButtonClick = isDup => {
		fillTestRoutine(row, isDup, '', testRoutinesDispatch);
		testRoutinesDispatch(setState(pageStateList.WIZARD_ONE));
	}

	const handleTestDelete = () => {
		let choice = confirm('Do you want to delete this test routine?');
		if(choice === true) {
			deleteTestRoutine(authDispatch, testRoutinesDispatch, row?.id);
			return true;
		}
		return false;
	}

	return (
		<TableRow hover tabIndex={-1}>
			<TableCell>
				{row?.testName}
			</TableCell>

			<TableCell>
				{row?.batteryPN}
			</TableCell>

			<TableCell>
				{row?.isLocked ?
					<LockIcon fontSize="small" color="error"/> :
					<LockOpenIcon fontSize="small" color="primary"/>
				}
			</TableCell>

			<TableCell>
				{`${row?.itemPN} (${row?.chemistry} ${row?.manufacturer})`}
			</TableCell>

			<TableCell>
				{row?.noCellSerial}
			</TableCell>

			<TableCell>
				{row?.noCellParallel}
			</TableCell>

			<TableCell>
				<ButtonGroup>
					<IconButton size="small" color="primary" onClick={() => handleButtonClick(false)}>
						<BorderColorIcon/>
					</IconButton>

					<IconButton size="small" color="primary" onClick={() => handleButtonClick(true)}>
						<ContentCopyIcon/>
					</IconButton>

					<IconButton size="small" color="error" onClick={handleTestDelete}>
						<DeleteIcon/>
					</IconButton>
					</ButtonGroup>
			</TableCell>
		</TableRow>
	);
}
