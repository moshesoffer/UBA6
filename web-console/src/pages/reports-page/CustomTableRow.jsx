import TableRow from '@mui/material/TableRow';
import TableCell from '@mui/material/TableCell';
import IconButton from '@mui/material/IconButton';
import DeleteIcon from '@mui/icons-material/Delete';
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
import {deleteReport, downloadReport} from 'src/action-creators/Reports';
import {useSettingsDispatch,} from 'src/store/SettingsProvider';

import { useState } from 'react';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';

export default function CustomTableRow(props) {
	const {metadata,} = props;

	const {row, selected, handleClick,} = props;

	const authDispatch = useAuthDispatch();
	const reportsDispatch = useReportsDispatch();
	const settingsDispatch = useSettingsDispatch();

	const handleEditClick = () => {
		reportsDispatch(setCurrentReport(row));
		authDispatch(setModal('edit.report'));
	};

	//const handleDownloadClick = () => {
	//	reportsDispatch(setCurrentReport(row));
	//	authDispatch(setModal('download.report'));
	//};
	const handleDownloadClick = () => {
	    const report = {
	        ...row,
	        sampleRate
	    };

    	//console.log('row:', row);
    	//console.log('report.id:', row?.id);
    	//console.log('sampleRate:', sampleRate);

	    reportsDispatch(setCurrentReport(report));
	    downloadReport(
	        authDispatch,
	        reportsDispatch,
	        settingsDispatch,
	        metadata,
	        report,
	        sampleRate
	    );
	    authDispatch(setModal('download.report'));
	};

	const metadata1 = {
	    page: 0,
	    rowsPerPage: 50,
	    order: "asc",
	    orderBy: "testName",
	    filters: ["", "", "", "", "", ""]
	};
	const handleDeleteClick = () => {
		let choice = confirm('Confirm Delete Report?');
		if(choice === true) {
			deleteReport(authDispatch, reportsDispatch, settingsDispatch, metadata1, row);
			return true;
		}
		return false;
	}

	const [sampleRate, setSampleRate] = useState(row?.sampleRate ?? 1);

	const handleSampleRateChange = (event) => {
	    setSampleRate(Number(event.target.value));
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
				<IconButton size="small" color="primary" onClick={handleDeleteClick}>
					<DeleteIcon/>
				</IconButton>
				<IconButton size="small" color="primary" onClick={handleEditClick}>
					<BorderColorIcon/>
				</IconButton>

				{
					row?.status === statusCodes.FINISHED &&
			    		<Select
			    		    size="small"
			    		    value={sampleRate}
			    		    onChange={handleSampleRateChange}
			    		    sx={{ ml: 1, minWidth: 30 }}
			    		>
			    		    {[1, 5, 10, 15, 30, 60, 180].map((value) => (
			    		        <MenuItem key={value} value={value}>
			    		            {value}
			    		        </MenuItem>
			    		    ))}
			    		</Select>
				}
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
