const logger = require('../utils/logger');
const { ubaDeviceConnectedMs } = require('./constants');
const lastInstantTestResults = new Map();

const setLastInstantTestResult = (runningTestID, testResult) => {
	const timestamp = new Date(testResult.timestamp + ' UTC');
	lastInstantTestResults.set(runningTestID, {...testResult, memCreatedTime: new Date(), timestamp});
};

const getLastInstantTestResult = (runningTestID) => {
	return lastInstantTestResults.get(runningTestID);
};

const deleteLastInstantTestResult = (runningTestID) => {
	lastInstantTestResults.delete(runningTestID);
};

const clearAllLastInstantTestResults = () => {
	lastInstantTestResults.clear();
	logger.info('Cleared all last instant test results from memory.');
};

const getConnectedAmount = () => {
	const now = Date.now();
  	let count = 0;
	for (const { memCreatedTime } of lastInstantTestResults.values()) {
		if (now - memCreatedTime.getTime() <= ubaDeviceConnectedMs) { // last 1 minute
			count++;
		}
	}
  	return count;
}

module.exports = {
	setLastInstantTestResult,
	getLastInstantTestResult,
	getConnectedAmount,
	deleteLastInstantTestResult,
	clearAllLastInstantTestResults,
}