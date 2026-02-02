import {useState, useEffect,} from 'react';

import Container from '@mui/material/Container';
import Card from '@mui/material/Card';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';

import CustomTableHead from 'src/components/CustomTableHead';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useTestRoutines, useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {useSettingsDispatch,} from 'src/store/SettingsProvider';
import {getTestRoutines,} from 'src/action-creators/TestRoutines';
import {getCells,} from 'src/action-creators/Settings';
import filtering, {handleChangePage, handleChangeRowsPerPage,} from 'src/utils/filtering';
import CustomTableToolbar from './CustomTableToolbar';
import CustomTableRow from './CustomTableRow';
import headLabels from './headLabels';
import {prepareValue, doFiltering,} from './filtering';

const filtersInitial = {
	testName: '',
	batteryPN: '',
	cellPN: '',
	isLocked: '',
};

export default function TableView() {

	const [page, setPage] = useState(0);
	const [order, setOrder] = useState('asc');
	const [orderBy, setOrderBy] = useState('testName');
	const [rowsPerPage, setRowsPerPage] = useState(50);
	const [filters, setFilters] = useState(filtersInitial);

	const {testRoutines,} = useTestRoutines();

	const authDispatch = useAuthDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();
	const settingsDispatch = useSettingsDispatch();

	const dataFiltered = filtering(testRoutines, order, orderBy, filters, prepareValue, doFiltering);

	useEffect(() => {
		getTestRoutines(authDispatch, testRoutinesDispatch);
		getCells(authDispatch, settingsDispatch);
	}, []);

	return (
		<Container maxWidth="false">
			<Card>
				<CustomTableToolbar {...{filters}} setFilters={setFilters} setPage={setPage}/>

				<TableContainer>
					<Table>
						<CustomTableHead {...{order}} setOrder={setOrder} {...{orderBy}} setOrderBy={setOrderBy} {...{headLabels}}/>

						<TableBody>
							{
								dataFiltered
									.slice(
										page * rowsPerPage,
										page * rowsPerPage + rowsPerPage
									)
									.map((row, key) => (
										<CustomTableRow id={row?.id} key={key} {...{row}}/>
									))
							}
						</TableBody>
					</Table>
				</TableContainer>

				{dataFiltered && dataFiltered.length > 50 && (
					<TablePagination
						page={page}
						component="div"
						count={dataFiltered.length}
						rowsPerPage={rowsPerPage}
						rowsPerPageOptions={[50, 100]}
						onPageChange={(event, page) => handleChangePage(page, setPage)}
						onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
					/>
				)}
			</Card>
		</Container>
	);
}
