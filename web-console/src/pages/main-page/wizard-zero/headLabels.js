import {getText,} from 'src/services/string-definitions';

export default [
	{
		label: '',
	},
	{
		id: 'testName',
		label: getText('common.TEST_NAME'),
	},
	{
		id: 'isLocked',
		label: getText('mainPage.wizardZero.TEST_LOCK'),
	},
	{
		id: 'customer',
		label: getText('mainPage.wizardOne.CUSTOMER'),
	},
	{
		label: getText('testEditor.BATTERY_P_N'),
	},
	{
		label: getText('reportsPage.BATTERY_S_N'),
	},
	{
		label: getText('testEditor.CELL_P_N'),
	},
	{
		id: 'noCellSerial',
		label: getText('testEditor.NO_CELLS_IN_SERIAL'),
	},
	{
		id: 'noCellParallel',
		label: getText('testEditor.NO_CELLS_IN_PARALLEL'),
	},
];
