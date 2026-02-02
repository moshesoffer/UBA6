import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';
import Radio from '@mui/material/Radio';
import LockIcon from '@mui/icons-material/Lock';
import LockOpenIcon from '@mui/icons-material/LockOpen';

import {validateObject,} from 'src/utils/validators';

export default function CustomTableRow(props) {

	const {row, selectedTest, setSelectedTest, isLocked,} = props;

	return (
		<TableRow hover tabIndex={-1}>
			<TableCell>
				<Radio
					checked={validateObject(selectedTest) && (selectedTest?.id?.toString() === row?.id?.toString())}
					value={JSON.stringify(row)}
					onChange={event => setSelectedTest(JSON.parse(event.target.value))}/>
			</TableCell>

			<TableCell>
				{row?.testName}
			</TableCell>

			<TableCell>
				{row?.isLocked ?
					<LockIcon fontSize="small" color="error"/> :
					<LockOpenIcon fontSize="small" color="primary"/>
				}
			</TableCell>

			<TableCell>
				{row?.customer}
			</TableCell>

			<TableCell>
				{row?.batteryPN}
			</TableCell>

			<TableCell>
				{row?.batterySN}
			</TableCell>

			<TableCell>
				{`${row?.chemistry} ${row?.manufacturer} ${row?.itemPN}`}
			</TableCell>

			<TableCell>
				{row?.noCellSerial}
			</TableCell>

			<TableCell>
				{row?.noCellParallel}
			</TableCell>
		</TableRow>
	);
}
