import {printCelsius, } from 'src/utils/helper';
import {category, statusCodes, getKeyByValue, UBA_CHANNEL_LIST, isTestRunning} from 'src/constants/unsystematic';
import {getText,} from 'src/services/string-definitions';
import {dateFromUtc,} from 'src/utils/dateTimeHelper';

const formatSeconds = seconds => [
	parseInt(seconds / 60 / 60, 10),
	parseInt(seconds / 60 % 60, 10),
	parseInt(seconds % 60, 10),
	// eslint-disable-next-line prefer-named-capture-group
].join(':').replace(/\b(\d)\b/ug, '0$1');

const getRuntime = timestampStart => {
	const start = dateFromUtc(timestampStart);
	const now = new Date();
	let diff = now.getTime() - start.getTime();
	diff = Math.round(diff / 1000);
	return formatSeconds(diff);
}

export const enrichUbaDevicesWithRunTime = (ubaDevices) => ubaDevices.map(ubaDevice => {
	if(ubaDevice.testRoutineChannels === UBA_CHANNEL_LIST.A_AND_B && ubaDevice.status !== statusCodes.STANDBY && !ubaDevice.parallelRun){
		const otherObj = ubaDevices.find(ubaDeviceObj => ubaDeviceObj.testRoutineChannels === UBA_CHANNEL_LIST.A_AND_B &&
			ubaDeviceObj.status !== statusCodes.STANDBY && ubaDeviceObj.ubaSN === ubaDevice.ubaSN &&
				ubaDeviceObj.channel !== ubaDevice.channel);
		ubaDevice.parallelRun = true;
		otherObj.parallelRun = true;
		//console.log('otherObj', otherObj, ubaDevice);
	}
	const runtime = getRuntime(ubaDevice.timestampStart);
	return {
		...ubaDevice,
		runtime,
	};
});

export const getVoltage = voltage => {
	if (voltage === null) {
		return getText('common.NOT_APPLICABLE');
	}
	if(voltage > 1000){
		return `${(Number(voltage/1000)).toFixed(2)}V`;
	}
	return `${(Number(voltage))}mV`;
	
}

export const getChargeCurrent = chargeCurrent => {
	if (chargeCurrent === null) {
		return getText('common.NOT_APPLICABLE');
	}
	if(chargeCurrent > 1000){
		return `${Number(chargeCurrent).toFixed(2)}A`;
	}
	return `${Number(chargeCurrent*1000).toFixed(2)}mA`;
}

export const getTemperature = temperature => {
	if (temperature === null) {
		return getText('common.NOT_APPLICABLE');
	}
	if(temperature<= -273){
		return getText('common.NOT_APPLICABLE');
	}

	return printCelsius(Number(temperature).toString());
}

export const getTestStep = data => {
	if (data?.status === null) {
		return getText('common.NOT_APPLICABLE');
	}

	if (data?.status===statusCodes.RUNNING) {
		return `${data?.testCurrentStep}/${data?.totalStagesAmount}`;
	}

	return getKeyByValue(statusCodes, data?.status);
}

export const getTestType = data => {
	if (data?.status === null) {
		return getText('common.NOT_APPLICABLE');
	}

	if (data?.status===statusCodes.RUNNING) {
		return data?.testState
	}

	return '';
}

export const generateDistinctColors = (numColors) => {
    const colors = [];
    const saturation = 70;  // Adjust the saturation (0-100%) for color vividness
    const lightness = 50;   // Adjust the lightness (0-100%) for brightness

    for (let i = 0; i < numColors; i+=1) {
        const hue = Math.floor((360 / numColors) * i);  // Evenly distribute hues
        const color = hslToHex(hue, saturation, lightness);
        colors.push(color);
    }

    return colors;
}

// Helper function to convert HSL to HEX
const hslToHex = (h, s, l) => {
    s /= 100;
    l /= 100;

    const c = (1 - Math.abs(2 * l - 1)) * s;
    const x = c * (1 - Math.abs((h / 60) % 2 - 1));
    const m = l - c / 2;
    let b = 0, g = 0, r = 0;

    if (h >= 0 && h < 60) {
        r = c; g = x; b = 0;
    } else if (h >= 60 && h < 120) {
        r = x; g = c; b = 0;
    } else if (h >= 120 && h < 180) {
        r = 0; g = c; b = x;
    } else if (h >= 180 && h < 240) {
        r = 0; g = x; b = c;
    } else if (h >= 240 && h < 300) {
        r = x; g = 0; b = c;
    } else if (h >= 300 && h < 360) {
        r = c; g = 0; b = x;
    }

    r = Math.round((r + m) * 255);
    g = Math.round((g + m) * 255);
    b = Math.round((b + m) * 255);

    return `#${  ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1).toUpperCase()}`;
}

export const convertTimestampToTime = (timestamp) => {
	const millis = timestamp * 1000;
	const hours = Math.floor(millis / 3600000); // 3600000 = 60 * 60 * 1000 (milliseconds in an hour)
	const minutes = Math.floor((millis % 3600000) / 60000); // Remaining minutes
	const seconds = Math.floor((millis % 60000) / 1000); // Remaining seconds

	const formattedHours = String(hours).padStart(2, '0');
	const formattedMinutes = String(minutes).padStart(2, '0');
	const formattedSeconds = String(seconds).padStart(2, '0');

	// If there are hours, return in HH:MM:SS format, otherwise return MM:SS
	return hours > 0
		? `${formattedHours}:${formattedMinutes}:${formattedSeconds}`
		: `${formattedMinutes}:${formattedSeconds}`;
}
