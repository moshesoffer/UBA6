const logger = require('../utils/logger');
const {status,} = require('../utils/constants');
const {runTest, changeRunningTestStatus,} = require('../services/transactionsService');
const {
	getInstantTestResults,
	addInstantTestResults,
	getPendingRunningTests
} = require('../services/runningTestService');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

exports.addInstantTestResults = async (req, res) => {
	try {
		await withTimeout(addInstantTestResults(req.body), AWAIT_TIMEOUT);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('addInstantTestResults', error);
		res.sendStatus(500);
	}
};

//this is fetching the graph data for the instantTestResults
exports.getInstantTestResults = async (req, res) => {
	try {
		const result = await withTimeout(getInstantTestResults(req.params?.runningTestID), AWAIT_TIMEOUT);
		res.json(result);
	} catch (error) {
		logger.error('getInstantTestResults', error);
		res.sendStatus(500);
	}
};

exports.getAllPendingRunningTests = async (req, res) => {
	try {
		const result = await withTimeout(getPendingRunningTests(), AWAIT_TIMEOUT);
		res.json(result);
	} catch (error) {
		logger.error('getAllPendingRunningTests', error);
		res.sendStatus(500);
	}
};

//this is for starting the test. it will delete running tests and recreate them
//When starting a test then first deleting running tests on the related ubaSNs + channels
exports.runTest = async (req, res) => {
	try {
		const {ids} = await withTimeout(runTest(req.body), AWAIT_TIMEOUT);
		res.end();
	} catch (error) {
		logger.error('runTest', error);
		res.sendStatus(500);
	}
};

//if the running test is on both channels then going to find the other channel running test and do the action on it as well. not only on runningTestID
exports.changeRunningTestStatus = async (req, res) => {
  const validStatuses = new Set(Object.values(status));
  if (!validStatuses.has(req.body?.newTestStatus)) {
    return res.status(400).json({ error: 'Invalid newTestStatus value: ' + req.body?.newTestStatus });
  }
  try {
    await withTimeout(changeRunningTestStatus(req.body?.runningTestID, req.body?.testRoutineChannels, req.body?.ubaSN, req.body?.newTestStatus), AWAIT_TIMEOUT);
    res.end();
  } catch (err) {
    logger.error(`changeRunningTestStatus newTestStatus: [${req.body?.newTestStatus}] [${req.body?.runningTestID}] [${req.body?.testRoutineChannels}] [${req.body?.ubaSN}] test`, err);
    res.sendStatus(500);
  }
};
