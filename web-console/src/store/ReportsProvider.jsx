import React, {createContext, useContext, useReducer,} from 'react';
import PropTypes from 'prop-types';

import reportsReducer, {initialReports,} from 'src/reducers/Reports';

const ReportsContext = createContext(null);
const ReportsDispatchContext = createContext(null);

export default function ReportsProvider(props) {

	const {children} = props;

	const [reports, dispatch] = useReducer(reportsReducer, initialReports);

	return (
		<ReportsContext.Provider value={reports}>
			<ReportsDispatchContext.Provider value={dispatch}>
				{children}
			</ReportsDispatchContext.Provider>
		</ReportsContext.Provider>
	);
}

export function useReports() {
	return useContext(ReportsContext);
}

export function useReportsDispatch() {
	return useContext(ReportsDispatchContext);
}

ReportsProvider.propTypes = {
	children: PropTypes.node,
};
