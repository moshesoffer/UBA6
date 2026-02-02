import React from 'react';
import PropTypes from 'prop-types';

import Box from '@mui/material/Box';
import Stack from '@mui/material/Stack';
import AppBar from '@mui/material/AppBar';
import Toolbar from '@mui/material/Toolbar';
import {useTheme} from '@mui/material/styles';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography';

import useResponsive from 'src/hooks/useResponsive';
import {bgBlur} from 'src/theme/css';
import Iconify from 'src/components/Iconify';
import { useLocation } from 'react-router-dom';
import {NAV, HEADER} from './configLayout';
import AccountPopover from './common/AccountPopover';
import {getText,} from 'src/services/string-definitions';

export default function Header({onOpenNav}) {

	const location = useLocation();
	const theme = useTheme();
	const lgUp = useResponsive('up', 'lg');

	const pageTitles = {
		'/': 'pageHeaders.UBA_CONTROL_CENTER',
		'/test-routines': 'pageHeaders.TEST_ROUTINES',
		'/reports': 'pageHeaders.REPORTS',
		'/settings': 'pageHeaders.SETTINGS',
		'/users': 'pageHeaders.USERS',
	};

	const currentPage = pageTitles[location.pathname] || 'pageHeaders.UBA_CONTROL_CENTER';

	const renderContent = (
		<>
			{!lgUp && (
				<IconButton onClick={onOpenNav} sx={{mr: 1}}>
					<Iconify icon="eva:menu-2-fill"/>
				</IconButton>
			)}
			<Typography variant="h4" sx={{color: theme.palette.warning.dark,}}>
				{getText(currentPage)}
			</Typography>

			<Box sx={{flexGrow: 1}}/>

			<Stack direction="row" alignItems="center" spacing={1}>
				<AccountPopover/>
			</Stack>
		</>
	);

	return (
		<AppBar sx={{
			boxShadow: 'none',
			height: HEADER.H_MOBILE,
			zIndex: theme.zIndex.appBar + 1,
			...bgBlur({
				color: theme.palette.background.default,
			}),
			transition: theme.transitions.create(['height'], {
				duration: theme.transitions.duration.shorter,
			}),
			...(lgUp && {
				width: `calc(100% - ${NAV.WIDTH + 1}px)`,
				height: HEADER.H_DESKTOP,
			}),
		}}>
			<Toolbar sx={{
				height: 1,
				px: {lg: 5},
			}}>
				{renderContent}
			</Toolbar>
		</AppBar>
	);
}

Header.propTypes = {
	onOpenNav: PropTypes.func,
};
