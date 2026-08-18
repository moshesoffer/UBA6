//const logger = require('./logger');
import {printCelsius, } from 'src/utils/helper';
import {category, statusCodes, getKeyByValue, UBA_CHANNEL_LIST, isTestRunning} from 'src/constants/unsystematic';
import {getText,} from 'src/services/string-definitions';
import {dateFromUtc,} from 'src/utils/dateTimeHelper';

const runtimeData = new Map();

const createRuntimeData = () => ({
    startTimeA: -1,
    pausedateChnlB: 0,
    runtimeChnlA: null,
    rundateChnlA: 0,

    startTimeB: -1,
    pausedateChnlA: 0,
    runtimeChnlB: null,
    rundateChnlB: 0,
});

const getRuntimeData = ubaSN => {
    if (!runtimeData.has(ubaSN)) {
        runtimeData.set(ubaSN, createRuntimeData());
    }
    return runtimeData.get(ubaSN);
};


export const getTestResultTimestamps = (reports) => {
	if (!reports) return [];

	return reports.flatMap(report =>
		report.testResults?.map(tr => tr.timestamp) || []
	);
};

export const buildRuntimeMap = (instantTestResults) => {
	if (!instantTestResults) return {};

	const map = {};

	instantTestResults.forEach(r => {
		map[r.timestamp] = convertTimestampToTime(r.timestamp);
	});

	return map;
};



const formatSeconds = seconds => [
	parseInt(seconds / 60 / 60, 10),
	parseInt(seconds / 60 % 60, 10),
	parseInt(seconds % 60, 10),
	// eslint-disable-next-line prefer-named-capture-group
].join(':').replace(/\b(\d)\b/ug, '0$1');

const getRuntime = (timestamp, startTimestamp) => {
    const now = dateFromUtc(timestamp);
    const start = dateFromUtc(startTimestamp);

    let diff = now.getTime() - start.getTime();
    diff = Math.round(diff / 1000);

    return diff;
};

export const getTestRuntime1 = ubaDevice => {
    if (!ubaDevice) return null;
    if (ubaDevice.channel !== 'A' && ubaDevice.channel !== 'B') return null;

	let currTime;

	const instant = ubaDevice.instantTestResults || [];
//	const runningTests = ubaDevice.runningTests || [];
//console.log('instant:', instant.length);
//console.log('runningTests:', runningTests.length);
	const lastInstantTimestamp =
    	instant.length > 0
    	    ? instant[instant.length - 1].lastInstantResultsTimestamp
    	    : ubaDevice.lastInstantResultsTimestamp; // fallback
	const testState =
    	instant.length > 0
    	    ? instant[instant.length - 1].testState
    	    : ubaDevice.testState; // fallback
//console.log (`==> lastInstantTimestamp: ${lastInstantTimestamp} testState: ${testState}`);

    if (ubaDevice.channel === 'A') {
        if (
            testState === 'Charge' ||
            testState === 'Discharge' ||
			testState === 'Pause'
        ) {
        	if (startTimeA === -1) {
            	startTimeA = ubaDevice.lastInstantResultsTimestamp;
        	}
//console.log('timestamp:', ubaDevice.lastInstantResultsTimestamp, startTimeA);
			currTime = getRuntime(ubaDevice.lastInstantResultsTimestamp, startTimeA);
            rundateChnlA = currTime - pausedateChnlA;
 			//console.log (`==> rundateChnlA: ${rundateChnlA}, currTime=${currTime}, pause=${pausedateChnlA},    now=${lastInstantTimestamp} start=${startTimeA}`);
//        } else if (testState === 'Pause') {
//			currTime = getRuntime(lastInstantTimestamp, startTimeA);
//            pausedateChnlA = currTime - rundateChnlA;
        } else if (testState === 'Standby') {
            runtimeChnlA = 0;
            startTimeA = -1;
        }

        runtimeChnlA = formatSeconds(rundateChnlA);
        return rundateChnlA;
    }

    if (ubaDevice.channel === 'B') {
        if (
            testState === 'Charge' ||
            testState === 'Discharge' ||
			testState === 'Pause'
        ) {
        	if (startTimeB === -1) {
            	startTimeB = ubaDevice.lastInstantResultsTimestamp;
        	}
//console.log('timestamp:', ubaDevice.lastInstantResultsTimestamp, startTimeB);
			currTime = getRuntime(ubaDevice.lastInstantResultsTimestamp, startTimeB);
            rundateChnlB = currTime - pausedateChnlB;
//        } else if (testState === 'Pause') {
//			currTime = getRuntime(ubaDevice.lastInstantResultsTimestamp, startTimeB);
//            pausedateChnlB = currTime - rundateChnlB;
        } else if (testState === 'Standby') {
            runtimeChnlB = 0;
            startTimeB = -1;
        }

        runtimeChnlB = formatSeconds(rundateChnlB);
        return rundateChnlB;
    }
};
export const getTestRuntime = ubaDevice => {
    if (!ubaDevice) return null;
    if (ubaDevice.channel !== 'A' && ubaDevice.channel !== 'B') return null;

    //const ubaSN = ubaDevice.ubaSN;
	const { ubaSN } = ubaDevice;
    const data = getRuntimeData(ubaSN);

    let currTime;

    const instant = ubaDevice.instantTestResults || [];

    const lastInstantTimestamp =
        instant.length > 0
            ? instant[instant.length - 1].lastInstantResultsTimestamp
            : ubaDevice.lastInstantResultsTimestamp;

    const testState =
        instant.length > 0
            ? instant[instant.length - 1].testState
            : ubaDevice.testState;

    if (ubaDevice.channel === 'A') {
        if (
            testState === 'Charge' ||
            testState === 'Discharge' ||
            testState === 'Pause'
        ) {
            if (data.startTimeA === -1) {
                data.startTimeA = ubaDevice.lastInstantResultsTimestamp;
            }

            currTime = getRuntime(
                ubaDevice.lastInstantResultsTimestamp,
                data.startTimeA
            );

            data.rundateChnlA = currTime - data.pausedateChnlA;

        } else if (testState === 'Standby') {
//logger.info(`Standby reset start_Time: ${ubaDevice.ubaSN} ${ubaDevice.channel}`);
            data.runtimeChnlA = 0;
            data.startTimeA = -1;
        }

        data.runtimeChnlA = formatSeconds(data.rundateChnlA);

        return data.rundateChnlA;
    }

    if (ubaDevice.channel === 'B') {
        if (
            testState === 'Charge' ||
            testState === 'Discharge' ||
            testState === 'Pause'
        ) {
            if (data.startTimeB === -1) {
                data.startTimeB = ubaDevice.lastInstantResultsTimestamp;
            }

            currTime = getRuntime(
                ubaDevice.lastInstantResultsTimestamp,
                data.startTimeB
            );

            data.rundateChnlB = currTime - data.pausedateChnlB;

        } else if (testState === 'Standby') {
            data.runtimeChnlB = 0;
            data.startTimeB = -1;
        }

        data.runtimeChnlB = formatSeconds(data.rundateChnlB);

        return data.rundateChnlB;
    }

    return null;
};


