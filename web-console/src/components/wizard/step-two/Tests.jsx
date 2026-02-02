import Box from '@mui/material/Box';

import {useTestRoutines,} from 'src/store/TestRoutinesProvider';
import {testTypeNames,} from 'src/constants/unsystematic';
import Typography from '@mui/material/Typography';
import Delay from './test-type-components/Delay';
import Charge from './test-type-components/charge';
import Discharge from './test-type-components/discharge';
import Loop from './test-type-components/Loop';
import { getText, } from 'src/services/string-definitions';
import GapBox from './GapBox';

export default function Tests() {

	const {plan,} = useTestRoutines();

	const getTestTypeComponent = (testType, key) => {
		switch (testType.type) {
			case testTypeNames.CHARGE:
				return <Charge key={key} {...testType}/>;
			case testTypeNames.DISCHARGE:
				return <Discharge key={key} {...testType}/>;
			case testTypeNames.DELAY:
				return <Delay key={key} {...testType}/>;
			case testTypeNames.LOOP:
				return <Loop key={key} {...testType}/>;
			default:
				return (
					<Box key={key}>
						Unknown test type
					</Box>
				);
		}
	}

	return (
		<Box sx={{
			width: 'calc(100% - 50px)',
			height: 'calc( 100vh - 200px)',
			overflowY: 'auto',
		}}>
			<Typography variant="h5" color="#637381" >
				{getText('mainPage.wizardTwo.TEST_PLAN')}
			</Typography>

			<GapBox id={-1}/>

			{plan.map((testType, key) => getTestTypeComponent(testType, key))}
		</Box>
	);
}
