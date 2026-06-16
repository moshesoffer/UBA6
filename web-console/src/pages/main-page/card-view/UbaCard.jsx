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
import {getVoltage, getChargeCurrent, getTemperature, getCapacity, getTestRoutineName,} from '../utils';
import {getActions} from '../Actions';
import { useTestRoutinesDispatch, } from 'src/store/TestRoutinesProvider';
import {getText, getDate} from 'src/services/string-definitions';
import Tooltip from '@mui/material/Tooltip';
import ErrorOutlineIcon from '@mui/icons-material/ErrorOutline';
import {getTestRuntime, } from '../utils';
import {useTestRoutines,} from 'src/store/TestRoutinesProvider';
import {useEffect, useRef,} from 'react';
import {getUbaDevices,} from 'src/action-creators/UbaDevices';

const POLLING_INTERVAL = 200;

export default function UbaCard({row}) {

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();
	const {testData,} = useTestRoutines();
	const pollingRef = useRef(null);

	useEffect(() => {
	console.log (`==> testData.noCellSerial: ${testData?.noCellSerial?.toString()}`);
	}, [testData?.testName,]);

	useEffect(() => {
		//console.log('MainPage useEffect');
		getUbaDevices(authDispatch, ubaDevicesDispatch);
		//return () => console.log('MainPage unmount useEffect');
	}, []);

	const getStep = channel => {
		if (row?.[channel]?.status===statusCodes.RUNNING) {
			return `${row?.[channel]?.testCurrentStep}/${row?.[channel]?.totalStagesAmount}`;
		} else {
			return '';
		}
	}

	useEffect(() => {
		clearInterval(pollingRef.current);
		pollingRef.current = setInterval(() => getUbaDevices(authDispatch, ubaDevicesDispatch, true), POLLING_INTERVAL);

		return () => {
			clearInterval(pollingRef.current);
		}
	}, []);

	const formatTime = (seconds) => {
	  if (!seconds) return "00:00:00";

	  const h = Math.floor(seconds / 3600);
	  const m = Math.floor((seconds % 3600) / 60);
	  const s = Math.floor(seconds % 60);

	  return [h, m, s]
	    .map(v => String(v).padStart(2, '0'))
	    .join(':');
	};

	const printCard = (channelIndex, size) => {
		let sx = {borderRight: 'solid',
				  borderRightWidth: '1px',
				  borderColor: 'solid',
				 };
		if (channelIndex) {
			sx = {};
		}

		let rowStatus = row?.[channelIndex]?.status & ~statusCodes.PENDING;

		return (
			<Grid item lg={size} sx={sx} title={row?.[channelIndex]?.lastInstantResultsTimestamp ? getDate(row?.[channelIndex]?.lastInstantResultsTimestamp) : undefined}>
				<Stack direction="row" spacing={2} justifyContent="space-between">
					<Box
					  	sx={{
					    	display: 'flex',
					    	flexDirection: 'column',
					    	width: '140px',
					    	height: '180px',
					    	color:
					    	  	!row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs ||
					    	  	row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs > 120000
					    	    	? 'green'
					    	    	: row?.[channelIndex]?.ubaDeviceConnectedTimeAgoMs > 60000
					    	    	? 'orange'
					    	    	: 'green',
					  	}}>

						<Stack direction="row" alignItems="center" justifyContent="space-between">
							<span style={{ fontSize: '18px', fontFamily: 'monospace', whiteSpace: 'pre' }}>
								<b>
									CH-{row?.[channelIndex]?.parallelRun ? getText('common.AB') : row?.[channelIndex]?.channel}
								</b>
							</span>

			  				<Stack 
								direction="row" 
								justifyContent=""
								style={{ position: 'relative', top: '-1px' }}
							>
								<Chip 
								  /*label={`${getKeyByValue(statusCodes, rowStatus)}`}*/
									label =	{(rowStatus & ~statusCodes.PENDING) & (statusCodes.RUNNING | statusCodes.NEXTSTEP | statusCodes.PAUSED) ?
												row?.[channelIndex]?.testState === null ? `${getKeyByValue(statusCodes, statusCodes.RUNNING)}`.toUpperCase() :
													`${row?.[channelIndex]?.testState}`.toUpperCase() :
											 (rowStatus & ~statusCodes.PENDING) & (statusCodes.STOPPED | statusCodes.FINISHED | statusCodes.SAVED) ?
												`${getKeyByValue(statusCodes, statusCodes.STOPPED)}`.toUpperCase() :

												`${getKeyByValue(statusCodes, statusCodes.STANDBY)}`.toUpperCase()
											}							
									sx={{
											backgroundColor: rowStatus & statusCodes.RUNNING ? '#92D051' :
													         rowStatus & statusCodes.NEXTSTEP ? '#92D051' :
														 	 rowStatus === statusCodes.FINISHED ? '#92D051' :
														 	 rowStatus === statusCodes.STOPPED ? '#FFA500' :
														 	 rowStatus === statusCodes.PAUSED ? '#FFFF00' :
														 	 rowStatus === statusCodes.STANDBY ? '#FFFFFF' :
														 	 rowStatus === statusCodes.ABORTED ? '#FF0000' :
														 	 'gray', // Default color if no match
										color: rowStatus === statusCodes.RUNNING || rowStatus === statusCodes.PAUSED || rowStatus === statusCodes.FINISHED ? 'black' : 'black', // Ensuring text is visible against background
										border: '1px solid black',
										fontSize: '2px',
										}}
								/>
								{row?.[channelIndex].error > 0 ? <Tooltip title={getErrorMessage(row?.[channelIndex].error)}><ErrorOutlineIcon color="error" sx={{ width: 20, height: 20, marginTop:0.2, paddingLeft: 0 }} /></Tooltip> : null}
							</Stack>
						</Stack>

						<span style={{ display: 'block', textAlign: 'center', fontSize: '4px' }}>
							.
						</span>

						<span style={{ display: 'block', textAlign: 'center', fontSize: '13px' }}>
							<b>
								{String(getTestRoutineName(row?.[channelIndex]))
									.slice(0, 16)}
							</b>
						</span>
						<span style={{ display: 'block', textAlign: 'center', fontSize: '4px' }}>
							.
						</span>

						<span style={{ textAlign: 'center', fontSize: '12px' }}>
						  	  	<b>{formatTime(getTestRuntime(row?.[channelIndex]))}</b>
						</span>
						<span style={{ display: 'block', textAlign: 'center', fontSize: '4px' }}>
							.
						</span>

					    <span style={{ fontFamily: 'monospace', whiteSpace: 'pre' }}>
							{String('voltage:').padEnd(8, ' ')} <b>{getVoltage(row?.[channelIndex]?.voltage).slice(0, 8)}</b>
						</span>

					    <span style={{ fontFamily: 'monospace', whiteSpace: 'pre' }}>
							{String('current:').padEnd(9, ' ')} 
						  	  	<b>{getChargeCurrent(row?.[channelIndex]?.current).slice(0, 10)}</b>
						</span>

					    <span style={{ fontFamily: 'monospace', whiteSpace: 'pre' }}>
						  	{String('capacity:').padEnd(9, ' ')}
						  		<b>{String(getCapacity(row?.[channelIndex]?.capacity)).slice(0, 10)}</b>
						</span>

					    <span style={{ fontFamily: 'monospace', whiteSpace: 'pre' }}>
							{String('temp:').padEnd(8, ' ')} <b>{String(getTemperature(row?.[channelIndex]?.temp)).slice(0, 8)}</b>
						</span> 

						<span style={{ fontFamily: 'monospace', whiteSpace: 'pre' }}>
						  	{String('step:').padEnd(9, ' ')}

						  	{row?.[channelIndex]?.status !== statusCodes.STANDBY && (
						  	  	<b>{row?.[channelIndex]?.testCurrentStep + 1} of {row?.[channelIndex]?.totalStagesAmount}</b>
						  	)}
						</span>
					</Box>	
				</Stack>

				<Stack direction="row" justifyContent="space-around" >
					{getActions(row?.[channelIndex], authDispatch, ubaDevicesDispatch, testRoutinesDispatch)}
				</Stack>
				<span style={{ display: 'block', textAlign: 'center', fontSize: '4px' }}>
					.
				</span>
			</Grid>
		);
	}

	return (
		<Grid item >
			<Card sx={{border: (row?.[0]?.status === statusCodes.ABORTED && row?.[0]?.error > 0) || (row?.[1]?.status === statusCodes.ABORTED && row?.[1]?.error > 0) ? '3px double #d64161' : '1px solid gray',}}>
				<CardHeader title={`${String(row?.[0]?.machineName).slice(0, 16)}`} titleTypographyProps={{textAlign: 'center',}} />

				<Divider sx={{ width: 300, borderColor: 'black', width: '300px' }} />

				<CardContent>
					<Grid container spacing={0}>
						{row?.[0]?.parallelRun ? (
							printCard(1, 12)
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
