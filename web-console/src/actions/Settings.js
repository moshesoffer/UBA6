import {
	SET_MACHINE_SETTINGS,
	SET_CELL_SETTINGS,
	SET_CURRENT_CELL,
	UPDATE_CURRENT_CELL,
} from 'src/constants/ActionTypes';

export const setMachineSettings = payload => ({
	type: SET_MACHINE_SETTINGS,
	payload,
});

export const setCellSettings = payload => ({
	type: SET_CELL_SETTINGS,
	payload,
});

export const setCurrentCell = payload => ({
	type: SET_CURRENT_CELL,
	payload,
});

export const updateCurrentCell = payload => ({
	type: UPDATE_CURRENT_CELL,
	payload,
});
