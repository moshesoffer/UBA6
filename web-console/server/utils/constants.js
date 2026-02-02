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

const status = {
	STANDBY: 0,
	STOPPED: 3,
	ABORTED: 4,
	FINISHED: 5,
	SAVED: 6,
	RUNNING: 33,
	PAUSED: 34,
	PENDING_STANDBY: 48,
	PENDING_SAVE: 22,
	PENDING_PAUSE: 50,
	PENDING_STOP: 51,
	PENDING_RUNNING: 49,
};

const isTestRunning = (runningStatus) => [status.RUNNING, status.PAUSED, status.PENDING_SAVE, status.PENDING_RUNNING, status.PENDING_PAUSE, status.PENDING_STOP, status.PENDING_STANDBY].includes(runningStatus);

const isTestInPending = (runningStatus) => [status.PENDING_SAVE, status.PENDING_RUNNING, status.PENDING_PAUSE, status.PENDING_STOP, status.PENDING_STANDBY].includes(runningStatus);

const DATE_RANGE = {
	lastWeek: '1 WEEK',
	lastMonth: '1 MONTH',
	last6Months: '6 MONTH',
	lastYear: '1 YEAR',
};

const RUNNING_TEST_ACTIONS = {
	STOP: 'stop',
	PAUSE: 'pause',
	RESUME: 'resume',
	CONFIRM: 'confirm',
};

const APIS = {
	apiInitials:'/web-console',
	get machinesApi() { return this.apiInitials + '/machines' },
	get ubaDevicesApi() { return this.apiInitials + '/uba-devices' },
	get graphDataApi() { return this.apiInitials + '/graph-data' },
	get cellsApi() { return this.apiInitials + '/cells' },
	get testRoutinesApi() { return this.apiInitials + '/test-routines' },
	get startTestApi() { return this.apiInitials + '/running-test' },
	get stopTestApi() { return this.apiInitials + '/stop-test' },
	get pauseTestApi() { return this.apiInitials + '/pause-test' },
	get resumeTestApi() { return this.apiInitials + '/resume-test' },
	get confirmTestApi() { return this.apiInitials + '/confirm-test' },
	get createReportApi() { return this.apiInitials + '/reports-and-data' },
	get getReportsApi() { return this.apiInitials + '/reports' },
	get updateReportApi() { return this.apiInitials + '/reports' },
	get reportsGraphApi() { return this.apiInitials + '/reports-graph' },
};

module.exports = {
	ubaChannels,
	testChannels,
	status,
	DATE_RANGE,
	TEST_ROUTINE_CHANNELS,
	RUNNING_TEST_ACTIONS,
	isTestRunning,
	isTestInPending,
	APIS,
};
