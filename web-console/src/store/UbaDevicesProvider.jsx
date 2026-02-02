import React, {createContext, useContext, useReducer,} from 'react';
import PropTypes from 'prop-types';

import ubaDevicesReducer, {initialUbaDevices,} from 'src/reducers/UbaDevices';

const UbaDevicesContext = createContext(null);
const UbaDevicesDispatchContext = createContext(null);

export default function UbaDevicesProvider(props) {

	const {children} = props;

	const [ubaDevices, dispatch] = useReducer(ubaDevicesReducer, initialUbaDevices);

	return (
		<UbaDevicesContext.Provider value={ubaDevices}>
			<UbaDevicesDispatchContext.Provider value={dispatch}>
				{children}
			</UbaDevicesDispatchContext.Provider>
		</UbaDevicesContext.Provider>
	);
}

export function useUbaDevices() {
	return useContext(UbaDevicesContext);
}

export function useUbaDevicesDispatch() {
	return useContext(UbaDevicesDispatchContext);
}

UbaDevicesProvider.propTypes = {
	children: PropTypes.node,
};
