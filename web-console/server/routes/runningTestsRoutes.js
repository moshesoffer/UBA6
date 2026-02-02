const express = require('express');
const router = express.Router();
const runningTestsController = require('../controllers/runningTestsController');

//this isnt in use currently, its for adding instantTestResults
router.post('/graph-data', runningTestsController.addInstantTestResults);

//this is fetching the graph data for the instantTestResults
router.get('/graph-data/:runningTestID', runningTestsController.getInstantTestResultsGraphData);

//this is for starting the test
router.post('/running-test', runningTestsController.runTest);

//this is for stoping the test
router.patch('/stop-test', runningTestsController.stopTest);

//this is for pausing the test
router.patch('/pause-test', runningTestsController.pauseTest);

//this is for resuming the test
router.patch('/resume-test', runningTestsController.resumeTest);

//this is for confirming the test is finished
router.patch('/confirm-test', runningTestsController.confirmTest);

module.exports = router;
