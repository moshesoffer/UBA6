import Box from '@mui/material/Box';

import {useTestRoutinesDispatch,} from 'src/store/TestRoutinesProvider';

import {handleOnDrop} from './utils';

export default function GapBox(props) {

	const {id,} = props;

	const testRoutinesDispatch = useTestRoutinesDispatch();

	return (
		<Box
			onDragOver={event => event.preventDefault()}
			onDrop={event => handleOnDrop(event, testRoutinesDispatch, id)}
			sx={{height: 20,}}
		/>
	);
}
