import React from 'react';

import SvgColor from 'src/components/SvgColor';
import {getText,} from 'src/services/string-definitions';
import {navigationPaths,} from 'src/constants/unsystematic';

const icon = name => <SvgColor src={`/assets/icons/navbar/${name}.svg`} sx={{width: 1, height: 1}}/>;

const navConfig = [
	{
		title: getText('pageHeaders.UBA_CONTROL_CENTER'),
		path: `/${navigationPaths.MAIN_PAGE}`,
		icon: icon('ic_home'),
	},
	{
		title: getText('pageHeaders.TEST_ROUTINES'),
		path: `/${navigationPaths.TEST_ROUTINES}`,
		icon: icon('ic_editor'),
	},
	{
		title: getText('pageHeaders.REPORTS'),
		path: `/${navigationPaths.REPORTS}`,
		icon: icon('ic_analytics'),
	},
	{
		title: getText('pageHeaders.SETTINGS'),
		path: `/${navigationPaths.SETTINGS}`,
		icon: icon('ic_settings'),
	},
	{
		title: getText('pageHeaders.USERS'),
		path: `/${navigationPaths.USERS}`,
		icon: icon('ic_user'),
	},
];

export default navConfig;
