import {useEffect,} from 'react';
import GraphDetails from './modal-views/GraphDetails';
import {useAuthDispatch,} from 'src/store/AuthProvider';
import {useUbaDevices, useUbaDevicesDispatch,} from 'src/store/UbaDevicesProvider';
import {getUbaDevices,} from 'src/action-creators/UbaDevices';
import {pageStateList,} from 'src/constants/unsystematic';
import WizardTwo from 'src/components/wizard/step-two';
import WizardOne from 'src/components/wizard/step-one';

import TableView from './table-view';
import CardView from './card-view';
import WizardZero from './wizard-zero';
import RunBatchTest from './run-batch-test';

export default function MainPage() {

	const {state,} = useUbaDevices();

	const authDispatch = useAuthDispatch();
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	useEffect(() => {
		//console.log('MainPage useEffect');
		getUbaDevices(authDispatch, ubaDevicesDispatch);
		//return () => console.log('MainPage unmount useEffect');
	}, []);

	switch (state) {
		case pageStateList.TABLE_VIEW: {
			return <><TableView/><GraphDetails maxHeight={95} maxWidth={95} minWidth={95} minHeight={95} actionName={['graph.details',]}/></>;
		}
		case pageStateList.CARDS_VIEW: {
			return <><CardView/><GraphDetails maxHeight={95} maxWidth={95} minWidth={95} minHeight={95} actionName={['graph.details',]}/></>;
		}
		case pageStateList.WIZARD_ZERO: {
			return <WizardZero/>;
		}
		case pageStateList.WIZARD_ONE: {
			return <WizardOne/>;
		}
		case pageStateList.WIZARD_TWO: {
			return <WizardTwo/>;
		}
		case pageStateList.RUN_BATCH_TEST: {
			return <RunBatchTest/>;
		}
		default: {
			console.error('Unknown state:', state);
			return <TableView/>;
		}
	}
}
