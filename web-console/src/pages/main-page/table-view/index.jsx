import { useEffect, useRef, useState } from 'react';

import Button from '@mui/material/Button';
import Card from '@mui/material/Card';
import Container from '@mui/material/Container';
import Stack from '@mui/material/Stack';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';
import { setSelectedDevices, setState, setCurrentUba } from 'src/actions/UbaDevices';
import CustomTableHead from 'src/components/CustomTableHead';
import { pageStateList, statusCodes, ubaChannel, } from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useUbaDevices, useUbaDevicesDispatch, } from 'src/store/UbaDevicesProvider';
import filtering, { handleChangePage, handleChangeRowsPerPage, } from 'src/utils/filtering';
import { validateArray, } from 'src/utils/validators';
import CustomTableToolbar from '../CustomTableToolbar';
import { doFiltering, prepareValue, } from '../filtering';
import { enrichUbaDevicesWithRunTime, } from '../utils';
import CustomTableRow from './CustomTableRow';
import CustomToolbar from '../CustomToolbar';
import headLabels from './headLabels';
import { getUbaDevices} from 'src/action-creators/UbaDevices';
import {useAuthDispatch,} from 'src/store/AuthProvider';

const POLLING_INTERVAL = 5000;

export default function TableView() {

	const filtersInitial = {
		ubaSN: '',
		batteryPN: '',
		status: '',
		machineName: '',
	};

	const pollingRef = useRef(null);

	const [order, setOrder] = useState('asc');
	const [orderBy, setOrderBy] = useState('runtime');
	const [page, setPage] = useState(0);
	const [rowsPerPage, setRowsPerPage] = useState(50);
	const [filters, setFilters] = useState(filtersInitial);
	const [hoveredRow, setHoveredRow] = useState(null);

	const {ubaDevices, selectedDevices,} = useUbaDevices();
	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	const ubaEnriched = enrichUbaDevicesWithRunTime(ubaDevices);
	const dataFiltered = filtering(ubaEnriched, order, orderBy, filters, prepareValue, doFiltering);

	const rowsWithErrors = dataFiltered.filter((row) => row.error > 0 && row.status === statusCodes.ABORTED);

	const rowsWithOutErrors = dataFiltered.filter((row) => row.status !== statusCodes.ABORTED);

	if(rowsWithErrors && rowsWithErrors.length > 0 && rowsWithOutErrors && rowsWithOutErrors.length > 0){
		rowsWithOutErrors[0].isFirstAfterErrors = true;
	}

	useEffect(() => {
		ubaDevicesDispatch(setSelectedDevices([]));
		ubaDevicesDispatch(setCurrentUba({}));
		//console.log('TableView useEffect');
		clearInterval(pollingRef.current);
		pollingRef.current = setInterval(() => getUbaDevices(authDispatch, ubaDevicesDispatch, true), POLLING_INTERVAL);
		return () => {
			//console.log('TableView unmount useEffect');
			clearInterval(pollingRef.current);
		}
	}, []);

	const prepareSelected = () => selectedDevices.map(element => getCompositeKey(element.ubaSN, element.channel));

	const getCompositeKey = (ubaSN, channel) => `${ubaSN}:${channel.toUpperCase()}`;

	const handleCheckClick = row => {
		const newItem = {
			ubaSN: row.ubaSN,
			name: row.name,
			ubaChannel: row.ubaChannel,
			channel: row.channel,
		};

		const selectedIndex = prepareSelected().indexOf(getCompositeKey(row.ubaSN, row.channel));
		let newSelected = [];
		const selected = [...selectedDevices];

		if (selectedIndex === -1) {
			newSelected = newSelected.concat(selected, newItem);
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

		ubaDevicesDispatch(setSelectedDevices(newSelected));
	};

	return (
			<Container maxWidth="false">
				<Card sx={{mb: 1}}>
					<CustomToolbar/>
				</Card>

				<Card sx={{mb: 1}}>
					<CustomTableToolbar {...{filters}} setFilters={setFilters} setPage={setPage}/>
				</Card>

				<Card>
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
											(!row.parallelRun || (row.parallelRun && row.channel === ubaChannel.A)) &&
												<CustomTableRow
													key={key}
													{...{row}}
													selected={prepareSelected().indexOf(getCompositeKey(row.ubaSN, row.channel)) !== -1}
													handleClick={() => handleCheckClick(row)}
													setHoveredRow={(rowHovered) => setHoveredRow(rowHovered)}
													hoveredRow={hoveredRow}
												/>
										))
								}
							</TableBody>
						</Table>
					</TableContainer>

					<Stack direction="row" alignItems="center" justifyContent="space-between" sx={{pl: 4, pt: 1, pb: 1}}>
						<Button size="medium" variant="contained" disabled={!validateArray(selectedDevices) || selectedDevices.length < 2} onClick={() => {ubaDevicesDispatch(setCurrentUba({})); ubaDevicesDispatch(setState(pageStateList.WIZARD_ZERO));}}>
							{getText('mainPage.runBatchTest.RUN_BATCH_TEST')}
						</Button>

						{dataFiltered && dataFiltered.length > 50 && (
							<TablePagination
								component="div"
								page={page}
								count={dataFiltered.length}
								rowsPerPage={rowsPerPage}
								rowsPerPageOptions={[50, 100]}
								onPageChange={(event, page) => handleChangePage(page, setPage)}
								onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
							/>
						)}
					</Stack>
				</Card>
			</Container>
	);
}
