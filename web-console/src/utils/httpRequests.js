import {validateString, validateObject, validateFunction,} from 'src/utils/validators';
import {setAjaxLoader} from 'src/actions/Auth';

const createRequestOptions = (method, data) => {
	const options = {
		method,
		headers: {
			'Content-Type': 'application/json',
		},
	};

	if (validateObject(data)) {
		options.body = JSON.stringify(data);
	}

	return options;
}

const prepareResponse = async response => {
	if (response.ok) {
		const dataString = await response.text();
		if (validateString(dataString)) {
			try {
				return JSON.parse(dataString);
			} catch (error) {
				// eslint-disable-next-line no-console
				console.info('Response:', dataString);
				// eslint-disable-next-line no-console
				console.info('Error parsing response:', error);
				throw new Error('JSON response parsing error.');
			}
		}

		return dataString;
	}

	let text = '';
	try {
		text = await response.text();
	} catch (error) {
		// eslint-disable-next-line no-console
		console.info(error);
	}
	if (validateString(text) && text.length < 30) {
		throw new Error(text);
	}

	if (validateString(response.statusText)) {
		throw new Error(response.statusText);
	}

	throw new Error('An error occurred.');
}

export const postData = async (authDispatch, pathname, method, data) => {
	const options = createRequestOptions(method, data);
	const url = `${import.meta.env.VITE_API_URL}/${pathname}`;

	if (validateFunction(authDispatch)) {
		//console.log('postData authDispatch', pathname,method, authDispatch);
		authDispatch(setAjaxLoader(true));
	}
	try {
		const response = await fetch(url, options);
		return await prepareResponse(response);
	} finally {
		if (validateFunction(authDispatch)) {
			authDispatch(setAjaxLoader(false));
		}
	}
}
