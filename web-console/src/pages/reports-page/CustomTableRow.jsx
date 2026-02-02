import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';
import IconButton from '@mui/material/IconButton';
import Checkbox from '@mui/material/Checkbox';
import CloudDownloadIcon from '@mui/icons-material/CloudDownload';
import BorderColorIcon from '@mui/icons-material/BorderColor';

import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useReportsDispatch,} from 'src/store/ReportsProvider';
import {setModal,} from 'src/actions/Auth';
import {setCurrentReport,} from 'src/actions/Reports';
import {getDate,} from 'src/services/string-definitions';
import {statusCodes, getKeyByValue} from 'src/constants/unsystematic';

import DownloadReport from './modal-views/DownloadReport';

export default function CustomTableRow(props) {

	const {row, selected, handleClick,} = props;

	const authDispatch = useAuthDispatch();
	const reportsDispatch = useReportsDispatch();

	const handleEditClick = () => {
		reportsDispatch(setCurrentReport(row));
		authDispatch(setModal('edit.report'));
	};

	const handleDownloadClick = () => {
		reportsDispatch(setCurrentReport(row));
		authDispatch(setModal('download.report'));
	};

	return (
		<TableRow hover tabIndex={-1} selected={selected}>
			<TableCell padding="checkbox">
				<Checkbox disableRipple checked={selected} onChange={handleClick}/>
			</TableCell>

			<TableCell>
				{row?.batteryPN}
			</TableCell>

			<TableCell>
				{getDate(row?.timestampStart)}
			</TableCell>

			<TableCell>
				{row?.batterySN}
			</TableCell>

			<TableCell>
				{row?.testName}
			</TableCell>

			<TableCell>
				{row?.machineName}
			</TableCell>

			<TableCell>
				{row?.ubaSN}
			</TableCell>

			<TableCell>
				{row?.channel}
			</TableCell>

			<TableCell>
				{getKeyByValue(statusCodes, row?.status)}
			</TableCell>

			<TableCell>
				{row?.timeOfTest}
			</TableCell>

			<TableCell>
				<IconButton size="small" color="primary" onClick={handleEditClick}>
					<BorderColorIcon/>
				</IconButton>
				{
					row?.status === statusCodes.FINISHED &&
						<IconButton size="small" color="primary" onClick={handleDownloadClick}>
							<CloudDownloadIcon/>
						</IconButton>
				}
			</TableCell>

			<DownloadReport p={0} actionName={['download.report',]}/>
		</TableRow>
	);
}
