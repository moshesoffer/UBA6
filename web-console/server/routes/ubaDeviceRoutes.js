const express = require('express');
const router = express.Router();
const ubaDeviceController = require('../controllers/ubaDeviceController');

/**
 * @swagger
 * components:
 *   schemas:
 *     UBADevice:
 *       type: object
 *       properties:
 *         ubaSN:
 *           type: string
 *           description: UBA device serial number (Primary Key)
 *         ubaChannel:
 *           type: integer
 *           description: UBA channel number
 *         machineMac:
 *           type: string
 *           description: MAC address of the connected machine
 *         name:
 *           type: string
 *           description: Device name
 *         comPort:
 *           type: string
 *           description: Communication port
 *         address:
 *           type: string
 *           description: Device address
 *       example:
 *         ubaSN: "UBA-001"
 *         ubaChannel: 1
 *         machineMac: "00:11:22:33:44:55"
 *         name: "UBA Device 1"
 *         comPort: "COM3"
 *         address: "0x01"
 */

/**
 * @swagger
 * /uba-devices:
 *   get:
 *     summary: Get all UBA devices and their status (this is being called today from uba service and from web ui)
 *     tags: [UBADevices]
 *     responses:
 *       200:
 *         description: List of all UBA devices with status
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *       500:
 *         description: Server error
 */
router.get('/uba-devices', ubaDeviceController.getUbaDevices);

/**
 * @swagger
 * /uba-devices:
 *   post:
 *     summary: Create a new UBA device and start a test (this is being called today from web ui)
 *     tags: [UBADevices]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/UBADevice'
 *     responses:
 *       201:
 *         description: UBA device and test created successfully
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 type: string
 *       500:
 *         description: Server error
 */
router.post('/uba-devices', ubaDeviceController.createUbaAndTest);

/**
 * @swagger
 * /uba-devices/{serial}:
 *   patch:
 *     summary: Update a UBA device by serial number (this is being called today from web ui)
 *     tags: [UBADevices]
 *     parameters:
 *       - in: path
 *         name: serial
 *         schema:
 *           type: string
 *         required: true
 *         description: UBA device serial number
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               machineMac:
 *                 type: string
 *               name:
 *                 type: string
 *               comPort:
 *                 type: string
 *               address:
 *                 type: string
 *             example:
 *               machineMac: "00:11:22:33:44:55"
 *               name: "UBA Device 1 Updated"
 *               comPort: "COM4"
 *               address: "0x02"
 *     responses:
 *       200:
 *         description: UBA device updated successfully
 *       404:
 *         description: UBA device not found
 *       500:
 *         description: Server error
 */
router.patch('/uba-devices/:serial', ubaDeviceController.updateUbaDevice);

/**
 * @swagger
 * /uba-devices/{serial}:
 *   delete:
 *     summary: Delete a UBA device and its test by serial number (this is being called today from web ui)
 *     tags: [UBADevices]
 *     parameters:
 *       - in: path
 *         name: serial
 *         schema:
 *           type: string
 *         required: true
 *         description: UBA device serial number
 *     responses:
 *       204:
 *         description: UBA device and test deleted successfully
 *       404:
 *         description: UBA device not found
 *       500:
 *         description: Server error
 */
router.delete('/uba-devices/:serial', ubaDeviceController.deleteUbaDeviceAndTest);

module.exports = router;
