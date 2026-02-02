const logger = require('../utils/logger');
const {status,RUNNING_TEST_ACTIONS} = require('../utils/constants');
const {runTest, actionOnRunningTest,} = require('../services/transactionsService');
const {
	changeTestStatus,
	getInstantTestResultsGraphData,
	addInstantTestResults
} = require('../services/runningTestService');

exports.addInstantTestResults = async (req, res) => {
	try {
		await addInstantTestResults(req.body);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('addInstantTestResults', error);
		res.sendStatus(500);
	}
};

//this is fetching the graph data for the instantTestResults
exports.getInstantTestResultsGraphData = async (req, res) => {
	try {
		const result = await getInstantTestResultsGraphData(req.params?.runningTestID);
		/*remove random elements - just for show DELETE THIS TODO!!!
		const arrCopy = [...result];
		for (let i = 0; i < 50; i++) {
			const randomIndex = Math.floor(Math.random() * arrCopy.length);
			arrCopy.splice(randomIndex, 1);
		}
		res.json(arrCopy);
		*/

		res.json(result);
	} catch (error) {
		logger.error('getInstantTestResultsGraphData', error);
		res.sendStatus(500);
	}
};

//this is for starting the test. it will delete running tests and recreate them
exports.runTest = async (req, res) => {
	try {
		const {ids,errorEnum} = await runTest(req.body);
		res.end();
		//TODO!!! remove the setTimeout, this is only for presentation
		if(process.env.TEST_ENV) return;
		setTimeout(async () => {
			try {
				for (const id of ids) {
					logger.info(`running-test start changeTestStatus [${id}]`);
					await changeTestStatus(id, errorEnum === 0 ? status.RUNNING : status.ABORTED);
					logger.info(`running-test finish changeTestStatus [${id}]`);
				}
				logger.info(`running-test finish all ids`, {ids: ids});
			} catch (error) {
				logger.error('Error changing status to RUNNING', error);
			}
		}, 7000);
	} catch (error) {
		logger.error('runTest', error);
		res.sendStatus(500);
	}
};

//this is for stoping the test
exports.stopTest = async (req, res) => {
	try {
		await actionOnRunningTest(req.body?.runningTestID, req.body?.testRoutineChannels, req.body?.ubaSN, RUNNING_TEST_ACTIONS.STOP);
		res.end();
	} catch (error) {
		logger.error('stopTest', error);
		res.sendStatus(500);
	}
};

//this is for pausing the test
exports.pauseTest = async (req, res) => {
	try {
		await actionOnRunningTest(req.body?.runningTestID, req.body?.testRoutineChannels, req.body?.ubaSN, RUNNING_TEST_ACTIONS.PAUSE);
		res.end();
	} catch (error) {
		logger.error('pauseTest', error);
		res.sendStatus(500);
	}
};

//this is for resuming the test
exports.resumeTest = async (req, res) => {
	try {
		await actionOnRunningTest(req.body?.runningTestID, req.body?.testRoutineChannels, req.body?.ubaSN, RUNNING_TEST_ACTIONS.RESUME);
		res.end();
	} catch (error) {
		logger.error('resumeTest', error);
		res.sendStatus(500);
	}
};

//this is for confirming(finished) the test
exports.confirmTest = async (req, res) => {
	try {
		await actionOnRunningTest(req.body?.runningTestID, req.body?.testRoutineChannels, req.body?.ubaSN, RUNNING_TEST_ACTIONS.CONFIRM);
		res.end();
	} catch (error) {
		logger.error('confirmTest', error);
		res.sendStatus(500);
	}
};