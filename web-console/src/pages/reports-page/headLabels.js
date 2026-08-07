import {getText,} from 'src/services/string-definitions';

export default [
	{
		label: ''
	},
	{
		id: 'batteryPN',
		label: getText('testEditor.BATTERY_P_N'),
		width: '10%',
	},
	{
		id: 'timestampStart',
		label: getText('common.DATE'),
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
		id: 'machineName',
		label: getText('common.LAB'),
	},
	{
		id: 'ubaSN',
		label: getText('reportsPage.UBA'),
	},
	{
		id: 'channel',
		label: getText('common.CHANNEL'),
	},
	{
		id: 'status',
		label: getText('common.STATUS'),
	},
	{
		label: getText('common.RUN_TIME'),
	},
	{
		label: getText('common.ACTION'),
	}
];
