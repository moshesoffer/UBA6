const express = require('express');
const router = express.Router();
const reportController = require('../controllers/reportController');

/**
 * @swagger
 * components:
 *   schemas:
 *     Report:
 *       type: object
 *       properties:
 *         id:
 *           type: string
 *           description: Report ID
 *         testName:
 *           type: string
 *         batteryPN:
 *           type: string
 *         batterySN:
 *           type: string
 *         notes:
 *           type: string
 *         customer:
 *           type: string
 *         workOrderNumber:
 *           type: string
 *         approvedBy:
 *           type: string
 *         conductedBy:
 *           type: string
 *         cellSupplier:
 *           type: string
 *         cellBatch:
 *           type: string
 *         testRoutineChannels:
 *           type: string
 *         machineName:
 *           type: string
 *         machineMac:
 *           type: string
 *         status:
 *           type: integer
 *         timestampStart:
 *           type: string
 *         ubaSN:
 *           type: string
 *       example:
 *         id: "rpt-123"
 *         testName: "Cycle Test"
 *         batteryPN: "BAT-001"
 *         batterySN: "SN-001"
 *         notes: "Routine cycle test"
 *         customer: "ACME"
 *         workOrderNumber: "WO-123"
 *         approvedBy: "QA Lead"
 *         conductedBy: "Operator"
 *         cellSupplier: "SupplierX"
 *         cellBatch: "Batch42"
 *         testRoutineChannels: "1,2,3"
 *         machineName: "Test Machine"
 *         timestampStart: "2025-07-08T11:00:00Z"
 *         ubaSN: "UBA-001"
 */

/**
 * @swagger
 * /reports:
 *   post:
 *     summary: Create a report and its data (this is NOT BEING USED yet)
 *     tags: [Reports]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/Report'
 *     responses:
 *       201:
 *         description: Report and data created successfully
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 success:
 *                   type: boolean
 *       500:
 *         description: Server error
 */
router.post('/reports', reportController.createReportAndTestResult);

/**
 * @swagger
 * /reports/search:
 *   post:
 *     summary: Fetch all final reports (This is being called from web UI to fetch all reports)
 *     tags: [Reports]
 *     requestBody:
 *       required: false
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *     responses:
 *       200:
 *         description: List of all reports
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 $ref: '#/components/schemas/Report'
 *       500:
 *         description: Server error
 */
router.post('/reports/search', reportController.getReports);

/**
 * @swagger
 * /reports/{id}:
 *   patch:
 *     summary: Update a report by ID (this is being called from web UI to update report details and also from uba service when a test is finished it will update testResults)
 *     tags: [Reports]
 *     parameters:
 *       - in: path
 *         name: id
 *         schema:
 *           type: string
 *         required: true
 *         description: Report ID to update
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               testName:
 *                 type: string
 *               batteryPN:
 *                 type: string
 *               batterySN:
 *                 type: string
 *               cellPN:
 *                type: string
 *               notes:
 *                 type: string
 *               customer:
 *                 type: string
 *               workOrderNumber:
 *                 type: string
 *               approvedBy:
 *                 type: string
 *               conductedBy:
 *                 type: string
 *               cellSupplier:
 *                 type: string
 *               cellBatch:
 *                 type: string
 *               status:
 *                 type: integer
 *             example:
 *               testName: "Cycle Test Updated"
 *               batteryPN: "BAT-002"
 *               batterySN: "SN-002"
 *               cellPN: "CELL-001"
 *               notes: "Updated notes"
 *               customer: "ACME"
 *               workOrderNumber: "WO-124"
 *               approvedBy: "QA Lead"
 *               conductedBy: "Operator"
 *               cellSupplier: "SupplierY"
 *               cellBatch: "Batch43"
 *     responses:
 *       200:
 *         description: Report updated successfully
 *       404:
 *         description: Report not found
 *       500:
 *         description: Server error
 */
router.patch('/reports/:id', reportController.updateReportAndTestResult);

/**
 * @swagger
 * /test-results/search:
 *   post:
 *     summary: Fetch graph data for reports (this is being called from web UI to fetch report graph data)
 *     tags: [Reports]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *     responses:
 *       200:
 *         description: Graph data for reports
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *       500:
 *         description: Server error
 */
router.post('/test-results/search', reportController.getTestResults);

/**
 * @swagger
 * /test-results/{reportID}/{exportType}:
 *   get:
 *     summary: Download report graph as Excel or PDF (This is being called from web UI to download final report)
 *     tags: [Reports]
 *     parameters:
 *       - in: path
 *         name: reportID
 *         schema:
 *           type: string
 *         required: true
 *         description: Report ID
 *       - in: path
 *         name: exportType
 *         schema:
 *           type: string
 *           enum: [excel, pdf]
 *         required: true
 *         description: Export file type
 *     responses:
 *       200:
 *         description: File download
 *         content:
 *           application/octet-stream:
 *             schema:
 *               type: string
 *               format: binary
 *       404:
 *         description: Report or export type not found
 *       500:
 *         description: Server error
 */
router.get('/test-results/:reportID/:exportType', reportController.downloadReportsGraph);

module.exports = router;
