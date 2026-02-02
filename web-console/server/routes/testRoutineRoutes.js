const express = require('express');
const router = express.Router();
const testRoutineController = require('../controllers/testRoutineController');

/**
 * @swagger
 * components:
 *   schemas:
 *     TestRoutine:
 *       type: object
 *       properties:
 *         testName:
 *           type: string
 *         isLocked:
 *           type: boolean
 *         batteryPN:
 *           type: string
 *         batterySN:
 *           type: string
 *         cellPN:
 *           type: string
 *         noCellSerial:
 *           type: integer
 *         noCellParallel:
 *           type: integer
 *         maxPerBattery:
 *           type: integer
 *         ratedBatteryCapacity:
 *           type: number
 *         channel:
 *           type: integer
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
 *         plan:
 *           type: array
 *           items:
 *             type: object
 *       example:
 *         testName: "Routine 1"
 *         isLocked: false
 *         batteryPN: "BAT-001"
 *         batterySN: "SN-001"
 *         cellPN: "CELL-001"
 *         noCellSerial: 4
 *         noCellParallel: 2
 *         maxPerBattery: 8
 *         ratedBatteryCapacity: 2200
 *         channel: 1
 *         notes: "Standard test"
 *         customer: "ACME"
 *         workOrderNumber: "WO-123"
 *         approvedBy: "QA Lead"
 *         conductedBy: "Operator"
 *         cellSupplier: "SupplierX"
 *         cellBatch: "Batch42"
 *         plan: []
 */

/**
 * @swagger
 * /test-routines:
 *   get:
 *     summary: Get all test routines (this is being called today from web ui)
 *     tags: [TestRoutines]
 *     responses:
 *       200:
 *         description: List of all test routines
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 $ref: '#/components/schemas/TestRoutine'
 */
router.get('/test-routines', testRoutineController.getTestRoutines);

/**
 * @swagger
 * /test-routines:
 *   post:
 *     summary: Create a new test routine (this is being called today from web ui)
 *     tags: [TestRoutines]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/TestRoutine'
 *     responses:
 *       201:
 *         description: Test routine created successfully
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
router.post('/test-routines', testRoutineController.createTestRoutine);

/**
 * @swagger
 * /test-routines/{id}:
 *   patch:
 *     summary: Update a test routine by ID (this is being called today from web ui)
 *     tags: [TestRoutines]
 *     parameters:
 *       - in: path
 *         name: id
 *         schema:
 *           type: string
 *         required: true
 *         description: Test routine ID to update
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/TestRoutine'
 *     responses:
 *       200:
 *         description: Test routine updated successfully
 *       404:
 *         description: Test routine not found
 *       500:
 *         description: Server error
 */
router.patch('/test-routines/:id', testRoutineController.updateTestRoutine);

/**
 * @swagger
 * /test-routines/{id}:
 *   delete:
 *     summary: Delete a test routine by ID (this is being called today from web ui)
 *     tags: [TestRoutines]
 *     parameters:
 *       - in: path
 *         name: id
 *         schema:
 *           type: string
 *         required: true
 *         description: Test routine ID to delete
 *     responses:
 *       204:
 *         description: Test routine deleted successfully
 *       404:
 *         description: Test routine not found
 *       500:
 *         description: Server error
 */
router.delete('/test-routines/:id', testRoutineController.deleteTestRoutine);

module.exports = router;
