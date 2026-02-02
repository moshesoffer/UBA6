import React from 'react';
import Alert from '@mui/material/Alert';
import Snackbar from '@mui/material/Snackbar';
import {useAuth, useAuthDispatch,} from 'src/store/AuthProvider';
import {setNotification, setSecondaryNotification} from 'src/actions/Auth';
import {validateString,} from 'src/utils/validators';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';

function Notifications() {

	const anchorOrigin = {
		vertical: 'top',
		horizontal: 'right'
	};
	const {message, severity,} = useAuth()?.notification || {};
	const {message: message2, severity: severity2,} = useAuth()?.secondaryNotification || {};
	const authDispatch = useAuthDispatch();

	if (validateString(message) || validateString(message2)) {
		return (
			<>
				<Snackbar
					{...{anchorOrigin}}
					key={message}
					transitionDuration={2000}
					open={validateString(message)}
				>
					<Alert variant="filled" {...{severity}} action={
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
				<Snackbar
					{...{anchorOrigin}}
					style={{ top: `90px` }}
					key={message2}
					transitionDuration={2000}
					open={validateString(message2)}
				>
					<Alert variant="filled" {...{severity: severity2}} action={
							<IconButton
								size="small"
								aria-label="close"
								color="inherit"
								onClick={() => authDispatch(setSecondaryNotification({ message: '' }))}
							>
								<CloseIcon fontSize="small" />
							</IconButton>
							}>
						{message2}
					</Alert>
				</Snackbar>
			</>
		);
	}

	return null;
}

export default Notifications;