export const enrichUbaDevicesWithRunTime = (ubaDevices) => ubaDevices.map(ubaDevice => {	
	if(ubaDevice.testRoutineChannels === UBA_CHANNEL_LIST.A_AND_B && ubaDevice.status !== statusCodes.STANDBY && !ubaDevice.parallelRun){
		//const otherObj = ubaDevices.find(ubaDeviceObj => ubaDeviceObj.testRoutineChannels === UBA_CHANNEL_LIST.A_AND_B &&
		//								 ubaDeviceObj.status !== statusCodes.STANDBY && 
		//								 ubaDeviceObj.ubaSN === ubaDevice.ubaSN &&
		//								 ubaDeviceObj.channel !== ubaDevice.channel); - canonot be set
			
		ubaDevice.parallelRun = true;
		//otherObj.parallelRun = true; - canoot be set
	}

	let rundate = getTestRuntime(ubaDevices);

	//if (ubaDevice.channel === 'A') {
	//	rundateChnlA = formatSeconds(rundateChnlA);
		return {
			...ubaDevice,
			rundate,
		};

	//} else if (ubaDevice.channel === 'B') {
	//	rundateChnlB = formatSeconds(rundateChnlB);
	//	return {
	//		...ubaDevice,
	//		rundateChnlB,
	//	};
	//}
});

export const getTestRoutineName = ubaDevice => {
	return `${(ubaDevice.testName)}`;
}

export const getVoltage = voltage => {
	if (voltage === null) {
		return getText('common.NOT_APPLICABLE');
	}
	if(voltage > 1000){
		return `${(Number(voltage/1000)).toFixed(2)} V`;
	}
	return `${(Number(voltage))} mV`;
}

export const getChargeCurrent = chargeCurrent => {
	//console.log ('==> ', {chargeCurrent}, '[A]'); //input here is in [A]
	if (chargeCurrent === null) {
		return getText('common.NOT_APPLICABLE');
	}
	if(Math.abs(chargeCurrent) > 1){
		return `${Number(chargeCurrent).toFixed(3)} A`;
	}
	return `${Number(chargeCurrent * 1000).toFixed(2)} mA`;
}

export const getTemperature = temperature => {
	if (temperature === null) {
		return getText('common.NOT_APPLICABLE');
	}
	if(temperature<= -273){
		return getText('common.NOT_APPLICABLE');
	}

	return printCelsius(Number(temperature.toFixed(2)).toString());
}

export const getCapacity = capacity => {
	//console.log ('==> ', {capacity}, '[Ah]'); //input here is in [mAh]
	if (capacity === null) {
		return getText('common.NOT_APPLICABLE');
	}
	
	//return `${Number(capacity / 1000).toFixed(5)} Ah`;
	if(Math.abs(capacity) > 1000){
		return `${Number(capacity/1000).toFixed(3)} Ah`;
	}
	return `${Number(capacity).toFixed(3)} mAh`;
}

export const getTestStep = data => {
	if (data?.status === null) {
		return getText('common.NOT_APPLICABLE');
	}

	if (data?.status===statusCodes.RUNNING) {
		return `${data?.testCurrentStep}/${data?.numStages}`;
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
