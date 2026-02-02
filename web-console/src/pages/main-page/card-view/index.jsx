import {useState, useEffect, useRef,} from 'react';

import Container from '@mui/material/Container';
import Card from '@mui/material/Card';
import Grid from '@mui/material/Grid';
import TablePagination from '@mui/material/TablePagination';
import {useUbaDevices, useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import { ubaChannel,} from 'src/constants/unsystematic';
import filtering, {handleChangePage, handleChangeRowsPerPage,} from 'src/utils/filtering';
import CustomToolbar from '../CustomToolbar';
import CustomTableToolbar from '../CustomTableToolbar';
import UbaCard from './UbaCard';
import {prepareValue, doFiltering,} from '../filtering';
import {enrichUbaDevicesWithRunTime,} from '../utils';
import {getUbaDevices,} from 'src/action-creators/UbaDevices';
import {useAuthDispatch,} from 'src/store/AuthProvider';

const POLLING_INTERVAL = 5000;

export default function CardView() {

	const filtersInitial = {
		ubaSN: '',
		batteryPN: '',
		status: '',
		machineName: '',
	};

	const pollingRef = useRef(null);

	const [page, setPage] = useState(0);
	const [rowsPerPage, setRowsPerPage] = useState(50);
	const [filters, setFilters] = useState(filtersInitial);

	const {ubaDevices,} = useUbaDevices();
	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	const ubaEnriched = enrichUbaDevicesWithRunTime(ubaDevices);
	let dataFiltered = filtering(ubaEnriched, 'asc', 'runtime', filters, prepareValue, doFiltering);
	const groupedData = dataFiltered.reduce((acc, item) => {
		const existing = acc.find(arr => arr[0]?.ubaSN === item.ubaSN);
		if (existing) {
			existing.push(item);
		} else {
			acc.push([item]);
		}
		return acc;
	}, []);
	dataFiltered = groupedData.map(item => {
		if (item.length === 2) {
			if (item[0]?.channel === ubaChannel.A) {//channel A should be first
				return item;
			} else {
				return [
					item[1],
					item[0],
				]
			}
		}

		if (item[0]?.channel === ubaChannel.A) {
			return [
				item[0],
				{},
			]
		}else {//this is B
			return [
				{
					ubaSN: item[0]?.ubaSN,//just for showing in the header
					machineName: item[0]?.machineName,//just for showing in the header
					name: item[0]?.name,//just for showing in the header
				},
				item[0],
			]
		}

		
	});

	useEffect(() => {
		clearInterval(pollingRef.current);
		pollingRef.current = setInterval(() => getUbaDevices(authDispatch, ubaDevicesDispatch, true), POLLING_INTERVAL);

		return () => clearInterval(pollingRef.current);
	}, []);

	return (
		<Container maxWidth="false">
			<Card sx={{mb: 1}} >
				<CustomToolbar/>
			</Card>

			<Card sx={{mb: 1}}>
				<CustomTableToolbar {...{filters}} setFilters={setFilters} setPage={setPage}/>
			</Card>

			<Grid container spacing={0.9} >
				{
					dataFiltered
						.slice(
							page * rowsPerPage,
							page * rowsPerPage + rowsPerPage
						)
						.map((row, key) => (
							<UbaCard key={key} {...{row}}/>
						))
				}
			</Grid>

			{dataFiltered && dataFiltered.length > 50 && (
				<Card sx={{mt: 2}}>
					<TablePagination
						page={page}
						component="div"
						count={dataFiltered.length}
						rowsPerPage={rowsPerPage}
						rowsPerPageOptions={[50, 100]}
						onPageChange={(event, page) => handleChangePage(page, setPage)}
						onRowsPerPageChange={event => handleChangeRowsPerPage(event.target.value, setPage, setRowsPerPage)}
					/>
				</Card>
			)}
		</Container>
	);
}
