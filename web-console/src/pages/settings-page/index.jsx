import {useState,} from 'react';

import Container from '@mui/material/Container';
import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import Tab from '@mui/material/Tab';
import Tabs from '@mui/material/Tabs';
import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import {useSettingsDispatch,} from 'src/store/SettingsProvider';
import {getText,} from 'src/services/string-definitions';
import {setModal} from 'src/actions/Auth';
import {setCurrentUba,} from 'src/actions/UbaDevices';
import {setCurrentCell,} from 'src/actions/Settings';
import {addEditSettings,} from 'src/constants/unsystematic';
import UbaDevices from './uba-devices';
import Machines from './machines';
import Cells from './cells';

const UBA_DEVICES = 'ubaDevices';
const MACHINES = 'machines';
const CELLS = 'cells';

export default function SettingsPage() {

	const [currentTab, setCurrentTab] = useState(UBA_DEVICES);
	const handleChangeTab = (event, newValue) => setCurrentTab(newValue);

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const settingsDispatch = useSettingsDispatch();

	let actionName = null;
	let actionValue = null;
	switch (currentTab) {
		case UBA_DEVICES: {
			actionName = 'settingsPage.uba.ADD_UBA';
			actionValue = addEditSettings.ADD_UBA_DEVICE;
			break;
		}
		case MACHINES: {
			actionName = null;
			actionValue = null;
			break;
		}
		case CELLS: {
			actionName = 'settingsPage.cell.ADD_CELL';
			actionValue = addEditSettings.ADD_CELL;
			break;
		}
		default: {
			console.error('Invalid tab value');
		}
	}

	const handleButtonClick = () => {
		ubaDevicesDispatch(setCurrentUba({}));
		settingsDispatch(setCurrentCell({}));
		authDispatch(setModal(actionValue));
	}

	return (
		<Container maxWidth="false">
			<Card>
				<Stack direction="row" justifyContent="space-between" >
					<Tabs value={currentTab} onChange={handleChangeTab} >
						<Tab value={UBA_DEVICES} label={getText('mainPage.UBA_DEVICES')}/>
						<Tab value={MACHINES} label={getText('settingsPage.LABS')}/>
						<Tab value={CELLS} label={getText('testEditor.CELL_P_N')}/>
					</Tabs>

					{actionValue &&
						<Button variant="contained" onClick={handleButtonClick} sx={{  mr: 4,}}>
							<Typography>{getText(actionName)}</Typography>
						</Button>
					}
				</Stack>

				{currentTab === UBA_DEVICES && <UbaDevices />}
				{currentTab === MACHINES && <Machines/>}
				{currentTab === CELLS && <Cells/>}
			</Card>
		</Container>
	);
}
