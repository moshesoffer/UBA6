import {SET_AUTH_CONDITION, SET_AJAX_LOADER, SET_NOTIFICATION, SET_MODAL,} from 'src/constants/ActionTypes';
import {notificationSeverity,} from 'src/constants/unsystematic';

const getSeverity = severity => {
	if (Object.values(notificationSeverity).includes(severity)) {
		return severity;
	}

	return notificationSeverity.ERROR;
}

export const initialAuth = {
	displayName: '',
	isAjaxLoaderVisible: false,
	notification: {
		severity: notificationSeverity.ERROR,
		message: '',
	},
	openedModalType: ''
};

export default function authReducer(state, action) {
	switch (action.type) {
		case SET_AUTH_CONDITION: {
			return {
				...state,
				displayName: action.payload,
			};
		}
		case SET_AJAX_LOADER: {
			return {
				...state,
				isAjaxLoaderVisible: action.payload,
			}
		}
		case SET_NOTIFICATION: {
			return {
				...state,
				notification: {
					severity: getSeverity(action.payload?.severity),
					message: action.payload.message,
				}
			};
		}
		case SET_MODAL: {
			return {
				...state,
				openedModalType: action.payload,
			}
		}
		default:
			return state;
	}
}
