import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import { useEffect, useState } from 'react';
import withModalView from 'src/components/withModalView';
import { getText, } from 'src/services/string-definitions';
import { useReports } from 'src/store/ReportsProvider';

function DownloadReport() {

	const {currentReport, } = useReports();
	//const authDispatch = useAuthDispatch();
	const [pdfDownloadLink, setPdfDownloadLink] = useState('');
	const [excelDownloadLink, setExcelDownloadLink] = useState('');
	//const reportsDispatch = useReportsDispatch();

	useEffect(() => {
		if (currentReport) {
			//console.log('currentReport', currentReport);
			const apiUrl = `${import.meta.env.VITE_API_URL}/test-results/${currentReport.id}/`;
			setPdfDownloadLink(apiUrl + 'PDF');
			setExcelDownloadLink(apiUrl + 'XSLX');
		}

	}, [currentReport,]);

	return (
		<Box sx={{background: theme => theme.palette.grey[200], borderRadius: 2, width: '20vw',}}>
			<Typography level="title-lg" sx={{px: 3.5, py: 2,}}>
				{getText('reportsPage.DOWNLOAD_REPORT')}
			</Typography>

			<Box alignItems="center" sx={{background: theme => theme.palette.grey[0], p: 2,}}/>

			<Stack alignItems="center" spacing={2} sx={{
				mt: -2,
				p: 2,
				background: theme => theme.palette.grey[0],
				borderRadius: 2,
			}}>

				<a href={excelDownloadLink} target="_blank" rel="noopener noreferrer" style={{width: '100%'}}>
					<Button fullWidth variant="contained" >
						{getText('reportsPage.DOWNLOAD_XSLX')}
					</Button>
                </a>

				<a href={pdfDownloadLink} target="_blank" rel="noopener noreferrer" style={{width: '100%'}}>
					<Button fullWidth variant="contained" >
						{getText('reportsPage.DOWNLOAD_PDF')}
					</Button>
                </a>

			</Stack>
		</Box>
	);
}

export default withModalView(DownloadReport);
