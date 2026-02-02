import React, {forwardRef} from 'react';

import Box from '@mui/material/Box';
import Link from '@mui/material/Link';

import RouterLink from 'src/routes/RouterLink';

const Logo = ({disabledLink = false, sx, ...other}, ref) => {

	const logo = (
		<Box
			component="img"
			src="/assets/images/amicell-logo.png"
			sx={{
				width: 70,
				...sx,
			}}
			{...other}
		>
		</Box>
	);

	if (disabledLink) {
		return logo;
	}

	return (
		<Link component={RouterLink} href="/" sx={{display: 'contents'}}>
			{logo}
		</Link>
	);
};

export default forwardRef(Logo);
