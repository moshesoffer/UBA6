import {setState as setStateDevices,} from 'src/actions/UbaDevices';
import {setState as setStateRoutines,} from 'src/actions/TestRoutines';
import {navigationPaths,} from 'src/constants/unsystematic';

export const changePageState = (state, pathname, ubaDevicesDispatch, testRoutinesDispatch) => {
	switch (pathname) {
		case `/${navigationPaths.MAIN_PAGE}`: {
			ubaDevicesDispatch(setStateDevices(state));
			break;
		}
		case `/${navigationPaths.TEST_ROUTINES}`: {
			testRoutinesDispatch(setStateRoutines(state));
			break;
		}
		default: {
			// eslint-disable-next-line no-console
			console.info('changePageState', 'Unknown pathname', pathname);
		}
	}
}
