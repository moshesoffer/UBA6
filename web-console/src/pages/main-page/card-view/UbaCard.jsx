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
import {ubaChannel, statusCodes, getKeyByValue} from 'src/constants/unsystematic';
import {getVoltage, getChargeCurrent, getTemperature, } from '../utils';
import {getActions} from '../Actions';
import { useTestRoutinesDispatch, } from 'src/store/TestRoutinesProvider';
import {getText} from 'src/services/string-definitions';


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
			<Grid item lg={size} sx={sx} >
				 <Stack direction="row" spacing={2} justifyContent="space-between">
					<Box>
						Ch-{row?.[channelIndex]?.parallelRun ? getText('common.AB') : row?.[channelIndex]?.channel}
					</Box>
					<Box>
						{getVoltage(row?.[channelIndex]?.voltage)}
					</Box>
				 </Stack>
				
				 <Stack direction="row" spacing={2} justifyContent="space-between">
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
				</Stack>
				<Stack direction="row" justifyContent="space-around">
					{getActions(row?.[channelIndex], authDispatch, ubaDevicesDispatch, testRoutinesDispatch, true)}
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
