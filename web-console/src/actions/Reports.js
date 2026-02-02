import {
	SET_REPORTS,
	SET_CURRENT_REPORT,
	UPDATE_CURRENT_REPORT,
	SET_TESTS_GRAPH,
} from 'src/constants/ActionTypes';

export const setReports = payload => ({
	type: SET_REPORTS,
	payload,
});

export const setCurrentReport = payload => ({
	type: SET_CURRENT_REPORT,
	payload,
});

export const updateCurrentReport = payload => ({
	type: UPDATE_CURRENT_REPORT,
	payload,
});

export const setTestsGraph = payload => ({
	type: SET_TESTS_GRAPH,
	payload,
});
