import { useEffect, useState, } from 'react';

import Button from '@mui/material/Button';
import Card from '@mui/material/Card';
import CardActions from '@mui/material/CardActions';
import Container from '@mui/material/Container';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableContainer from '@mui/material/TableContainer';
import TablePagination from '@mui/material/TablePagination';
import {setCurrentUba,setSelectedDevices} from '/src/actions/UbaDevices';
import { getCells, } from 'src/action-creators/Settings';
import { getTestRoutines, } from 'src/action-creators/TestRoutines';
import { setState, } from 'src/actions/UbaDevices';
import CustomTableHead from 'src/components/CustomTableHead';
import { pageStateList, UBA_CHANNEL_LIST, getSelectedCardsOrTableView,  } from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useAuthDispatch, } from 'src/store/AuthProvider';
import { useSettingsDispatch, } from 'src/store/SettingsProvider';
import { useTestRoutines, useTestRoutinesDispatch, } from 'src/store/TestRoutinesProvider';
import { useUbaDevices, useUbaDevicesDispatch, } from 'src/store/UbaDevicesProvider';
import filtering, { handleChangePage, handleChangeRowsPerPage, } from 'src/utils/filtering';
import { fillTestRoutine, isOtherChannelFree, resetTestParameters } from 'src/utils/helper';
import { validateObject, } from 'src/utils/validators';

import CustomTableRow from './CustomTableRow';
import CustomTableToolbar from './CustomTableToolbar';
import { doFiltering, prepareValue, } from './filtering';
import headLabels from './headLabels';

const filtersInitial = {
	testName: '',
	customer: '',
	batteryPN: '',
	cellPN: '',
	noCellSerial: '',
	isLocked: '',
};

export default function WizardZero(props) {

	const [order, setOrder] = useState('asc');
	const [orderBy, setOrderBy] = useState('testName');
	const [page, setPage] = useState(0);
	const [rowsPerPage, setRowsPerPage] = useState(50);
	const [filters, setFilters] = useState(filtersInitial);
	const [selectedTest, setSelectedTest] = useState(null);

	const {currentUba, ubaDevices} = useUbaDevices();
	const {testRoutines,} = useTestRoutines();

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();
	const settingsDispatch = useSettingsDispatch();

	let dataFiltered = filtering(testRoutines, order, orderBy, filters, prepareValue, doFiltering);
	
	//console.log('aaaa',dataFiltered);
	//console.log('bbbb',currentUba);
	//console.log('cccc',ubaDevices);
	const isAnotherChannelFree = isOtherChannelFree(ubaDevices, currentUba);
	//console.log('dddd',sameUbaButWithDifferentChannel, isAnotherChannelFree);
	
	dataFiltered = dataFiltered.filter(test => {
		if (test.channel === UBA_CHANNEL_LIST.A_OR_B) {
			// 'A-or-B' test.
			return true;
		}

		if (!validateObject(currentUba, true)) {
			// We are on the "Run Batch Test" flow.
			return false;//dont allow a & b in batch
		}

		// 'A-and-B' test. Filter out unsuitable tests.
		return isAnotherChannelFree;
	});

	useEffect(() => {
		getTestRoutines(authDispatch, testRoutinesDispatch);
		getCells(authDispatch, settingsDispatch);
	}, []);

	const handleButtonClick = () => {
		fillTestRoutine(selectedTest, false, currentUba?.channel, testRoutinesDispatch);

		if (validateObject(currentUba, true)) {
			ubaDevicesDispatch(setState(pageStateList.WIZARD_ONE));
			return;
		}

		ubaDevicesDispatch(setState(pageStateList.RUN_BATCH_TEST));
	}

	const handleCancelClick = () => {
		resetTestParameters(testRoutinesDispatch);
		//ubaDevicesDispatch(setCurrentUba({}));//not sure if this is needed
    	//ubaDevicesDispatch(setSelectedDevices([]));//not sure if this is needed
		ubaDevicesDispatch(setState(getSelectedCardsOrTableView()));
	}

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
										<CustomTableRow id={row?.id} key={key} {...{row}} {...{selectedTest}} {...{setSelectedTest}} />
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

			<Card sx={{mt: 4,}}>
				<CardActions sx={{justifyContent: 'space-between', mx: 4, my: 2,}}>
					<Button variant="contained" color="primary" sx={{width: 140}} onClick={handleCancelClick}>
						{getText('common.CANCEL')}
					</Button>

					<Button variant="contained" disabled={!validateObject(selectedTest)} onClick={handleButtonClick}>
						{getText('mainPage.wizardOne.SELECT')}
					</Button>
				</CardActions>
			</Card>
		</Container>
	);
}
