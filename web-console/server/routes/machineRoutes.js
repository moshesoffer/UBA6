const express = require('express');
const router = express.Router();
const machineController = require('../controllers/machineController');

/**
 * @swagger
 * components:
 *   schemas:
 *     Machine:
 *       type: object
 *       required:
 *         - mac
 *         - name
 *         - ip
 *       properties:
 *         mac:
 *           type: string
 *           description: Machine MAC address (Primary Key)
 *         name:
 *           type: string
 *           description: Machine name
 *         ip:
 *           type: string
 *           description: Machine IP address
 *       example:
 *         mac: "00:11:22:33:44:55"
 *         name: "Test Machine"
 *         ip: "192.168.1.100"
 */

/**
 * @swagger
 * /machines:
 *   get:
 *     summary: Get all machines (this is being called today from uba service and web ui)
 *     tags: [Machines]
 *     responses:
 *       200:
 *         description: List of all machines
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 $ref: '#/components/schemas/Machine'
 */
router.get('/machines', machineController.getMachines);

/**
 * @swagger
 * /machines:
 *   post:
 *     summary: Create a new machine (this is being called today from uba service)
 *     tags: [Machines]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/Machine'
 *     responses:
 *       201:
 *         description: Machine created successfully
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
router.post('/machines', machineController.createMachine);

/**
 * @swagger
 * /machines/{mac}:
 *   patch:
 *     summary: Update a machine by MAC address (this is being called today from uba service)
 *     tags: [Machines]
 *     parameters:
 *       - in: path
 *         name: mac
 *         schema:
 *           type: string
 *         required: true
 *         description: MAC address of the machine to update
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               name:
 *                 type: string
 *                 description: Machine name
 *               ip:
 *                 type: string
 *                 description: Machine IP address
 *             example:
 *               name: "Test Machine Updated"
 *               ip: "192.168.1.101"
 *     responses:
 *       200:
 *         description: Machine updated successfully
 *       404:
 *         description: Machine not found
 *       500:
 *         description: Server error
 */
router.patch('/machines/:mac', machineController.updateMachine);

/**
 * @swagger
 * /machines/{mac}:
 *   delete:
 *     summary: Delete a machine by MAC address (this is being called by the web ui)
 *     tags: [Machines]
 *     parameters:
 *       - in: path
 *         name: mac
 *         schema:
 *           type: string
 *         required: true
 *         description: MAC address of the machine to delete
 *     responses:
 *       204:
 *         description: Machine deleted successfully
 *       404:
 *         description: Machine not found
 *       500:
 *         description: Server error
 */
router.delete('/machines/:mac', machineController.deleteMachine);

module.exports = router;
