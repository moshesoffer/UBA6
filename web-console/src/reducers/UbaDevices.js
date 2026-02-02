import {
	MAIN_PAGE_STATE,
	SET_UBA_DEVICES,
	SET_CURRENT_UBA,
	SET_SELECTED_DEVICES,
	UPDATE_CURRENT_UBA,
	SET_UBA_TOTAL,
} from 'src/constants/ActionTypes';
import {pageStateList,getSelectedCardsOrTableView} from 'src/constants/unsystematic';

export const initialUbaDevices = {
	state: getSelectedCardsOrTableView(),
	ubaDevices: [],
	currentUba: {},
	selectedDevices: [],
	ubaTotal: {},
};

export default function ubaDevicesReducer(state, action) {
	switch (action.type) {
		case MAIN_PAGE_STATE: {
			return {
				...state,
				state: action.payload,
			}
		}
		case SET_UBA_DEVICES: {
			return {
				...state,
				ubaDevices: action.payload,
			};
		}
		case SET_CURRENT_UBA: {
			return {
				...state,
				currentUba: action.payload,
			};
		}
		case SET_SELECTED_DEVICES: {
			return {
				...state,
				selectedDevices: action.payload,
			};
		}
		case UPDATE_CURRENT_UBA: {
			return {
				...state,
				currentUba: {
					...state.currentUba,
					[action.payload.dataKey]: action.payload.dataValue,
				},
			};
		}
		case SET_UBA_TOTAL: {
			return {
				...state,
				ubaTotal: action.payload,
			};
		}
		default:
			return state;
	}
};
