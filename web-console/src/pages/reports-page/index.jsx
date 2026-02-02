import { useEffect, useState } from 'react';

import Button from "@mui/material/Button";
import Card from '@mui/material/Card';
import Container from '@mui/material/Container';
import Stack from '@mui/material/Stack';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';

import { getReportData, getReports, } from 'src/action-creators/Reports';
import { setModal, } from 'src/actions/Auth';
import CustomTableHead from 'src/components/CustomTableHead';
import { getText, } from 'src/services/string-definitions';
import { useAuthDispatch, } from 'src/store/AuthProvider';
import { useReports, useReportsDispatch, } from 'src/store/ReportsProvider';
import { useSettingsDispatch, } from 'src/store/SettingsProvider';
import { handleChangePage, handleChangeRowsPerPage, } from 'src/utils/filtering';
import { validateArray, } from 'src/utils/validators';
import CustomTableRow from './CustomTableRow';
import CustomTableToolbar from './CustomTableToolbar';
//const ResultsGraphs = lazy(() => import("./modal-views/ResultsGraphs"));
import headLabels from './headLabels';
import EditReport from './modal-views/EditReport';
import ResultsGraphs from './modal-views/ResultsGraphs';

export default function ReportsPage() {

	const filtersInitial = {
		batteryPN: '',
		batterySN: '',
		testName: '',
		ubaSN: '',
		machineName: '',
		dateRange: '',
	};

	const [order, setOrder] = useState('asc');
	const [orderBy, setOrderBy] = useState('testName');
	const [page, setPage] = useState(0);
	const [rowsPerPage, setRowsPerPage] = useState(50);
	const [filters, setFilters] = useState(filtersInitial);
	const [selected, setSelected] = useState([]);

	const {reports,} = useReports();

	const authDispatch = useAuthDispatch();
	const reportsDispatch = useReportsDispatch();
	const settingsDispatch = useSettingsDispatch();

	useEffect(() => {
		getReports(authDispatch, reportsDispatch, settingsDispatch, {
			page,
			rowsPerPage,
			order,
			orderBy,
			filters,
		});
	}, [
		page,
		rowsPerPage,
		order,
		orderBy,
		filters,]);

	const handleCheckClick = (event, id) => {
		const selectedIndex = selected.indexOf(id);
		let newSelected = [];

		if (selectedIndex === -1) {
			newSelected = newSelected.concat(selected, id);
		} else if (selectedIndex === 0) {
			newSelected = newSelected.concat(selected.slice(1));
		} else if (selectedIndex === selected.length - 1) {
			newSelected = newSelected.concat(selected.slice(0, -1));
		} else if (selectedIndex > 0) {
			newSelected = newSelected.concat(
				selected.slice(0, selectedIndex),
				selected.slice(selectedIndex + 1)
			);
		}

		setSelected(newSelected);
	};

	const handleGraphOpening = () => {
		getReportData(authDispatch, reportsDispatch, selected);
		authDispatch(setModal('results.graphs'))
	};

	return (
		<Container maxWidth="false">
			<Card sx={{mb: 1}}>
				<CustomTableToolbar {...{filters}} setFilters={setFilters} setPage={setPage}/>
			</Card>

			<Card>
				<TableContainer>
					<Table>
						<CustomTableHead {...{order}} setOrder={setOrder} {...{orderBy}} setOrderBy={setOrderBy} {...{headLabels}}/>

						<TableBody>
							{
								reports.rows.map((row, key) => (
									<CustomTableRow
										key={key}
										{...{row}}
										selected={selected.indexOf(row.id) !== -1}
										handleClick={event => handleCheckClick(event, row.id)}
									/>
								))
							}
						</TableBody>
					</Table>
				</TableContainer>

				<Stack direction="row" alignItems="center" justifyContent="space-between" sx={{pl: 4, pt: 1, pb: 1}}>
					<Button size="medium" variant="contained" disabled={!validateArray(selected)} onClick={handleGraphOpening}>
						{getText('reportsPage.SHOW_RESULT_GRAPHS')}
					</Button>

					{reports && reports.length > 50 && (
						<TablePagination
							component="div"
							page={page}
							count={reports?.count}
							rowsPerPage={rowsPerPage}
							rowsPerPageOptions={[50, 100]}
							onPageChange={(event, page) => handleChangePage(page, setPage)}
							onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
						/>
					)}
				</Stack>
			</Card>

			<ResultsGraphs maxHeight={95} maxWidth={95} minWidth={95} minHeight={95} actionName={['results.graphs',]} />
			<EditReport p={0} maxHeight={100} actionName={['edit.report',]} metadata={{
				page,
				rowsPerPage,
				order,
				orderBy,
				filters,
			}}/>
		</Container>
	);
}
