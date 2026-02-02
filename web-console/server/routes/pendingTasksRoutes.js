const express = require('express');
const router = express.Router();

const pendingTasksController = require('../controllers/pendingTasksController');

/**
 * @swagger
 * /pending-tasks:
 *   get:
 *     summary: Get all pending tasks (called from uba service)
 *     tags: [PendingTasks]
 *     parameters:
 *       - in: query
 *         name: machineMac
 *         schema:
 *           type: string
 *         required: true
 *         description: Filter by machine MAC address
 *     responses:
 *       200:
 *         description: Object containing arrays of pending running tests and pending connection uba devices
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 pendingReports:
 *                   type: array
 *                   items:
 *                     type: object
 *                 pendingRunningTests:
 *                   type: array
 *                   items:
 *                     type: object
 *                 pendingConnectionUbaDevices:
 *                   type: array
 *                   items:
 *                     type: object
 *                     properties:
 *                       machineMac:
 *                         type: string
 *                       comPort:
 *                         type: string
 *                       address:
 *                         type: string
 *                       action:
 *                         type: string
 *                         enum: [query, connect, disconnect]
 *       500:
 *         description: Server error
 */
router.get('/pending-tasks', pendingTasksController.getPendingTasks);

/**
 * @swagger
 * /pending-tasks:
 *   post:
 *     summary: Mark pending tasks as executed (called from uba service)
 *     tags: [PendingTasks]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               pendingUbaDevice:
 *                 type: object
 *                 required:
 *                   - machineMac
 *                   - address
 *                   - comPort
 *                   - actionResult
 *                   - action
 *                 properties:
 *                   machineMac:
 *                     type: string
 *                   address:
 *                     type: string
 *                   comPort:
 *                     type: string
 *                   ubaSN:
 *                     type: string
 *                   ubaChannel:
 *                     type: string
 *                   name:
 *                     type: string
 *                   fwVersion:
 *                     type: string
 *                   hwVersion:
 *                     type: string
 *                   actionResult:
 *                     type: string
 *                     enum: [success, deviceNotFound]
 *                   action:
 *                     type: string
 *                     enum: [query, addToWatchList, removeFromWatchList]
 *     responses:
 *       200:
 *         description: Pending tasks marked as executed
 *       400:
 *         description: Invalid request
 *       500:
 *         description: Server error
 */
router.post('/pending-tasks', pendingTasksController.pendingTasksExecuted);

/**
 * @swagger
 * /query-uba-devices:
 *   post:
 *     summary: Query uba devices (called from Web UI)
 *     tags: [PendingTasks]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required:
 *               - machineMac
 *               - address
 *               - comPort
 *             properties:
 *               machineMac:
 *                 type: string
 *               address:
 *                 type: string
 *               comPort:
 *                 type: string
 *               ubaSN:
 *                 type: string
 *                 description: Optional
 *     responses:
 *       200:
 *         description: Uba device query result
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 machineMac:
 *                   type: string
 *                 address:
 *                   type: string
 *                 comPort:
 *                   type: string
 *                 ubaSN:
 *                   type: string
 *                 ubaChannel:
 *                   type: string
 *                 name:
 *                   type: string
 *                 actionResult:
 *                   type: string
 *                 action:
 *                   type: string
 *       400:
 *         description: Invalid request
 *       500:
 *         description: Server error
 */
router.post('/query-uba-devices', pendingTasksController.queryUbaDevice);

module.exports = router;