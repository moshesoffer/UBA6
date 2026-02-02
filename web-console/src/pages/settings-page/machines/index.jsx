import {useState, useEffect,} from 'react';

import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';

import CustomTableHead from 'src/components/CustomTableHead';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useSettings, useSettingsDispatch,} from 'src/store/SettingsProvider';
import {getMachines,} from 'src/action-creators/Settings';
import {handleChangePage, handleChangeRowsPerPage,} from 'src/utils/filtering';

import CustomTableRow from './CustomTableRow';
import headLabels from './headLabels';

export default function Machines() {

	const [page, setPage] = useState(0);
	const [rowsPerPage, setRowsPerPage] = useState(50);

	const {machines,} = useSettings();
	const authDispatch = useAuthDispatch();
	const settingsDispatch = useSettingsDispatch();

	useEffect(() => {
		getMachines(authDispatch, settingsDispatch);
	}, []);

	return (
		<>
			<TableContainer>
				<Table>
					<CustomTableHead headLabels={headLabels}/>

					<TableBody>
						{
							machines
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

			{machines && machines.length > 50 && (
				<TablePagination
					page={page}
					component="div"
					count={machines.length}
					rowsPerPage={rowsPerPage}
					rowsPerPageOptions={[50, 100]}
					onPageChange={(event, page) => handleChangePage(page, setPage)}
					onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
				/>
			)}
		</>
	);
}
