import {getItem,setItem} from 'src/utils/localStorage';

export const DUP = 'dup';

export const TITLE_WIDTH = 200;
export const SECOND_PART__WIDTH = 221;

export const notificationSeverity = {
	SUCCESS: 'success',
	INFO: 'info',
	WARNING: 'warning',
	ERROR: 'error',
};

export const testTypeNames = {
	DELAY: 'delay',
	CHARGE: 'charge',
	DISCHARGE: 'discharge',
	LOOP: 'loop',
};

export const chargeLimitParts = {
	DATA_VALUE: 'dataValue',
	DATA_UNIT: 'dataUnit',
};

export const pageStateList = {
	TABLE_VIEW: 'tableView',
	CARDS_VIEW: 'cardsView',
	WIZARD_ZERO: 'wizardZero',
	WIZARD_ONE: 'wizardOne',
	WIZARD_TWO: 'wizardTwo',
	RUN_BATCH_TEST: 'runBatchTest',
};

export const getSelectedCardsOrTableView = () => getItem('selectedCardsOrTableView') || pageStateList.TABLE_VIEW;

export const setSelectedCardsOrTableView = (value) => setItem('selectedCardsOrTableView', value);

export const navigationPaths = {
	MAIN_PAGE: '',
	TEST_ROUTINES: 'test-routines',
	REPORTS: 'reports',
	SETTINGS: 'settings',
	USERS: 'users',
}

export const ubaChannel = {
	// eslint-disable-next-line id-length
	A: 'A',
	// eslint-disable-next-line id-length
	B: 'B',
	AB: 'AB',
}
export const UBA_CHANNEL_LIST = {
	A_OR_B: 'A-or-B',
	A_AND_B: 'A-and-B',
};

export const LOCK_STATUS = [
	'opened',
	'locked',
];

export const addEditSettings = {
	ADD_UBA_DEVICE: 'add.uba',
	EDIT_UBA_DEVICE: 'edit.uba',
	ADD_CELL: 'add.cell',
	EDIT_CELL: 'edit.cell',
};

export const statusCodes = {
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

export const isStatusInPending = (status) => [statusCodes.PENDING_SAVE, statusCodes.PENDING_RUNNING, statusCodes.PENDING_PAUSE, statusCodes.PENDING_STOP, statusCodes.PENDING_STANDBY].includes(status);

export const getKeyByValue = (obj, value) => Object.entries(obj).find(([key, val]) => val === value)?.[0];

export const isTestRunning = (status) => [statusCodes.RUNNING, statusCodes.PAUSED, statusCodes.PENDING_SAVE, statusCodes.PENDING_RUNNING, statusCodes.PENDING_PAUSE, statusCodes.PENDING_STOP, statusCodes.PENDING_STANDBY].includes(status);

export const isTestNeedToBeConfirmed = (status) => [statusCodes.STOPPED, statusCodes.ABORTED, statusCodes.FINISHED].includes(status);

export const category = {
	RUNNING: 'RUNNING',
	STANDBY: 'STANDBY',
};

export const DATE_RANGE  = [
	'lastWeek',
	'lastMonth',
	'last6Months',
	'lastYear',
]

export const unitVariants = {
	one: [
		{
			parameter: 'absoluteMa',
			label: 'mainPage.wizardTwo.ABSOLUTE_MA',
		},
		{
			parameter: 'absoluteA',
			label: 'mainPage.wizardTwo.ABSOLUTE_A',
		},
		{
			parameter: 'relative',
			label: 'mainPage.wizardTwo.RELATIVE',
		},
	],
	two: [
		{
			parameter: 'absoluteMa',
			label: 'mainPage.wizardTwo.ABSOLUTE_MA',
		},
		{
			parameter: 'absoluteA',
			label: 'mainPage.wizardTwo.ABSOLUTE_A',
		},
		{
			parameter: 'relative',
			label: 'mainPage.wizardTwo.RELATIVE',
		},
	],
	tree: [
		{
			parameter: 'absoluteMah',
			label: 'mainPage.wizardTwo.ABSOLUTE_MAH',
		},
		{
			parameter: 'absoluteAh',
			label: 'mainPage.wizardTwo.ABSOLUTE_AH',
		},
		{
			parameter: 'relative',
			label: 'mainPage.wizardTwo.RELATIVE',
		},
	],
	six: [
		{
			parameter: 'absoluteMa',
			label: 'mainPage.wizardTwo.ABSOLUTE_MA',
		},
		{
			parameter: 'absoluteA',
			label: 'mainPage.wizardTwo.ABSOLUTE_A',
		},
		{
			parameter: 'relative',
			label: 'mainPage.wizardTwo.RELATIVE',
		},
		{
			parameter: 'power',
			label: 'mainPage.wizardTwo.POWER',
		},
		{
			parameter: 'resistance',
			label: 'mainPage.wizardTwo.RESISTANCE',
		},
	],
	four: [
		{
			parameter: 'internal',
			label: 'mainPage.wizardTwo.INTERNAL',
		},
		{
			parameter: 'external',
			label: 'mainPage.wizardTwo.EXTERNAL',
		},
	],
	five: [
		{
			parameter: 'internal',
			label: 'mainPage.wizardTwo.INTERNAL',
		},
		{
			parameter: 'external1',
			label: 'mainPage.wizardTwo.EXTERNAL_1',
		},
		{
			parameter: 'external2',
			label: 'mainPage.wizardTwo.EXTERNAL_2',
		},
	],
};
