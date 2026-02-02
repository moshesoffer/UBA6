const path = require('node:path');

const reportsDataPath = process.env.TEST_ENV ? path.join(__dirname, '../../../amicell-tests/reports-data') : path.join(__dirname, '../../../amicell-data/reports-data');
const testResultsFileName = 'testResults.json';
const ubaDeviceConnectedMs = 60_000; // 1 minute

const ubaChannels = {
	A: 'A',
	B: 'B',
	AB: 'AB',
};

const testChannels = [
	'A',
	'B',
];

const TEST_ROUTINE_CHANNELS = {
	A_OR_B: 'A-or-B',
	A_AND_B: 'A-and-B',
};

const STANDBY= 0x0001;
const STOPPED= 0x0002;
const ABORTED= 0x0004;
const FINISHED= 0x0008;
const SAVED= 0x0010;
const RUNNING= 0x0020;
const PAUSED= 0x0040;
const PENDING= 0x0100;
const IS_TEST_RUNNING = PENDING|RUNNING|PAUSED;

const status = {
	STANDBY: STANDBY,
	STOPPED: STOPPED,
	ABORTED: ABORTED,
	FINISHED: FINISHED,
	SAVED: SAVED,
	RUNNING: RUNNING,
	PAUSED: PAUSED,
	PENDING: PENDING,
	PENDING_STANDBY: PENDING|STANDBY,
	PENDING_STOP: PENDING|STOPPED,
	PENDING_RUNNING: PENDING|RUNNING,
	PENDING_SAVE: PENDING|SAVED,
	PENDING_PAUSE: PENDING|PAUSED,
	IS_TEST_RUNNING: PENDING|RUNNING|PAUSED,
};

const isTestRunning =  (runningStatus) => (runningStatus & status.IS_TEST_RUNNING) !== 0;
const isTestInPending =  (runningStatus) => (runningStatus & status.PENDING) !== 0;


const DATE_RANGE = {
	lastWeek: '1 WEEK',
	lastMonth: '1 MONTH',
	last6Months: '6 MONTH',
	lastYear: '1 YEAR',
};

const APIS = {
	apiInitials:'/web-console',
	get machinesApi() { return this.apiInitials + '/machines' },
	get ubaDevicesApi() { return this.apiInitials + '/uba-devices' },
	get instantTestResultsApi() { return this.apiInitials + '/instant-test-results' },
	get cellsApi() { return this.apiInitials + '/cells' },
	get testRoutinesApi() { return this.apiInitials + '/test-routines' },
	get startTestApi() { return this.apiInitials + '/running-test' },
	get changeRunningTestStatusApi() { return this.apiInitials + '/change-running-test-status' },
	get getAllPendingRunningTestsApi() { return this.apiInitials + '/pending-tests' },
	get createReportApi() { return this.apiInitials + '/reports' },
	get getReportsApi() { return this.apiInitials + '/reports/search' },
	get updateReportApi() { return this.apiInitials + '/reports' },
	get reportsGraphApi() { return this.apiInitials + '/test-results/search' },
	get pendingTasksApi() { return this.apiInitials + '/pending-tasks' },
	get queryUbaDevicesApi() { return this.apiInitials + '/query-uba-devices' }
};

module.exports = {
	ubaChannels,
	testChannels,
	status,
	DATE_RANGE,
	TEST_ROUTINE_CHANNELS,
	isTestRunning,
	isTestInPending,
	APIS,
	reportsDataPath,
	testResultsFileName,
	ubaDeviceConnectedMs
};
