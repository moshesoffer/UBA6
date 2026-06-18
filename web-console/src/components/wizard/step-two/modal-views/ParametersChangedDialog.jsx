import PropTypes from 'prop-types';

import Button from '@mui/material/Button';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogTitle from '@mui/material/DialogTitle';

import {getText,} from 'src/services/string-definitions';
import {actionOption,} from '../../utils';

import {paramChangeOption,} from 'src/components/wizard/step-two';

export default function ParametersChangedDialog(props) {

	const {open, onClose, doOption} = props;

	const handleOk = () => onClose(actionOption.SAVE_RUN);

	const handleNoSave = () => onClose(actionOption.RUN);

	const handleCancel = () => onClose(actionOption.CANCEL);

	const isRunOption = doOption === paramChangeOption.doRunTest;

	return (
		<Dialog open={open}>
			<DialogTitle>
				{isRunOption ? 'Save Changes and/or Run:' : 'Save TR:'}
			</DialogTitle>

			<DialogActions>
				<Button onClick={handleOk} autoFocus>
					{isRunOption
						? getText('common.SAVE_RUN')
						: getText('common.SAVE')}
				</Button>

				{isRunOption && (
					<Button onClick={handleNoSave}>
						{getText('common.RUN_NO_SAVE')}
					</Button>
				)}

				<Button onClick={handleCancel}>
					{getText('common.CANCEL')}
				</Button>
			</DialogActions>
		</Dialog>
	)
}

ParametersChangedDialog.propTypes = {
	onClose: PropTypes.func.isRequired,
	open: PropTypes.bool.isRequired,
};
