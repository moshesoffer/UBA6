import React from 'react';
import {useLocation} from 'react-router-dom';
import PropTypes from 'prop-types';

import Box from '@mui/material/Box';
import {alpha} from '@mui/material/styles';
import ListItemButton from '@mui/material/ListItemButton';
import Tooltip from '@mui/material/Tooltip';
import Typography from '@mui/material/Typography';

import RouterLink from 'src/routes/RouterLink';
import {useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import {useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';
import {pageStateList, navigationPaths,getSelectedCardsOrTableView} from 'src/constants/unsystematic';
import {setState as ubaDevicesState,} from 'src/actions/UbaDevices';
import {setState as testRoutinesState,} from 'src/actions/TestRoutines';
import {resetTestParameters,} from 'src/utils/helper';

export default function NavItem({item}) {

	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	const {pathname} = useLocation();
	const active = item.path === pathname;

	const tooltipTitle = (
		<Typography fontSize={24}>
			{item.title}
		</Typography>
	);

	const menuClickHook = () => {
		switch (item.path) {
			case `/${navigationPaths.MAIN_PAGE}`: {
				resetTestParameters(testRoutinesDispatch);
				ubaDevicesDispatch(ubaDevicesState(getSelectedCardsOrTableView()));
				break;
			}
			case `/${navigationPaths.TEST_ROUTINES}`: {
				resetTestParameters(testRoutinesDispatch);
				testRoutinesDispatch(testRoutinesState(pageStateList.TABLE_VIEW));
				break;
			}
			default: {
			}
		}
	}

	return (
		<Tooltip title={tooltipTitle} arrow placement="right">
			<ListItemButton
				component={RouterLink}
				href={item.path}
				onClick={menuClickHook}
				sx={{
					minHeight: 44,
					borderRadius: 0.75,
					typography: 'body2',
					color: 'text.secondary',
					textTransform: 'capitalize',
					fontWeight: 'fontWeightMedium',
					...(active && {
						color: 'primary.main',
						fontWeight: 'fontWeightSemiBold',
						bgcolor: theme => alpha(theme.palette.primary.main, 0.08),
						'&:hover': {
							bgcolor: theme => alpha(theme.palette.primary.main, 0.16),
						},
					}),
				}}
			>
				<Box component="span" sx={{width: 24, height: 24, mr: 2}}>
					{item.icon}
				</Box>
			</ListItemButton>
		</Tooltip>
	);
}

NavItem.propTypes = {
	item: PropTypes.object,
};
