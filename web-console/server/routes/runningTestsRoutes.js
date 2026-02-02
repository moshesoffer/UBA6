const express = require('express');
const router = express.Router();
const runningTestsController = require('../controllers/runningTestsController');

/**
 * @swagger
 * /instant-test-results:
 *   post:
 *     summary: Add instant test results. this is for adding instantTestResults (this is being called today from uba service)
 *     tags: [RunningTests]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *     responses:
 *       201:
 *         description: Instant test results added
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
router.post('/instant-test-results', runningTestsController.addInstantTestResults);

/**
 * @swagger
 * /instant-test-results/{runningTestID}:
 *   get:
 *     summary: Get instant test results graph data (this is being called today from web ui for showing the graph)
 *     tags: [RunningTests]
 *     parameters:
 *       - in: path
 *         name: runningTestID
 *         schema:
 *           type: string
 *         required: true
 *         description: Running test ID
 *     responses:
 *       200:
 *         description: Graph data for the running test
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 type: object
 *       404:
 *         description: Running test not found
 *       500:
 *         description: Server error
 */
router.get('/instant-test-results/:runningTestID', runningTestsController.getInstantTestResults);

/**
 * @swagger
 * /pending-tests:
 *   get:
 *     summary: Get all pending running tests (currently not in use)
 *     tags: [RunningTests]
 *     responses:
 *       200:
 *         description: List of all pending running tests
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 type: object
 *       500:
 *         description: Server error
 */
router.get('/pending-tests', runningTestsController.getAllPendingRunningTests);

/**
 * @swagger
 * /running-test:
 *   post:
 *     summary: Start a running test (this is being called today from web ui)
 *     tags: [RunningTests]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *     responses:
 *       200:
 *         description: Test started
 *       500:
 *         description: Server error
 */
router.post('/running-test', runningTestsController.runTest);

/**
 * @swagger
 * /change-running-test-status:
 *   patch:
 *     summary: Perform an action (stop, pause, resume, confirm, etc..) on a running test (this is being called today from uba service)
 *     tags: [RunningTests]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               runningTestID:
 *                 type: string
 *               testRoutineChannels:
 *                 type: string
 *               ubaSN:
 *                 type: string
 *               newTestStatus:
 *                 type: integer
 *     responses:
 *       200:
 *         description: Test action performed
 *       400:
 *         description: Invalid action or request
 *       500:
 *         description: Server error
 */
router.patch('/change-running-test-status', runningTestsController.changeRunningTestStatus);

module.exports = router;
