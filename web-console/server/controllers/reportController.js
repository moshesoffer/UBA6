const logger = require('../utils/logger');
const {getReports, getTestResults,updateReport} = require('../services/reportService');
const { downloadReportsGraph } = require('../utils/downloadReportGraphHelper');
const { createReportAndTestResult, updateReportAndTestResult } = require('../services/transactionsService');

exports.createReportAndTestResult = async (req, res) => {
	try {
		await createReportAndTestResult(req.body);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('createReportAndTestResult', error);
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
exports.updateReportAndTestResult = async (req, res) => {
	try {
		await updateReportAndTestResult(req.params?.id, req.body);
		res.end();
	} catch (error) {
		logger.error('updateReportAndTestResult', error);
		res.sendStatus(500);
	}
};

//this for fetching graph data of final reports of several reports or for one
exports.getTestResults = async (req, res) => {
	try {
		const result = await getTestResults(req.body);
		res.json(result);
	} catch (error) {
		logger.error('getTestResults', error);
		res.sendStatus(500);
	}
};

//this is for download excel or pdf
exports.downloadReportsGraph = async (req, res) => {
	await downloadReportsGraph(req, res);
};



