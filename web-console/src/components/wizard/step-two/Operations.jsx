import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import IconButton from '@mui/material/IconButton';
import Tooltip from '@mui/material/Tooltip';
import BatteryCharging80Icon from '@mui/icons-material/BatteryCharging80';
import FlashOffIcon from '@mui/icons-material/FlashOff';
import AvTimerIcon from '@mui/icons-material/AvTimer';
import LoopIcon from '@mui/icons-material/Loop';

import {getText,} from 'src/services/string-definitions';
import {useUbaDevices,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {addTestType,} from 'src/actions/TestRoutines';
import {testTypeNames,} from 'src/constants/unsystematic';
import {validateObject,} from 'src/utils/validators';

export default function Operations() {

	const {currentUba,} = useUbaDevices();
	const {testData,} = useTestRoutines();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const addTestHandler = testType => testRoutinesDispatch(addTestType(testType));

	return (
		<Stack component={Card} justifyContent="space-around" sx={{height: 350, p: 0,}}>
			<Tooltip title={getText('testEditor.CHARGE')} placement="left">
				<span>
				<IconButton color="secondary" onClick={() => addTestHandler(testTypeNames.CHARGE)} disabled={!!testData?.isLocked && validateObject(currentUba, true)}>
					<BatteryCharging80Icon fontSize="large"/>
				</IconButton>
				</span>
			</Tooltip>

			<Tooltip title={getText('testEditor.DISCHARGE')} placement="left">
				<span>
				<IconButton color="secondary" onClick={() => addTestHandler(testTypeNames.DISCHARGE)} disabled={!!testData?.isLocked && validateObject(currentUba, true)}>
					<FlashOffIcon fontSize="large"/>
				</IconButton>
				</span>
			</Tooltip>

			<Tooltip title={getText('testEditor.DELAY')} placement="left">
				<span>
				<IconButton color="secondary" onClick={() => addTestHandler(testTypeNames.DELAY)} disabled={!!testData?.isLocked && validateObject(currentUba, true)}>
					<AvTimerIcon fontSize="large"/>
				</IconButton>
				</span>
			</Tooltip>

			<Tooltip title={getText('testEditor.LOOP')} placement="left">
				<span>
				<IconButton color="secondary" onClick={() => addTestHandler(testTypeNames.LOOP)} disabled={!!testData?.isLocked && validateObject(currentUba, true)}>
					<LoopIcon fontSize="large"/>
				</IconButton>
				</span>
			</Tooltip>
		</Stack>
	);
}
