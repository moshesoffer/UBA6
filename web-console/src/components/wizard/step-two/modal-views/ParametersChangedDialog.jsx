import PropTypes from 'prop-types';

import Button from '@mui/material/Button';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogTitle from '@mui/material/DialogTitle';

import {getText,} from 'src/services/string-definitions';

export default function ParametersChangedDialog(props) {

	const {open, onClose,} = props;

	const handleCancel = () => onClose(false);

	const handleOk = () => onClose(true);

	return (
		<Dialog open={open}>
			<DialogTitle>
				Test was changed and changes will be saved
			</DialogTitle>

			<DialogActions>
				<Button onClick={handleCancel}>
					{getText('common.CANCEL')}
				</Button>

				<Button onClick={handleOk} autoFocus>
					{getText('common.OK')}
				</Button>
			</DialogActions>
		</Dialog>
	);
}

ParametersChangedDialog.propTypes = {
	onClose: PropTypes.func.isRequired,
	open: PropTypes.bool.isRequired,
};
