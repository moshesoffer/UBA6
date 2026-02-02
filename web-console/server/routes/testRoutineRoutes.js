const express = require('express');
const router = express.Router();
const testRoutineController = require('../controllers/testRoutineController');

router.get('/test-routines', testRoutineController.getTestRoutines);

router.post('/test-routines', testRoutineController.createTestRoutine);

router.patch('/test-routines/:id', testRoutineController.updateTestRoutine);

router.delete('/test-routines/:id', testRoutineController.deleteTestRoutine);

module.exports = router;
