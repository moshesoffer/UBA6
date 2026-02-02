import {useState, useEffect,} from 'react';

import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';

import CustomTableHead from 'src/components/CustomTableHead';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevices, useUbaDevicesDispatch} from 'src/store/UbaDevicesProvider';
import {useSettingsDispatch,} from 'src/store/SettingsProvider';
import {getUbaDevices,} from 'src/action-creators/UbaDevices';
import {getMachines,} from 'src/action-creators/Settings';
import {handleChangePage, handleChangeRowsPerPage,} from 'src/utils/filtering';
import {addEditSettings,} from 'src/constants/unsystematic';

import CustomTableRow from './CustomTableRow';
import headLabels from './headLabels';
import AddEditUbaDevice from '../modal-views/AddEditUbaDevice';

export default function UbaDevices() {

	const [page, setPage] = useState(0);
	const [rowsPerPage, setRowsPerPage] = useState(50);

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const settingsDispatch = useSettingsDispatch();

	const {ubaDevices,} = useUbaDevices();
	let ubaDevicesPrepared = Object.groupBy(ubaDevices, uba => uba.ubaSN);
	ubaDevicesPrepared = Object.values(ubaDevicesPrepared);
	ubaDevicesPrepared = ubaDevicesPrepared.map(uba => uba[0]);

	useEffect(() => {
		getUbaDevices(authDispatch, ubaDevicesDispatch);
		getMachines(authDispatch, settingsDispatch);
	}, []);

	return (
		<>
			<TableContainer>
				<Table>
					<CustomTableHead headLabels={headLabels}/>

					<TableBody>
						{
							ubaDevicesPrepared
								.slice(
									page * rowsPerPage,
									page * rowsPerPage + rowsPerPage
								)
								.map((row, key) => (
									<CustomTableRow id={row?.id} key={key} {...{row}} />
								))
						}
					</TableBody>
				</Table>
			</TableContainer>
			{ubaDevicesPrepared && ubaDevicesPrepared.length > 50 && (
				<TablePagination
					page={page}
					component="div"
					count={ubaDevicesPrepared.length}
					rowsPerPage={rowsPerPage}
					rowsPerPageOptions={[50, 100]}
					onPageChange={(event, page) => handleChangePage(page, setPage)}
					onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
				/>
			)}

			<AddEditUbaDevice p={0} actionName={[addEditSettings.ADD_UBA_DEVICE, addEditSettings.EDIT_UBA_DEVICE,]}/>
		</>
	);
}
