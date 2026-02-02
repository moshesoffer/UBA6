import {useState,} from 'react';

import Box from '@mui/material/Box';
import Stack from '@mui/material/Stack';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';

import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useReports, useReportsDispatch,} from 'src/store/ReportsProvider';
import {useSettingsDispatch,} from 'src/store/SettingsProvider';
import {updateReport,} from 'src/action-creators/Reports';
import {setModal,} from 'src/actions/Auth';
import {setCurrentReport, updateCurrentReport, } from 'src/actions/Reports';
import withModalView from 'src/components/withModalView';
import {getText,} from 'src/services/string-definitions';
import {validateString,} from 'src/utils/validators';
import {getInputValue, handleInputChange,} from 'src/utils/helper';

function EditReport(props) {

	const {metadata,} = props;

	const [testNameError, setTestNameError] = useState('');
	const [batteryPNError, setBatteryPNError] = useState('');
	const [batterySNError, setBatterySNError,] = useState('');
	const [notesError, setNotesError,] = useState('');
	const [customerError, setCustomerError,] = useState('');
	const [workOrderNumberError, setWorkOrderNumberError,] = useState('');
	const [approvedByError, setApprovedByError,] = useState('');
	const [conductedByError, setConductedByError,] = useState('');
	const [cellSupplierError, setCellSupplierError,] = useState('');
	const [cellBatchError, setCellBatchError,] = useState('');

	const {currentReport,} = useReports();

	const authDispatch = useAuthDispatch();
	const reportsDispatch = useReportsDispatch();
	const settingsDispatch = useSettingsDispatch();

	const handleChange = (dataKey, dataValue) => {
		handleInputChange(reportsDispatch, updateCurrentReport, dataKey, dataValue);
	};

	const handleSubmitClick = () => {
		updateReport(authDispatch, reportsDispatch, settingsDispatch, metadata, currentReport);
		reportsDispatch(setCurrentReport({}));
		authDispatch(setModal(''));
	};

	return (
		<Box sx={{background: theme => theme.palette.grey[200], borderRadius: 2, width: '22vw',}}>
			<Typography level="title-lg" sx={{px: 3.5, py: 2,}}>
				{getText('reportsPage.editReport.EDIT_REPORT')}
			</Typography>

			<Stack alignItems="center" spacing={2} sx={{
				background: theme => theme.palette.grey[0],
				p: 2,
				maxHeight: 'calc(98vh - 108px)',
				overflowY: 'auto',
			}}>
				<TextField
					fullWidth
					size="small"
					error={validateString(testNameError)}
					label={getText('mainPage.wizardOne.TEST_PLAN_NAME')}
					value={getInputValue(currentReport, 'testName')}
					onChange={event => handleChange('testName', event.target.value)}
					helperText={testNameError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(batteryPNError)}
					label={getText('testEditor.BATTERY_P_N')}
					value={getInputValue(currentReport, 'batteryPN')}
					onChange={event => handleChange('batteryPN', event.target.value)}
					helperText={batteryPNError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(batterySNError)}
					label={getText('reportsPage.BATTERY_S_N')}
					value={getInputValue(currentReport, 'batterySN')}
					onChange={event => handleChange('batterySN', event.target.value)}
					helperText={batterySNError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(notesError)}
					label={getText('mainPage.wizardOne.NOTES')}
					value={getInputValue(currentReport, 'notes')}
					onChange={event => handleChange('notes', event.target.value)}
					helperText={notesError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(customerError)}
					label={getText('mainPage.wizardOne.CUSTOMER')}
					value={getInputValue(currentReport, 'customer')}
					onChange={event => handleChange('customer', event.target.value)}
					helperText={customerError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(workOrderNumberError)}
					label={getText('mainPage.wizardOne.WORK_ORDER_NUMBER')}
					value={getInputValue(currentReport, 'workOrderNumber')}
					onChange={event => handleChange('workOrderNumber', event.target.value)}
					helperText={workOrderNumberError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(approvedByError)}
					label={getText('mainPage.wizardOne.APPROVED_BY')}
					value={getInputValue(currentReport, 'approvedBy')}
					onChange={event => handleChange('approvedBy', event.target.value)}
					helperText={approvedByError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(conductedByError)}
					label={getText('mainPage.wizardOne.CONDUCTED_BY')}
					value={getInputValue(currentReport, 'conductedBy')}
					onChange={event => handleChange('conductedBy', event.target.value)}
					helperText={conductedByError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(cellSupplierError)}
					label={getText('mainPage.wizardOne.CELL_SUPPLIER')}
					value={getInputValue(currentReport, 'cellSupplier')}
					onChange={event => handleChange('cellSupplier', event.target.value)}
					helperText={cellSupplierError}
				/>

				<TextField
					fullWidth
					size="small"
					error={validateString(cellBatchError)}
					label={getText('mainPage.wizardOne.CELL_DATE')}
					value={getInputValue(currentReport, 'cellBatch')}
					onChange={event => handleChange('cellBatch', event.target.value)}
					helperText={cellBatchError}
				/>
			</Stack>

			<Stack direction="row" justifyContent="center" sx={{
				mt: -2,
				p: 2,
				background: theme => theme.palette.grey[0],
				borderRadius: 2,
			}}>
				<Button variant="contained" onClick={handleSubmitClick} sx={{width: 100,}}>
					{getText('common.UPDATE')}
				</Button>
			</Stack>
		</Box>
	);
}

export default withModalView(EditReport);
