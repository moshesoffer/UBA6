import {useState, useEffect,} from 'react';

import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';

import CustomTableHead from 'src/components/CustomTableHead';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useSettings, useSettingsDispatch,} from 'src/store/SettingsProvider';
import {getCells,} from 'src/action-creators/Settings';
import {handleChangePage, handleChangeRowsPerPage,} from 'src/utils/filtering';
import {addEditSettings,} from 'src/constants/unsystematic';

import CustomTableRow from './CustomTableRow';
import headLabels from './headLabels';
import AddEditCell from '../modal-views/AddEditCell';

export default function SettingsPage() {

	const [page, setPage] = useState(0);
	const [order, setOrder] = useState('asc');
	const [orderBy, setOrderBy] = useState('chemistry');
	const [rowsPerPage, setRowsPerPage] = useState(50);

	const {cells,} = useSettings();
	const authDispatch = useAuthDispatch();
	const settingsDispatch = useSettingsDispatch();

	useEffect(() => {
		getCells(authDispatch, settingsDispatch);
	}, []);

	return (
		<>
			<TableContainer>
				<Table>
					<CustomTableHead {...{order}} setOrder={setOrder} {...{orderBy}} setOrderBy={setOrderBy} headLabels={headLabels}/>

					<TableBody>
						{
							cells
								.sort((a, b) => order==='asc' ? -b[orderBy].localeCompare(a[orderBy]) : b[orderBy].localeCompare(a[orderBy]))
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

			{cells && cells.length > 50 && (
				<TablePagination
					page={page}
					component="div"
					count={cells.length}
					rowsPerPage={rowsPerPage}
					rowsPerPageOptions={[50, 100]}
					onPageChange={(event, page) => handleChangePage(page, setPage)}
					onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
				/>
			)}

			<AddEditCell p={0} maxHeight={100} actionName={[addEditSettings.ADD_CELL, addEditSettings.EDIT_CELL,]}/>
		</>
	);
}
