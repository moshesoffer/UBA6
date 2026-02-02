import {validateInteger, validateString,} from 'src/utils/validators';

import text from './text';

const mutateString = (match, number, args) => {
	if (validateInteger(args[number]) || validateString(args[number])) {
		return args[number];
	}

	return match;
}

const printf = (string, ...replacement) => {
	const args = replacement;
	// eslint-disable-next-line
	return string.replace(/{(\d+)}/g, (match, number) => mutateString(match, number, args));
};

export const getText = (pathString, ...replacement) => {
	const path = pathString.split('.');

	let stringDefinition = text[path[0]][path[1]];
	if (validateString(text[path[0]][path[1]][path[2]])) {
		stringDefinition = text[path[0]][path[1]][path[2]];
	}

	if (!validateString(stringDefinition)) {
		throw new Error(`String definition not found for path: ${pathString}`);
	}

	return printf(stringDefinition, ...replacement);
}

export const getDate = rawDate => {
	const locale = 'he-IL';
	const timeZone = 'Asia/Jerusalem';
	const options = {
		year: '2-digit',
		month: '2-digit',
		day: '2-digit',
		hour: 'numeric',
		minute: 'numeric',
		second: 'numeric',
		hour12: false,
		timeZone,
	}

	const date = new Date(rawDate);
	if (date.toString() === 'Invalid Date') {
		// eslint-disable-next-line no-console
		console.info('Invalid date:', rawDate);
		return rawDate;
	}

	const dateFormat = new Intl.DateTimeFormat(locale, options);

	return dateFormat.format(date);
}
