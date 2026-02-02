import {getText,} from 'src/services/string-definitions';

export default [
	{
		id: 'testName',
		label: getText('common.TEST_NAME'),
	},
	{
		id: 'batteryPN',
		label: getText('testEditor.BATTERY_P_N'),
	},
	{
		id: 'isLocked',
		label: getText('mainPage.wizardZero.TEST_LOCK'),
	},
	{
		id: 'cellPN',
		label: getText('testEditor.CELL_P_N'),
	},
	{
		label: getText('testEditor.NO_CELLS_IN_SERIAL'),
	},
	{
		label: getText('testEditor.NO_CELLS_IN_PARALLEL'),
	},
	{
		label: getText('common.ACTION'),
	},
];
