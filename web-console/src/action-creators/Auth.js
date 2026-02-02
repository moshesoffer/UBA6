import {setAuthCondition, setNotification,} from 'src/actions/Auth';
import {postData} from 'src/utils/httpRequests';
import {setItem, removeItem,} from 'src/utils/localStorage';
import {handleRequestError,} from 'src/utils/helper';

export const login = async (dispatch, payload) => {
	try {
		const result = await postData(dispatch, 'login', 'POST', payload);
		dispatch(setAuthCondition(result.name));
		setItem('displayName', result.name);
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		dispatch(setNotification({message: preparedMessage,}));
	}
}

export const logout = async dispatch => {
	try {
		await postData(dispatch, 'logout', 'POST');
	} catch (error) {
		const preparedMessage = handleRequestError(error);
		dispatch(setNotification({message: preparedMessage,}));
	} finally {
		dispatch(setAuthCondition(''));
		removeItem('displayName');
	}
}
