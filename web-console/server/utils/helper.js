const logger = require('./logger');
const fs = require('fs');
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

const createFolderIfNotExists = (folderPath) => {
	if (!fs.existsSync(folderPath)) {
		fs.mkdirSync(folderPath, { recursive: true });
		logger.info(`folderPath created: ${folderPath}`);
	} else {
		logger.info(`folderPath already exists: ${folderPath}`);
	}
};

const readTextFromFile = (filePath) => {
	return fs.readFileSync(filePath, 'utf8');
};

const median = (numbers) => {
  if (!Array.isArray(numbers) || numbers.length === 0) return null;

  const sorted = [...numbers].sort((a, b) => a - b);

  const mid = Math.floor(sorted.length / 2);

  if (sorted.length % 2 === 0) {
    return (sorted[mid - 1] + sorted[mid]) / 2;
  } else {
    return sorted[mid];
  }
};

const formatSecondsToHHMMSS = (totalSeconds) => {
  const hours = String(Math.floor(totalSeconds / 3600)).padStart(2, '0');
  const minutes = String(Math.floor((totalSeconds % 3600) / 60)).padStart(2, '0');
  const seconds = String(totalSeconds % 60).padStart(2, '0');

  return `${hours}:${minutes}:${seconds}`;
};

module.exports = {
	checkRunningTestKeys,
	checkOrderParameter,
	createFolderIfNotExists,
	readTextFromFile,
	median,
	formatSecondsToHHMMSS,
}
