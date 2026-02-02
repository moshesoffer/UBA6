const express = require('express');
const router = express.Router();
const ubaDeviceController = require('../controllers/ubaDeviceController');

router.get('/uba-devices', ubaDeviceController.getUbaDevices);

router.post('/uba-devices', ubaDeviceController.createUbaAndTest);

router.patch('/uba-devices/:serial', ubaDeviceController.updateUbaDevice);

router.delete('/uba-devices/:serial', ubaDeviceController.deleteUbaDeviceAndTest);

module.exports = router;
