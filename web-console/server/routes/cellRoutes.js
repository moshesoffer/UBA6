const express = require('express');
const router = express.Router();
const cellController = require('../controllers/cellController');

/**
 * @swagger
 * components:
 *   schemas:
 *     Cell:
 *       type: object
 *       required:
 *         - itemPN
 *         - chemistry
 *         - manufacturer
 *         - minVoltage
 *         - nomVoltage
 *         - maxVoltage
 *         - minCapacity
 *         - nomCapacity
 *         - minTemp
 *         - maxTemp
 *         - chargeOption
 *       properties:
 *         itemPN:
 *           type: string
 *           description: Unique part number (Primary Key)
 *         chemistry:
 *           type: string
 *           description: Chemistry type
 *         manufacturer:
 *           type: string
 *           description: Manufacturer name
 *         minVoltage:
 *           type: number
 *           description: Minimum voltage
 *         nomVoltage:
 *           type: number
 *           description: Nominal voltage
 *         maxVoltage:
 *           type: number
 *           description: Maximum voltage
 *         minCapacity:
 *           type: number
 *           description: Minimum capacity
 *         nomCapacity:
 *           type: number
 *           description: Nominal capacity
 *         minTemp:
 *           type: number
 *           description: Minimum temperature
 *         maxTemp:
 *           type: number
 *           description: Maximum temperature
 *         chargeOption:
 *           type: string
 *           description: Charge option
 *       example:
 *         itemPN: "PN12345"
 *         chemistry: "Li-ion"
 *         manufacturer: "ACME Corp"
 *         minVoltage: 2.5
 *         nomVoltage: 3.7
 *         maxVoltage: 4.2
 *         minCapacity: 2000
 *         nomCapacity: 2200
 *         minTemp: -20
 *         maxTemp: 60
 *         chargeOption: "Standard"
 */

/**
 * @swagger
 * /cells:
 *   get:
 *     summary: Get all cells (this is being called today from web ui)
 *     tags: [Cells]
 *     responses:
 *       200:
 *         description: List of all cells
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 $ref: '#/components/schemas/Cell'
 */
router.get('/cells', cellController.getCells);

/**
 * @swagger
 * /cells:
 *   post:
 *     summary: Create a new cell (this is being called today from web ui)
 *     tags: [Cells]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/Cell'
 *     responses:
 *       201:
 *         description: Cell created successfully
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
router.post('/cells', cellController.createCell);

/**
 * @swagger
 * /cells/{itemPN}:
 *   patch:
 *     summary: Update a cell by part number (this is being called today from web ui)
 *     tags: [Cells]
 *     parameters:
 *       - in: path
 *         name: itemPN
 *         schema:
 *           type: string
 *         required: true
 *         description: Part number of the cell to update
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               chemistry:
 *                 type: string
 *                 description: Chemistry type
 *               manufacturer:
 *                 type: string
 *                 description: Manufacturer name
 *               minVoltage:
 *                 type: number
 *                 description: Minimum voltage
 *               nomVoltage:
 *                 type: number
 *                 description: Nominal voltage
 *               maxVoltage:
 *                 type: number
 *                 description: Maximum voltage
 *               minCapacity:
 *                 type: number
 *                 description: Minimum capacity
 *               nomCapacity:
 *                 type: number
 *                 description: Nominal capacity
 *               minTemp:
 *                 type: number
 *                 description: Minimum temperature
 *               maxTemp:
 *                 type: number
 *                 description: Maximum temperature
 *               chargeOption:
 *                 type: string
 *                 description: Charge option
 *             example:
 *               chemistry: "Li-ion"
 *               manufacturer: "ACME Corp"
 *               minVoltage: 2.5
 *               nomVoltage: 3.7
 *               maxVoltage: 4.2
 *               minCapacity: 2000
 *               nomCapacity: 2200
 *               minTemp: -20
 *               maxTemp: 60
 *               chargeOption: "Standard"
 *     responses:
 *       200:
 *         description: Cell updated successfully
 *       404:
 *         description: Cell not found
 *       500:
 *         description: Server error
 */
router.patch('/cells/:itemPN', cellController.updateCell);

/**
 * @swagger
 * /cells/{itemPN}:
 *   delete:
 *     summary: Delete a cell by part number (this is being called today from web ui)
 *     tags: [Cells]
 *     parameters:
 *       - in: path
 *         name: itemPN
 *         schema:
 *           type: string
 *         required: true
 *         description: Part number of the cell to delete
 *     responses:
 *       204:
 *         description: Cell deleted successfully
 *       404:
 *         description: Cell not found
 *       500:
 *         description: Server error
 */
router.delete('/cells/:itemPN', cellController.deleteCell);

module.exports = router;
