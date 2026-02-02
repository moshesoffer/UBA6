import {SET_AUTH_CONDITION, SET_AJAX_LOADER, SET_NOTIFICATION, SET_MODAL,} from 'src/constants/ActionTypes';

export const setAuthCondition = payload => ({
	type: SET_AUTH_CONDITION,
	payload,
});

export const setAjaxLoader = payload => ({
	type: SET_AJAX_LOADER,
	payload,
});

export const setNotification = payload => ({
	type: SET_NOTIFICATION,
	payload,
});

export const setModal = payload => ({
	type: SET_MODAL,
	payload
});
