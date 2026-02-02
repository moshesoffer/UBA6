import {useTestRoutines,} from 'src/store/TestRoutinesProvider';
import {pageStateList,} from 'src/constants/unsystematic';

import WizardOne from 'src/components/wizard/step-one';
import WizardTwo from 'src/components/wizard/step-two';

import TableView from './table-view';

export default function TestRoutines() {

	const {state,} = useTestRoutines();

	switch (state) {
		case pageStateList.TABLE_VIEW:  {
			return <TableView/>;
		}
		case pageStateList.CARDS_VIEW: {//there isnt really cards view for testRoutines, this is just a patch
			return <TableView/>;
		}
		case pageStateList.WIZARD_ONE: {
			return <WizardOne/>;
		}
		case pageStateList.WIZARD_TWO: {
			return <WizardTwo/>;
		}
		default: {
			return <TableView/>;
		}
	}
}
