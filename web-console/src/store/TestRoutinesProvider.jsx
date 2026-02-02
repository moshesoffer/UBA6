import React, {createContext, useContext, useReducer,} from 'react';
import PropTypes from 'prop-types';

import testRoutinesReducer, {initialTestRoutines,} from 'src/reducers/TestRoutines';

const TestRoutinesContext = createContext(null);
const TestRoutinesDispatchContext = createContext(null);

export default function TestRoutinesProvider(props) {

	const {children} = props;

	const [testRoutines, dispatch] = useReducer(testRoutinesReducer, initialTestRoutines);

	return (
		<TestRoutinesContext.Provider value={testRoutines}>
			<TestRoutinesDispatchContext.Provider value={dispatch}>
				{children}
			</TestRoutinesDispatchContext.Provider>
		</TestRoutinesContext.Provider>
	);
}

export function useTestRoutines() {
	return useContext(TestRoutinesContext);
}

export function useTestRoutinesDispatch() {
	return useContext(TestRoutinesDispatchContext);
}

TestRoutinesProvider.propTypes = {
	children: PropTypes.node,
};
