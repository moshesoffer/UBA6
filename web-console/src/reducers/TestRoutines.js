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
import {UBA_CHANNEL_LIST, pageStateList,} from 'src/constants/unsystematic';
import {getUnit,} from 'src/utils/helper';

const excludedTestData = ['maxPerBattery', 'ratedBatteryCapacity',];

export const initialTestRoutines = {
	saveIsRun: 0,
	existingTest: {
		isTrue: false,
		isChanged: false,
	},
	state: pageStateList.TABLE_VIEW,
	testRoutines: [],
	plan: [],
	testData: {
		id: null,
		testName: '',
		isLocked: 0,
		batteryPN: '',
		batterySN: '',
		cellPN: '',
		noCellSerial: '',
		noCellParallel: '',
		maxPerBattery: null,
		maxVoltage: null,
		ratedBatteryCapacity: null,
		channel: UBA_CHANNEL_LIST.A_AND_B,
		notes: '',
		customer: '',
		workOrderNumber: '',
		approvedBy: '',
		conductedBy: '',
		cellSupplier: '',
		cellBatch: '',
	},
	instantTestResults: [],//Deprecated
	graphData: [],
};

const setPlan = (plan, testType) => {
	const chargeLimitUnit = getUnit('tree');
	const dischargeLimitUnit = getUnit('tree');
	const chargeCurrentUnit = getUnit('two');
	const dischargeCurrentUnit = getUnit('six');
	const cutOffCurrentUnit = getUnit('one');

	return [
		...plan,
		{
			id: plan.length,
			type: testType,
			isCollapsed: false,
			source: getUnit('four'),
			isMinTemp: true,
			minTemp: null,
			isMaxTemp: true,
			maxTemp: null,
			isMaxTime: false,
			maxTime: '00:00:00',
			delayTime: null,
			isChargeLimit: false,
			chargeLimit: `:${chargeLimitUnit}`,
			isDischargeLimit: false,
			dischargeLimit: `:${dischargeLimitUnit}`,
			chargeCurrent: `:${chargeCurrentUnit}`,
			dischargeCurrent: `:${dischargeCurrentUnit}`,
			cRate: null,
			isCutOffCurrent: false,
			cutOffCurrent: `:${cutOffCurrentUnit}`,
			isCutOffVoltage: false,
			cutOffVoltage: null,
			chargePerCell: null,
			waitTemp: null,
			goToStep: null,
			repeatStep: null,
		}
	];
};

const changePlan = (plan, data) => {
	let newPlan = plan.map(testType => {
		if (testType.id === data.previousId) {
			return {
				...testType,
				id: data.newId,
			};
		}

		return testType;
	});
	newPlan.sort((aElement, bElement) => aElement.id - bElement.id);
	newPlan = newPlan.map((testType, index) => ({
		...testType,
		id: index,
	}));

	return newPlan;
};

const foldPlan = (plan, testTypeId) => plan.map(testType => {
	if (testType.id === testTypeId) {
		return {
			...testType,
			isCollapsed: !testType.isCollapsed,
		};
	}

	return testType;
});

const deletePlan = (plan, testTypeId) => {
	let newPlan = plan.filter(testType => testType.id !== testTypeId);
	newPlan = newPlan.map((testType, index) => ({
		...testType,
		id: index,
	}));

	return newPlan;
}

const testTypeData = (plan, testTypeId, dataKey, dataValue) => plan.map(testType => {
	if (testType.id === testTypeId) {
		return {
			...testType,
			[dataKey]: dataValue,
		};
	}

	return testType;
});

export default function TestRoutinesReducer(state, action) {
	switch (action.type) {
		case START_SAVE_ACTION: {
			return {
				...state,
				saveIsRun: state.saveIsRun + 1,
			}
		}
		case TEST_ROUTINES_STATE: {
			return {
				...state,
				state: action.payload,
			}
		}
		case SET_TEST_ROUTINES: {
			return {
				...state,
				testRoutines: action.payload,
			};
		}
		case SET_GRAPH_DATA: {
			return {
				...state,
				graphData: action.payload,
			};
		}
		case SET_TEST_TYPES: {
			return {
				...state,
				plan: action.payload,
			};
		}
		case ADD_TEST_TYPE: {
			return {
				...state,
				plan: setPlan(state.plan, action.payload),
			};
		}
		case CHANGE_TEST_TYPE: {
			return {
				...state,
				plan: changePlan(state.plan, action.payload),
			};
		}
		case FOLD_TEST_TYPE: {
			return {
				...state,
				plan: foldPlan(state.plan, action.payload),
			};
		}
		case DELETE_TEST_TYPE: {
			return {
				...state,
				plan: deletePlan(state.plan, action.payload),
			};
		}
		case TEST_TYPE_DATA: {
			const existingTest = {
				...state.existingTest,
			};
			if (!action.payload.isExclude) {
				existingTest.isChanged = true;
			}

			return {
				...state,
				plan: testTypeData(state.plan, action.payload.id, action.payload.dataKey, action.payload.dataValue),
				existingTest,
			};
		}
		case SET_TEST_DATA: {
			const existingTest = {
				...state.existingTest,
			};
			if (!excludedTestData.includes(action.payload.dataKey)) {
				existingTest.isChanged = true;
			}

			return {
				...state,
				testData: {
					...state.testData,
					[action.payload.dataKey]: action.payload.dataValue,
				},
				existingTest,
			};
		}
		case ALL_TEST_DATA: {
			return {
				...state,
				testData: action.payload,
			};
		}
		case EXISTING_TEST_PARAMETERS: {
			return {
				...state,
				existingTest: {
					...state.existingTest,
					isTrue: true,
				},
			};
		}
		case TEST_EDIT_PARAMETERS: {
			return {
				...state,
				saveIsRun: 0,
				existingTest: {
					isTrue: false,
					isChanged: false,
				},
			};
		}
		default:
			return state;
	}
}
