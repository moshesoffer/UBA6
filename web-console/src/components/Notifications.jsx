import React from 'react';
import Alert from '@mui/material/Alert';
import Snackbar from '@mui/material/Snackbar';
import {useAuth, useAuthDispatch,} from 'src/store/AuthProvider';
import {setNotification,} from 'src/actions/Auth';
import {validateString,} from 'src/utils/validators';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';

function Notifications() {

	const anchorOrigin = {
		vertical: 'top',
		horizontal: 'right'
	};
	const {message, severity,} = useAuth()?.notification || {};
	const authDispatch = useAuthDispatch();

	if (validateString(message)) {
		return (
			<Snackbar
				{...{anchorOrigin}}
				key={message}
				transitionDuration={2000}
				open={validateString(message)}
				onClose={() => authDispatch(setNotification({message: '',}))}
			>
				< Alert variant="filled" {...{severity}} action={
						<IconButton
							size="small"
							aria-label="close"
							color="inherit"
							onClick={() => authDispatch(setNotification({ message: '' }))}
						>
							<CloseIcon fontSize="small" />
						</IconButton>
						}>
					{message}
				</Alert>
			</Snackbar>
		);
	}

	return null;
}

export default Notifications;
