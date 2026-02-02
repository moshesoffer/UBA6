export const dateFromUtc = utcDate => {
	const date = new Date(utcDate);
	return new Date(date.getTime() + date.getTimezoneOffset() * 60000);
}
