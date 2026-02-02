import React, {useEffect,} from 'react';
import {useLocation} from 'react-router-dom';
import PropTypes from 'prop-types';

import Box from '@mui/material/Box';
import Stack from '@mui/material/Stack';
import Drawer from '@mui/material/Drawer';

import useResponsive from 'src/hooks/useResponsive';
import Logo from 'src/components/logo';

import {NAV} from './configLayout';
import navConfig from './navConfig';
import NavItem from './NavItem';

export default function Nav({openNav, onCloseNav}) {

	const {pathname} = useLocation();
	const upLg = useResponsive('up', 'lg');

	useEffect(() => {
		if (openNav) {
			onCloseNav();
		}
		// eslint-disable-next-line react-hooks/exhaustive-deps
	}, [pathname]);

	const renderMenu = (
		<Stack component="nav" spacing={0.5} sx={{px: 2}}>
			{navConfig.map((item) => (
				<NavItem key={item.title} item={item}/>
			))}
		</Stack>
	);

	const renderContent = (
		<Box sx={{
			height: 1,
			'& .simplebar-content': {
				height: 1,
				display: 'flex',
				flexDirection: 'column',
			},
		}}>
			<Logo disabledLink={true} sx={{mt: 3, ml: 2, mb: 3,}}/>

			{renderMenu}

			<Box sx={{flexGrow: 1}}/>
		</Box>
	);

	return (
		<Box sx={{
			flexShrink: {lg: 0},
			width: {lg: NAV.WIDTH},
		}}>
			{upLg ? (
				<Box sx={{
					height: 1,
					position: 'fixed',
					width: NAV.WIDTH,
					borderRight: (theme) => `dashed 1px ${theme.palette.divider}`,
				}}>
					{renderContent}
				</Box>
			) : (
				<Drawer
					open={openNav}
					onClose={onCloseNav}
					PaperProps={{
						sx: {
							width: NAV.WIDTH,
						},
					}}
				>
					{renderContent}
				</Drawer>
			)}
		</Box>
	);
}

Nav.propTypes = {
	openNav: PropTypes.bool,
	onCloseNav: PropTypes.func,
};
