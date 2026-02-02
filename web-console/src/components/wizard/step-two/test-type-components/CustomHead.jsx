import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import ButtonGroup from '@mui/material/ButtonGroup';
import IconButton from "@mui/material/IconButton";
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ExpandLessIcon from '@mui/icons-material/ExpandLess';
import CancelIcon from '@mui/icons-material/Cancel';

import {getText,} from 'src/services/string-definitions';
import {useUbaDevices,} from 'src/store/UbaDevicesProvider';
import {useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {foldTestType, deleteTestType,} from 'src/actions/TestRoutines';
import {validateObject,} from 'src/utils/validators';

const customStyles = {
	width: 1,
	p: 1,
	border: 'solid 2px #d0d8e5',
	borderRadius: '16px',
};

export default function CustomHead(props) {

	const {id, isCollapsed, title,} = props;
	const {currentUba,} = useUbaDevices();
	const {testData,} = useTestRoutines();
	const testRoutinesDispatch = useTestRoutinesDispatch();

	let sx = [];
	if (isCollapsed) {
		sx = customStyles;
	}

	return (
		<Stack
			direction="row"
			alignItems="center"
			justifyContent="space-between"
			sx={sx}
		>
			<Typography
				draggable
				variant="h6"
				onDragStart={event => event.dataTransfer.setData('text/plain', JSON.stringify(props))}
				sx={{pl: 2,}}
			>
				{`${id + 1} - ${getText(title)}`}
			</Typography>

			<ButtonGroup variant="contained">
				<IconButton onClick={() => testRoutinesDispatch(foldTestType(id))} disabled={!!testData?.isLocked && validateObject(currentUba, true)} sx={{fontSize: '35px', p: 0,}}>
					{isCollapsed ?
						<ExpandMoreIcon fontSize="inherit" color="primary"/> :
						<ExpandLessIcon fontSize="inherit" color="primary"/>
					}
				</IconButton>

				<IconButton onClick={() => testRoutinesDispatch(deleteTestType(id))} disabled={!!testData?.isLocked && validateObject(currentUba, true)} sx={{fontSize: '35px', p: 0,}}>
					<CancelIcon fontSize="inherit" color="warning"/>
				</IconButton>
			</ButtonGroup>
		</Stack>
	);
}
