import {
	MAIN_PAGE_STATE,
	SET_UBA_DEVICES,
	SET_CURRENT_UBA,
	SET_SELECTED_DEVICES,
	UPDATE_CURRENT_UBA,
	SET_UBA_TOTAL,
} from 'src/constants/ActionTypes';

export const setState = payload => ({
	type: MAIN_PAGE_STATE,
	payload,
});

export const setUbaDevices = payload => ({
	type: SET_UBA_DEVICES,
	payload,
});

export const setCurrentUba = payload => ({
	type: SET_CURRENT_UBA,
	payload,
});

export const setSelectedDevices = payload => ({
	type: SET_SELECTED_DEVICES,
	payload,
});

export const updateCurrentUba = payload => ({
	type: UPDATE_CURRENT_UBA,
	payload,
});

export const setUbaTotal = payload => ({
	type: SET_UBA_TOTAL,
	payload,
});
