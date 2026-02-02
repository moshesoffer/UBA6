const express = require('express');
const router = express.Router();
const cellController = require('../controllers/cellController');

router.get('/cells', cellController.getCells);

router.post('/cells', cellController.createCell);

router.patch('/cells/:itemPN', cellController.updateCell);

router.delete('/cells/:itemPN', cellController.deleteCell);

module.exports = router;
