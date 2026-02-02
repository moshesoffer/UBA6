import React, {useEffect,} from 'react';
import {Navigate, useLocation,} from 'react-router-dom';
import PropTypes from 'prop-types';

import {useAuth, useAuthDispatch,} from 'src/store/AuthProvider';
import {validateString,} from 'src/utils/validators';
import {getItem,} from 'src/utils/localStorage';
import {setAuthCondition,} from 'src/actions/Auth';

function RequireAuth({children}) {

	let displayName = useAuth()?.displayName;
	const authDispatch = useAuthDispatch();

	useEffect(() => {
		if (!validateString(displayName)) {
			displayName = getItem('displayName');

			if (validateString(displayName)) {
				authDispatch(setAuthCondition(displayName));
			}
		}
	}, []);

	const location = useLocation();
	let from = location.state?.from?.pathname || '/';

	if (!validateString(displayName)) {
		// Redirect them to the /login page, but save the current location they were trying to go to when they were redirected.
		// This allows us to send them along to that page after they login, which is a nicer user experience than dropping them off on the home page.
		return <Navigate to="/login" state={{from: location}} replace/>;
	}

	return children;
}

RequireAuth.propTypes = {
	children: PropTypes.node,
};

export default RequireAuth;
