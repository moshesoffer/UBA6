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

const STANDBY= 0x0001;
const STOPPED= 0x0002;
const ABORTED= 0x0004;
const FINISHED= 0x0008;//8
const SAVED= 0x0010;
const RUNNING= 0x0020;//32
const PAUSED= 0x0040;//64
const PENDING= 0x0100;

export const statusCodes = {
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

export const isTestRunning =  (runningStatus) => (runningStatus & statusCodes.IS_TEST_RUNNING) !== 0;
export const isStatusInPending =  (runningStatus) => (runningStatus & statusCodes.PENDING) !== 0;


export const errorCodes = {
  0x00000001: 'NOT_AVAILABLE',
  0x00000002: 'I2C_PERIPHERAL',
  0x00000004: 'BAT_HIGH_VOLTAGE',
  0x00000008: 'BAT_HIGH_CURRENT',
  0x00000010: 'BAT_HIGH_TEMP',
  0x00000020: 'AMB_HIGH_TEMP',
  0x00000040: 'GEN_HIGH_VOLTAGE',
  0x00000080: 'BIST_FAILED',
  0x00000100: 'BUSY',
  0x00000200: 'REVERSE_POLARITY',
  0x00000400: 'OVER_VOLTAGE',
  0x00000800: 'DISCHARGE_MOSFET_SHORT',
  0x00001000: 'CHARGE_MOSFET_SHORT',
  0x00002000: 'LOW_INPUT_VOLTAGE',
  0x00004000: 'HIGH_INPUT_VOLTAGE',
  0x00008000: 'VGEN_EXPECTED_MAX_VOLTAGE',
  0x00010000: 'VGEN_FAILED',
  0x00020000: 'VGEN_LIMITES',
  0x00040000: 'OVERCURRENT',
  0x00080000: 'NO_CALIBRATION',
  0x00100000: 'BAT_NOT_CONNECTED',
  0x00200000: 'BAT_TEMP_SENSOR_NC',
  0x00400000: 'INTERNAL_TEMP_SENSOR_NC',
  0x00800000: 'EXTERNAL_LINE_ERROR',
  0x01000000: 'INTERNAL_LINE_ERROR',
  0x02000000: 'CHANNEL_EMPTY',
  0x04000000: 'CHANNEL_MULTI_LINE_VOLTAGE_MISMATCH',
  0x08000000: 'CHANNEL_MULTI_LINE_CURRENT_MISMATCH',
  0x10000000: 'SD_CARD',
  0x20000000: 'USER_ABORT',
  0x40000000: 'CHANNEL_ERROR',
};

export const getErrorMessage = (errorCode) => {
	if (errorCode === 0) return 'No Error';

	const messages = [];
	for (const [flag, message] of Object.entries(errorCodes)) {
		if (errorCode & parseInt(flag,10)) {
			messages.push(message);
		}
	}

	return messages.length > 0
		? messages.join(', ')
		: 'Unrecognized error code';
};


export const getKeyByValue = (obj, value) => Object.entries(obj).find(([key, val]) => val === value)?.[0];

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
