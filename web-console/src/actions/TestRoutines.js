import {
	TEST_ROUTINES_STATE,
	SET_TEST_ROUTINES,
	SET_GRAPH_DATA,
	SET_TEST_TYPES,
	ADD_TEST_TYPE,
	CHANGE_TEST_TYPE,
	FOLD_TEST_TYPE,
	DELETE_TEST_TYPE,
	TEST_TYPE_DATA,
	SET_TEST_DATA,
	ALL_TEST_DATA,
	START_SAVE_ACTION,
	EXISTING_TEST_PARAMETERS,
	TEST_EDIT_PARAMETERS,
} from 'src/constants/ActionTypes';

export const setState = payload => ({
	type: TEST_ROUTINES_STATE,
	payload,
});

export const setTestRoutines = payload => ({
	type: SET_TEST_ROUTINES,
	payload,
});

export const setGraphData = payload => ({
	type: SET_GRAPH_DATA,
	payload,
});

export const setPlan = payload => ({
	type: SET_TEST_TYPES,
	payload,
});

export const addTestType = payload => ({
	type: ADD_TEST_TYPE,
	payload,
});

export const changeTestType = payload => ({
	type: CHANGE_TEST_TYPE,
	payload,
});

export const foldTestType = payload => ({
	type: FOLD_TEST_TYPE,
	payload,
});

export const deleteTestType = payload => ({
	type: DELETE_TEST_TYPE,
	payload,
});

export const testTypeData = payload => ({
	type: TEST_TYPE_DATA,
	payload,
});

export const setTestData = payload => ({
	type: SET_TEST_DATA,
	payload,
});

export const allTestData = payload => ({
	type: ALL_TEST_DATA,
	payload,
});

export const existingTestParameters = () => ({
	type: EXISTING_TEST_PARAMETERS,
});

export const testEditParameters = () => ({
	type: TEST_EDIT_PARAMETERS,
});

export const startSaveAction = () => ({
	type: START_SAVE_ACTION,
});
