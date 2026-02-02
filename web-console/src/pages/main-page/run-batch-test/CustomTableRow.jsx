import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';

export default function CustomTableRow(props) {

	const {row,} = props;

	return (
		<TableRow hover tabIndex={-1}>
			<TableCell>
				{row?.name}
			</TableCell>

			<TableCell>
				{row?.channel}
			</TableCell>

			<TableCell>
				{row?.batterySerialNumber}
			</TableCell>
		</TableRow>
	);
}
