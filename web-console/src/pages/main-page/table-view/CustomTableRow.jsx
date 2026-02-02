import Checkbox from '@mui/material/Checkbox';
import TableCell from '@mui/material/TableCell';
import TableRow from '@mui/material/TableRow';
import { statusCodes, isTestRunning, getErrorMessage} from 'src/constants/unsystematic';
import { useAuthDispatch, } from 'src/store/AuthProvider';
import { useTestRoutinesDispatch, } from 'src/store/TestRoutinesProvider';
import { useUbaDevicesDispatch, } from 'src/store/UbaDevicesProvider';
import {getActions} from '../Actions';
import ErrorOutlineIcon from '@mui/icons-material/ErrorOutline';
import { getChargeCurrent, getTemperature, getTestStep, getTestType, getVoltage, } from '../utils';
import Tooltip from '@mui/material/Tooltip';
import {getText, getDate} from 'src/services/string-definitions';
import Typography from '@mui/material/Typography';

export default function CustomTableRow(props) {

	const {row, selected, handleClick,hoveredRow, setHoveredRow} = props;
	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	return (
		<TableRow tabIndex={-1} selected={selected} onMouseEnter={() => setHoveredRow(row)}
			onMouseLeave={() => setHoveredRow(null)} 
			sx={{
				backgroundColor:
					(hoveredRow && hoveredRow.ubaSN === row.ubaSN && hoveredRow.parallelRun && row.parallelRun) ? 'lightblue' : hoveredRow && hoveredRow?.runningTestID === row?.runningTestID ? 'lightblue' : 'inherit',//this isnt needed anymore, because in parallel we hide the channel B row
				borderTop: row.isFirstAfterErrors===true ? '1px double #d64161' : 'none',
				}}
		>
			<TableCell padding="checkbox">
				<Checkbox
					disableRipple
					checked={selected}
					onChange={handleClick}
					disabled={isTestRunning(row?.status)}
				/>
			</TableCell>

			<TableCell>
				{row?.name}
			</TableCell>

			<TableCell>
				{row?.parallelRun ? getText('common.AB') : row?.channel}
			</TableCell>

			<TableCell>
				{row?.machineName}
			</TableCell>

			<TableCell>
				{row?.batteryPN}
			</TableCell>

			<TableCell>
				{row?.batterySN}
			</TableCell>

			<TableCell>
				{row?.testName}
			</TableCell>

			<TableCell sx={{ textAlign: 'left', whiteSpace: 'nowrap' }}>
				<span style={{ display: 'inline-flex', alignItems: 'center' }}>
					{`${getTestType(row)} (${getTestStep(row)})`}
					{(row?.error > 0) ? (
						<Tooltip title={getErrorMessage(row.error)}>
							<ErrorOutlineIcon color="error" style={{ marginLeft: 4 }} />
						</Tooltip>
						) : null}
				</span>
			</TableCell>

			<TableCell>
				{isTestRunning(row?.status) ? row?.runtime : ''}
			</TableCell>

			<TableCell sx={{ color: !row?.ubaDeviceConnectedTimeAgoMs || row?.ubaDeviceConnectedTimeAgoMs > 120000 ? 'red' : (row?.ubaDeviceConnectedTimeAgoMs > 60000 ? 'orange' : 'green'), }}>
				{row?.lastInstantResultsTimestamp ? (
					<>
						<Typography sx={{fontSize: '0.6rem'}}>
							{`${getVoltage(row?.voltage)} ${getChargeCurrent(row?.current)} ${getTemperature(row?.temp)}`}
						</Typography>
						<Typography sx={{fontSize: '0.6rem'}}>
							{getDate(row?.lastInstantResultsTimestamp)}
						</Typography>
					</>
				) : getText('common.NOT_APPLICABLE')}
			</TableCell>
			
			<TableCell>
				{getActions(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)}
			</TableCell>

		</TableRow>
	);
}
