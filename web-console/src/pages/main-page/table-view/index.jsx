import { useEffect, useRef, useState } from 'react';

import Button from '@mui/material/Button';
import Card from '@mui/material/Card';
import Container from '@mui/material/Container';
import Stack from '@mui/material/Stack';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';
import { getUbaDevices } from 'src/action-creators/UbaDevices';
import { setSecondaryNotification } from 'src/actions/Auth';
import { setCurrentUba, setSelectedDevices, setState } from 'src/actions/UbaDevices';
import CustomTableHead from 'src/components/CustomTableHead';
import { pageStateList, statusCodes, ubaChannel, } from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useAuth, useAuthDispatch } from 'src/store/AuthProvider';
import { useUbaDevices, useUbaDevicesDispatch, } from 'src/store/UbaDevicesProvider';
import filtering, { handleChangePage, handleChangeRowsPerPage, } from 'src/utils/filtering';
import { validateArray, } from 'src/utils/validators';
import CustomTableToolbar from '../CustomTableToolbar';
import CustomToolbar from '../CustomToolbar';
import { doFiltering, prepareValue, } from '../filtering';
import { enrichUbaDevicesWithRunTime, } from '../utils';
import CustomTableRow from './CustomTableRow';
import headLabels from './headLabels';

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
	const [dataFiltered, setDataFiltered] = useState([]);
	const [amountOfRowsWithErrors, setAmountOfRowsWithErrors] = useState(-1);

	const {ubaDevices, selectedDevices,} = useUbaDevices();
	const authDispatch = useAuthDispatch();
	const secondaryNotification = useAuth()?.secondaryNotification || {};
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	useEffect(() => {
		ubaDevicesDispatch(setSelectedDevices([]));
		ubaDevicesDispatch(setCurrentUba({}));
		//console.log('TableView useEffect');
		clearInterval(pollingRef.current);
		pollingRef.current = setInterval(() => getUbaDevices(authDispatch, ubaDevicesDispatch, true), POLLING_INTERVAL);
		return () => {
			//console.log('TableView unmount useEffect');
			authDispatch(setSecondaryNotification({message: '',}));
			clearInterval(pollingRef.current);
		}
	}, []);

	useEffect(() => {
		if(!ubaDevices || ubaDevices.length === 0) return;
		const ubaEnriched = enrichUbaDevicesWithRunTime(ubaDevices);
		const dataFilteredTemp = filtering(ubaEnriched, order, orderBy, filters, prepareValue, doFiltering);
		const abortedRowsWithErrors = dataFilteredTemp.filter((row) => row.error > 0 && row.status === statusCodes.ABORTED);
		const rowsWithOutErrors = dataFilteredTemp.filter((row) => !row.error || row.error === 0);

		if(abortedRowsWithErrors && abortedRowsWithErrors.length > 0 && rowsWithOutErrors && rowsWithOutErrors.length > 0){
			rowsWithOutErrors[0].isFirstAfterErrors = true;
		}
		setDataFiltered(dataFilteredTemp);

		const rowsWithErrors = dataFilteredTemp.filter((row) => row.error > 0);
		//amountOfRowsWithErrors > -1 to avoid showing notification on first load
		//rowsWithErrors?.length > 0 to avoid showing notification when there are no errors
		//rowsWithErrors.length > amountOfRowsWithErrors to avoid showing notification on every refresh
		if(!secondaryNotification.message && amountOfRowsWithErrors !== -1 && rowsWithErrors?.length > 0 && rowsWithErrors.length > amountOfRowsWithErrors){
			authDispatch(setSecondaryNotification({message: 'Some devices have errors',}));
		} else if(secondaryNotification.message && rowsWithErrors?.length === 0){
			authDispatch(setSecondaryNotification({message: '',}));
		}

		setAmountOfRowsWithErrors(rowsWithErrors.length);

	}, [ubaDevices]);

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
