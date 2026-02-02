const express = require('express');
const router = express.Router();
const reportController = require('../controllers/reportController');

//this is for creating report and reportData
router.post('/reports-and-data', reportController.createReportAndData);

//this is for fetching all final reports
router.post('/reports', reportController.getReports);

//this is for updating report
router.patch('/reports/:id', reportController.updateReport);

//this for fetching graph data of final reports of several reports or for one
router.post('/reports-graph', reportController.getReportData);

//this is for download excel or pdf
router.get('/reports-graph/:reportID/:exportType', reportController.downloadReportsGraph);

module.exports = router;
