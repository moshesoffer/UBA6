import Grid from '@mui/material/Grid';
import Card from '@mui/material/Card';
import CardHeader from '@mui/material/CardHeader';
import CardContent from '@mui/material/CardContent';
import Stack from '@mui/material/Stack';
import Box from '@mui/material/Box';
import Divider from '@mui/material/Divider';
import Chip from '@mui/material/Chip';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import {ubaChannel, statusCodes, getKeyByValue, getErrorMessage} from 'src/constants/unsystematic';
import {getVoltage, getChargeCurrent, getTemperature, } from '../utils';
import {getActions} from '../Actions';
import { useTestRoutinesDispatch, } from 'src/store/TestRoutinesProvider';
import {getText, getDate} from 'src/services/string-definitions';
import Tooltip from '@mui/material/Tooltip';
import ErrorOutlineIcon from '@mui/icons-material/ErrorOutline';

export default function UbaCard({row}) {

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const getStep = channel => {
		if (row?.[channel]?.status===statusCodes.RUNNING) {
			return `${row?.[channel]?.testCurrentStep}/${row?.[channel]?.totalStagesAmount}`;
		} else {
			return '';
		}
	}

	const printCard = (channelIndex, size) => {
		let sx = {borderRight: theme => `solid 1px ${theme.palette.divider}`,};
		if (channelIndex) {
			sx = {};
		}
		return (
			<Grid item lg={size} sx={sx} title={row?.[channelIndex]?.lastInstantResultsTimestamp ? getDate(row?.[channelIndex]?.lastInstantResultsTimestamp) : undefined}>
				 <Stack direction="row" spacing={2} justifyContent="space-between">
					<Box>
						Ch-{row?.[channelIndex]?.parallelRun ? getText('common.AB') : row?.[channelIndex]?.channel}
					</Box>
					<Box sx={{ color: !row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs || row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs > 120000 ? 'red' : (row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs > 60000 ? 'orange' : 'green'), }}>
						{getVoltage(row?.[channelIndex]?.voltage)}
					</Box>
				 </Stack>
				
				 <Stack direction="row" spacing={2} justifyContent="space-between" sx={{ color: !row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs || row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs > 120000 ? 'red' : (row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs > 60000 ? 'orange' : 'green'), }}>
				 	<Box>
						{getChargeCurrent(row?.[channelIndex]?.current)}
					</Box>

					<Box>
						{getTemperature(row?.[channelIndex]?.temp)}
					</Box>
				</Stack>

				<Stack direction="row" justifyContent="space-around">
					<Chip 
						label={`${getKeyByValue(statusCodes, row?.[channelIndex]?.status)} ${getStep(channelIndex)}`}
						sx={{
							backgroundColor: row?.[channelIndex]?.status === statusCodes.RUNNING ? '#FFFF00' :
												row?.[channelIndex]?.status === statusCodes.FINISHED ? '#92D051' :
												row?.[channelIndex]?.status === statusCodes.STOPPED ? '#FFA500' :
												row?.[channelIndex]?.status === statusCodes.PAUSED ? '#D0CECE' :
												row?.[channelIndex]?.status === statusCodes.STANDBY ? '#8FAADC' :
												row?.[channelIndex]?.status === statusCodes.ABORTED ? '#FF0000' :
												'gray', // Default color if no match
							color: row?.[channelIndex]?.status === statusCodes.RUNNING || row?.[channelIndex]?.status === statusCodes.PAUSED || row?.[channelIndex]?.status === statusCodes.FINISHED ? 'black' : 'white', // Ensuring text is visible against background
							border: '1px solid black',
							fontWeight: 'bold',
							}}
					/>
					{row?.[channelIndex].error > 0 ? <Tooltip title={getErrorMessage(row?.[channelIndex].error)}><ErrorOutlineIcon color="error" sx={{ width: 20, height: 20, marginTop:0.2, paddingLeft: 0 }} /></Tooltip> : null}
				</Stack>
				<Stack direction="row" justifyContent="space-around">
					{getActions(row?.[channelIndex], authDispatch, ubaDevicesDispatch, testRoutinesDispatch)}
				</Stack>
			</Grid>
		);
	}

	return (
		<Grid item >
			<Card sx={{border: (row?.[0]?.status === statusCodes.ABORTED && row?.[0]?.error > 0) || (row?.[1]?.status === statusCodes.ABORTED && row?.[1]?.error > 0) ? '2px double #d64161' : '1px solid gray',}}>
				<CardHeader title={`${row?.[0]?.name} / ${row?.[0]?.machineName}`} titleTypographyProps={{textAlign: 'center',}} />

				<Divider inset="none"/>

				<CardContent>
					<Grid container spacing={0}>
						{row?.[0]?.parallelRun ? (
							printCard(0, 12)
						 ) : <>
						 	{row?.[0]?.channel === ubaChannel.A ? (
								printCard(0, 6) 
							) : (
								<Grid item lg={6} sx={{borderRight: theme => `solid 1px ${theme.palette.divider}`,}}/>
							)}

							{row?.[1]?.channel === ubaChannel.B ? (
								printCard(1, 6) 
							) : (
									<Grid item lg={6}/>
								)
							}
						 </>
					}		
					</Grid>
				</CardContent>
			</Card>
		</Grid>
	);
}
