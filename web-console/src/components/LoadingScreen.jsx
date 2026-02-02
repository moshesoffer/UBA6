import React from 'react';
import PropTypes from 'prop-types';

import Box from '@mui/material/Box';
import LinearProgress from '@mui/material/LinearProgress';

import {useAuth,} from 'src/store/AuthProvider';

export default function LoadingScreen() {

	if (useAuth()?.isAjaxLoaderVisible) {
		return (
			<Box sx={{
				px: 5,
				width: 1,
				flexGrow: 1,
				minHeight: 1,
				display: 'flex',
				alignItems: 'center',
				justifyContent: 'center',
			}}>
				<LinearProgress color="inherit" sx={{width: 1, maxWidth: 360}}/>
			</Box>
		);
	}

	return null;
}

LoadingScreen.propTypes = {
	sx: PropTypes.object,
};
