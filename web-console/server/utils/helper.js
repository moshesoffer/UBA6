const {validateString, validateArray,} = require('./validators');
const {testChannels,} = require('./constants');

const orderEnum = ['ASC', 'DESC'];

const checkRunningTestKeys = ubaSNs => {
	if (!validateArray(ubaSNs, true)) {
		throw new Error(`The ubaSNs ${ubaSNs} should be not empty array.`);
	}

	ubaSNs.forEach(item => {
		if (!validateString(item.ubaSN) || !validateString(item.ubaSN.trim())) {
			throw new Error(`Invalid ubaSN ${item.ubaSN}.`);
		}

		if (!testChannels.includes(item.channel)) {
			throw new Error(`Invalid channel ${item.channel}.`);
		}
	});
}

const checkOrderParameter = metadata => {
	if (!orderEnum.includes(metadata.order.toUpperCase())) {
		throw new Error(`Invalid order ${metadata.order}.`);
	}
}

const enrichUbaDevices = (ubaDevices, instantTestResults) => ubaDevices.map(ubaDevice => {

	let testState = null;
	let testCurrentStep = null;
	let voltage = null;
	let current = null;
	let temp = null;
	let capacity = null;
	let error = null;

	for (const result of instantTestResults) {
		if (ubaDevice.runningTestID === result.runningTestID) {
			({
				testState,
				testCurrentStep,
				voltage,
				current,
				temp,
				capacity,
				error
			} = result);

			break;
		}
	}

	return {
		...ubaDevice,
		testState,
		testCurrentStep,
		voltage,
		current,
		temp,
		capacity,
		error,
	};
});

module.exports = {
	checkRunningTestKeys,
	checkOrderParameter,
	enrichUbaDevices,
}
