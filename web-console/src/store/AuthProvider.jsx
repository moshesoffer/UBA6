import React, {createContext, useContext, useReducer,} from 'react';
import PropTypes from 'prop-types';

import authReducer, {initialAuth,} from 'src/reducers/Auth';

const AuthContext = createContext(null);
const AuthDispatchContext = createContext(null);

export default function AuthProvider(props) {

	const {children} = props;

	const [auth, dispatch] = useReducer(authReducer, initialAuth);

	return (
		<AuthContext.Provider value={auth}>
			<AuthDispatchContext.Provider value={dispatch}>
				{children}
			</AuthDispatchContext.Provider>
		</AuthContext.Provider>
	);
}

export function useAuth() {
	return useContext(AuthContext);
}

export function useAuthDispatch() {
	return useContext(AuthDispatchContext);
}

AuthProvider.propTypes = {
	children: PropTypes.node,
};
