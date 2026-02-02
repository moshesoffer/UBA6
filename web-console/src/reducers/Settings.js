import {
	SET_MACHINE_SETTINGS,
	SET_CELL_SETTINGS,
	SET_CURRENT_CELL,
	UPDATE_CURRENT_CELL,
} from 'src/constants/ActionTypes';

export const initialSettings = {
	machines: [],
	currentCell: {},
	cells: [],
};

export default function settingsReducer(state, action) {
	switch (action.type) {
		case SET_MACHINE_SETTINGS: {
			return {
				...state,
				machines: action.payload,
			};
		}
		case SET_CELL_SETTINGS: {
			return {
				...state,
				cells: action.payload,
			};
		}
		case SET_CURRENT_CELL: {
			return {
				...state,
				currentCell: action.payload,
			};
		}
		case UPDATE_CURRENT_CELL: {
			return {
				...state,
				currentCell: {
					...state.currentCell,
					[action.payload.dataKey]: action.payload.dataValue,
				},
			};
		}
		default:
			return state;
	}
}
