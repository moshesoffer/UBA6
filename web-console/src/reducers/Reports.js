import {
	SET_REPORTS,
	SET_CURRENT_REPORT,
	UPDATE_CURRENT_REPORT,
	SET_TESTS_GRAPH,
} from 'src/constants/ActionTypes';

export const initialReports = {
	reports: {
		rows: [],
		count: 0,
	},
	currentReport: {},
	testsGraph: {},
};

export default function TestRoutinesReducer(state, action) {
	switch (action.type) {
		case SET_REPORTS: {
			return {
				...state,
				reports: action.payload,
			};
		}
		case SET_CURRENT_REPORT: {
			return {
				...state,
				currentReport: action.payload,
			};
		}
		case UPDATE_CURRENT_REPORT: {
			return {
				...state,
				currentReport: {
					...state.currentReport,
					[action.payload.dataKey]: action.payload.dataValue,
				},
			};
		}
		case SET_TESTS_GRAPH: {
			return {
				...state,
				testsGraph: action.payload,
			};
		}
		default:
			return state;
	}
}
