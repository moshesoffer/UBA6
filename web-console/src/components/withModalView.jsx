import React from 'react';
import PropTypes from 'prop-types';

import Modal from '@mui/material/Modal';
import Box from '@mui/material/Box';
import IconButton from '@mui/material/IconButton';
import HighlightOffIcon from '@mui/icons-material/HighlightOff';

import {setModal,} from 'src/actions/Auth';
import {validateInteger, validateString,} from 'src/utils/validators';
import {useAuth, useAuthDispatch,} from 'src/store/AuthProvider';

const style = {
	position: 'absolute',
	top: '50%',
	left: '50%',
	transform: 'translate(-50%, -50%)',
	maxWidth: '70%',
	maxHeight: '80%',
	bgcolor: 'background.paper',
	border: '2px solid #000',
	borderRadius: 2,
	boxShadow: 24,
	p: 4,
	overflow: 'auto',
};

function withModalView(ActionComponent) {
	function ModalView(props) {

		const {actionName, maxWidth, maxHeight, p, minWidth, minHeight,} = props;
		const openedModalType = useAuth()?.openedModalType;
		const authDispatch = useAuthDispatch();

		const updatedStyle = {
			...style,
			maxWidth: validateInteger(maxWidth) ? `${maxWidth}%` : style.maxWidth,
			maxHeight: validateInteger(maxHeight) ? `${maxHeight}%` : style.maxHeight,
			p: validateInteger(p) ? p : style.p,
		}
		if(minWidth && validateInteger(minWidth)) {
			updatedStyle.minWidth = `${minWidth}%`
		}
		if(minHeight && validateInteger(minHeight)) {
			updatedStyle.minHeight = `${minHeight}%`
		}

		const handleCloseModal = event => {
			if (event?.currentTarget?.tagName?.toLowerCase() === 'button' || event?.currentTarget === event?.target) {
				authDispatch(setModal(''));
			}
		};

		return (
			<Modal open={validateString(openedModalType) && actionName.includes(openedModalType)} onClose={handleCloseModal}>
				<Box sx={updatedStyle}>
					<IconButton
						size="small"
						onClick={handleCloseModal}
						sx={{
							position: 'absolute',
							top: 0,
							right: 0,
						}}
					>
						<HighlightOffIcon fontSize="large"/>
					</IconButton>

					<ActionComponent {...props}/>
				</Box>
			</Modal>
		);
	}

	ModalView.propTypes = {
		actionName: PropTypes.array,
	};

	return ModalView;
}

export default withModalView;
