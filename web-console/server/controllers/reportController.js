const logger = require('../utils/logger');
const {getReports, getReportData,updateReport} = require('../services/reportService');
const { downloadReportsGraph } = require('../utils/downloadReportGraphHelper');
const { createReportAndData } = require('../services/transactionsService');

exports.createReportAndData = async (req, res) => {
	try {
		await createReportAndData(req.body);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('createReportAndData', error);
		res.sendStatus(500);
	}
};

//this is for fetching all final reports
exports.getReports = async (req, res) => {
	try {
		const result = await getReports(req.body);
		res.json(result);
	} catch (error) {
		logger.error('getReports', error);
		res.sendStatus(500);
	}
};

//this is for updating reports and report data, when clicking edit on the modal itself
exports.updateReport = async (req, res) => {
	try {
		await updateReport(req.params?.id, req.body);
		res.end();
	} catch (error) {
		logger.error('updateReport', error);
		res.sendStatus(500);
	}
};

//this for fetching graph data of final reports of several reports or for one
exports.getReportData = async (req, res) => {
	try {
		const result = await getReportData(req.body);
		res.json(result);
	} catch (error) {
		logger.error('getReportData', error);
		res.sendStatus(500);
	}
};

//this is for download excel or pdf
exports.downloadReportsGraph = async (req, res) => {
	await downloadReportsGraph(req, res);
};



