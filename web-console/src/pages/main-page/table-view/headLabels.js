import {getText,} from 'src/services/string-definitions';

export default [
	{
		label: ''
	},
	{
		id: 'name',
		label: getText('common.NAME'),
	},
	{
		label: getText('common.CH_'),
	},
	{
		id: 'machineName',
		label: getText('common.LAB'),
	},
	{
		id: 'batteryPN',
		label: getText('testEditor.BATTERY_P_N'),
	},
	{
		id: 'batterySN',
		label: getText('reportsPage.BATTERY_S_N'),
	},
	{
		id: 'testName',
		label: getText('common.TEST_NAME'),
	},
	{
		id: 'status',
		label: getText('common.STATUS'),
	},
	{
		id: 'runtime',
		label: getText('common.RUN_TIME'),
	},
	{
		label: getText('mainPage.READING'),
	},
	{
		label: getText('common.ACTION'),
	}
];
