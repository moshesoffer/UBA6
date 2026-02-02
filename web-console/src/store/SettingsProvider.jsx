import React, {createContext, useContext, useReducer,} from 'react';
import PropTypes from 'prop-types';

import settingsReducer, {initialSettings,} from 'src/reducers/Settings';

const SettingsContext = createContext(null);
const SettingsDispatchContext = createContext(null);

export default function SettingsProvider(props) {

	const {children} = props;

	const [settings, dispatch] = useReducer(settingsReducer, initialSettings);

	return (
		<SettingsContext.Provider value={settings}>
			<SettingsDispatchContext.Provider value={dispatch}>
				{children}
			</SettingsDispatchContext.Provider>
		</SettingsContext.Provider>
	);
}

export function useSettings() {
	return useContext(SettingsContext);
}

export function useSettingsDispatch() {
	return useContext(SettingsDispatchContext);
}

SettingsProvider.propTypes = {
	children: PropTypes.node,
};
