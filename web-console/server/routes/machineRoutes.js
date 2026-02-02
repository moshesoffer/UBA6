const express = require('express');
const router = express.Router();
const machineController = require('../controllers/machineController');

router.get('/machines', machineController.getMachines);

//currently not in use
router.post('/machines', machineController.createMachine);

//currently not in use
router.patch('/machines/:mac', machineController.updateMachine);

//currently not in use
router.delete('/machines/:mac', machineController.deleteMachine);

module.exports = router;
