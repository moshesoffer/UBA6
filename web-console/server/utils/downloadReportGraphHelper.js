const logger = require('../utils/logger');
const {generalConsts, cells} = require('../utils/excelConstants');
const { getReportWithTestResults,} = require('../services/reportService');
const XlsxPopulate = require('xlsx-populate');
const path = require('path');
const fs = require('fs');
const { exec } = require('child_process');
const { v4: uuidv4 } = require('uuid');
const { validateArray, } = require('../utils/validators');
const { median } = require('../utils/helper');

const UNIT_VARIANTS = {
	absoluteMa: 'mA',
	absoluteA: 'A',
	absoluteMah: 'mAh',
	relative: 'C',
	absoluteAh: 'Ah',
	power: 'W',
	resistance: 'Ohm',
}

const getLastDischarges = (planByStepIndex) => {
	let lastDischargesCapacity = 0;
	let lastDischargesEnergy = 0;
	let dischargeStepFound = false;
	//sorted desc going from last step to first step
	Object.entries(planByStepIndex).sort(([a], [b]) => Number(b) - Number(a)).forEach(([stepIndex, planStepWrapper]) => {
		if (planStepWrapper.planStep.type === 'discharge') {
			dischargeStepFound = true;
			lastDischargesCapacity += planStepWrapper.capacitySum;
			lastDischargesEnergy += planStepWrapper.energySum;
		} else if(dischargeStepFound) {
			return { lastDischargesCapacity: (lastDischargesCapacity / 3600) / 1000, lastDischargesEnergy: (lastDischargesEnergy / 3600) / 1000 };
		}
	});
	return { lastDischargesCapacity: (lastDischargesCapacity / 3600) / 1000, lastDischargesEnergy: (lastDischargesEnergy / 3600) / 1000 };
}

const deleteFiles = (excelOutputFilePath, pdfPath) => {
	if (fs.existsSync(excelOutputFilePath)){
		fs.unlinkSync(excelOutputFilePath);
	}
	if (fs.existsSync(pdfPath)){
		fs.unlinkSync(pdfPath);
	}
}

const downloadReportsGraph = async (req, res) => {
	let excelOutputFilePath;
	let excelFileStream;
	let pdfPath;
	let pdfFileStream;

	try {
		const exportType = req.params?.exportType;
		if(exportType !== 'XSLX' && exportType !== 'PDF') return returnErrorCode(res, 400, 'Invalid export type. Use XSLX or PDF');
		logger.info(`downloadReportsGraph [${req.params?.reportID}] going to getReportWithTestResults`);
		const reportWithTestResults = await getReportWithTestResults(req.params?.reportID);
		logger.info(`downloadReportsGraph [${req.params?.reportID}] finished getReportWithTestResults`);
		if(!validateArray(reportWithTestResults)) return returnErrorCode(res, 404, 'Report not found');
		const testData = reportWithTestResults[0];
		if(!testData) return returnErrorCode(res, 404, 'Report not found.');
		const testResults = testData.testResults;//the json file with all the test results
		const planArr = testData.plan;
		if(!validateArray(testResults) || !validateArray(planArr)) return returnErrorCode(res, 404, 'Test results or plan not found.');


		const templatePath = path.join(__dirname, generalConsts.excelTemplateFilePath);
		const workbook = await XlsxPopulate.fromFileAsync(templatePath);
		const dataSheet = workbook.sheet(generalConsts.DataSheetName);
		const reportSheet = workbook.sheet(generalConsts.reportSheetName);
		logger.info(`downloadReportsGraph [${req.params?.reportID}] [${templatePath}] start to fill testData`);
		
		reportSheet.cell(cells.reportName).value(testData.testName);
		//last discharges will be set later on in this function
		
		//Test Data:
		reportSheet.cell(cells.startTime).value(testData.timestampStart);
		reportSheet.cell(cells.ubaSN).value(testData.ubaSN);
		reportSheet.cell(cells.ubaChannel).value(testData.channel);
		
		reportSheet.cell(cells.customer).value(testData.customer);
		reportSheet.cell(cells.workOrderNo).value(testData.workOrderNumber);

		//Battery Rated data:
		reportSheet.cell(cells.batteryPN).value(testData.batteryPN);
		reportSheet.cell(cells.batterySN).value(testData.batterySN);
		reportSheet.cell(cells.chemistry).value(testData.chemistry);
		reportSheet.cell(cells.noCellsInSerial).value(testData.noCellSerial);
		reportSheet.cell(cells.noCellsInParallel).value(testData.noCellParallel);
		reportSheet.cell(cells.ratedCapacity).value(testData.ratedBatteryCapacity);
		reportSheet.cell(cells.cellPN).value(testData.cellPN);
		reportSheet.cell(cells.cellBatchDateCode).value(testData.cellBatch);
		reportSheet.cell(cells.cellSupplier).value(testData.cellSupplier);

		logger.info(`downloadReportsGraph [${req.params?.reportID}] start to fill testResults data sheet for the graph and reportSheet steps`);
		let previousStepIndex = -1;
		const planByStepIndex = {};
		let previousTimestamp = 0;
		let maxTemperature = -9999;
		testResults.forEach((testResult, index) => {
			const currentPlanIndex = testResult.planIndex;
			const currentStepIndex = testResult.stepIndex;
			const planStep = planArr[currentPlanIndex];
			const row = index + 2;
			dataSheet.cell(`A${row}`).value(testResult.timestamp);
			dataSheet.cell(`B${row}`).value(planStep.type);
			dataSheet.cell(`C${row}`).value(testResult.voltage);
			dataSheet.cell(`D${row}`).value(testResult.current);
			dataSheet.cell(`E${row}`).value(testResult.temperature);
			dataSheet.cell(`F${row}`).value(currentStepIndex);
			dataSheet.cell(`G${row}`).value(currentPlanIndex);
			if(previousStepIndex != currentStepIndex) {
				//this is a new test plan step
				//planByStepIndex[previousStepIndex].lastTestResult = testResults[index - 1];

				previousStepIndex = currentStepIndex;
				if (planByStepIndex[currentStepIndex]) {
					logger.warn(`THIS IS NOT EXPECTED!!! downloadReportsGraph [${req.params?.reportID}] duplicate planIndex found: ${currentStepIndex}`);
					return returnErrorCode(res, 404, 'Duplicate stepIndex found');
				}
				planByStepIndex[currentStepIndex] = {
					planStep,
					planIndex: currentPlanIndex,
					//firstTestResult: testResult,
					timestampArr: [],
					currentArr: [],
					voltageArr: [],
					temperatureArr: [],
					capacitySum: 0,
					energySum: 0,
				}
			}
			if(testResult.temperature > maxTemperature) maxTemperature = testResult.temperature;

			planByStepIndex[currentStepIndex].timestampArr.push(testResult.timestamp);
			planByStepIndex[currentStepIndex].currentArr.push(testResult.current);
			planByStepIndex[currentStepIndex].voltageArr.push(testResult.voltage);
			planByStepIndex[currentStepIndex].temperatureArr.push(testResult.temperature);
			const deltaT = testResult.timestamp - previousTimestamp;
			planByStepIndex[currentStepIndex].capacitySum += (testResult.current * deltaT);
			planByStepIndex[currentStepIndex].energySum += (deltaT * (testResult.current / 1000) * testResult.voltage);

			previousTimestamp = testResult.timestamp;
		});

		let rowNumber = 41;
		//iterate ascending from step 1 and on
		Object.entries(planByStepIndex).sort(([a], [b]) => Number(a) - Number(b)).forEach(([stepIndex, planStepWrapper]) => {
			if(planStepWrapper.planStep.type==='charge') {
				rowNumber = addChargeStepFromPlan(stepIndex, planStepWrapper, testData, rowNumber, reportSheet);
			} else if (planStepWrapper.planStep.type==='discharge') {
				rowNumber = addDischargeStepFromPlan(stepIndex, planStepWrapper, testData, rowNumber, reportSheet);
			} else if (planStepWrapper.planStep.type==='delay') {
				rowNumber = addDelayStepFromPlan(stepIndex, planStepWrapper, testData, rowNumber, reportSheet);
			} else {
				logger.warn(`THIS IS NOT EXPECTED!!! downloadReportsGraph [${req.params?.reportID}] unknown planStep type: ${planStepWrapper.planStep.type}`);
				return returnErrorCode(res, 404, 'Unknown planStep type found');
			}
		});
		
		const lastDischarges = getLastDischarges(planByStepIndex);
		reportSheet.cell(cells.lastDischarges1).value(lastDischarges.lastDischargesCapacity);
		reportSheet.cell(cells.lastDischarges2).value(lastDischarges.lastDischargesEnergy);
		//add summary
		addSummary(testData, lastDischarges, maxTemperature, rowNumber, reportSheet);
		

		const id = uuidv4();
		excelOutputFilePath = path.join(__dirname, 'output-' + id + '.xlsx');
		logger.info(`downloadReportsGraph [${req.params?.reportID}] write to [${excelOutputFilePath}]`);
		await workbook.toFileAsync(excelOutputFilePath);
		if(exportType === 'XSLX'){
			// Set headers for file download filename="${resultsGraphData[0].reportID}.xlsx"
			res.setHeader('Content-Disposition', `attachment; filename="${testData.reportID}.xlsx"`);
			res.setHeader('Content-Type', 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
			logger.info(`downloadReportsGraph [${req.params?.reportID}] createReadStream to [${excelOutputFilePath}]`);
			// Stream the Excel file to the client
			excelFileStream = fs.createReadStream(excelOutputFilePath);
			excelFileStream.on('error', (error) => {
				logger.error(`Error reading file: ${error.message}`);
				deleteFiles(excelOutputFilePath, pdfPath);
				return returnErrorCode(res, 500, 'Error reading file');
			});
			excelFileStream.pipe(res);
			// Clean up the file after the response is sent
			excelFileStream.on('end', () => {
				fs.unlinkSync(excelOutputFilePath);
			});
		} else {
			//exec('soffice --headless --convert-to pdf ' +  excelOutputFilePath, (error, stdout, stderr) => {
			exec('soffice --headless --convert-to pdf --outdir ' + __dirname + ' ' + excelOutputFilePath, (error, stdout, stderr) => {
				if (error) {
					logger.error(`****Error: ${error.message}`);
					deleteFiles(excelOutputFilePath, pdfPath);
					return returnErrorCode(res, 500, 'Error converting file');
				}
				if (stderr) {
					logger.error(`*****Stderr: ${stderr}`);
					deleteFiles(excelOutputFilePath, pdfPath);
					return returnErrorCode(res, 500, 'Error in conversion process');
				}
				
				logger.info(`pdf converted ${excelOutputFilePath}    Stdout: `, stdout);
				try {
					res.setHeader('Content-Disposition', `attachment; filename="${testData.reportID}.pdf"`);
					res.setHeader('Content-Type', 'application/pdf');
					pdfPath = excelOutputFilePath.replace('.xlsx', '.pdf');
					pdfFileStream = fs.createReadStream(pdfPath);
					pdfFileStream.on('error', (error) => {
						deleteFiles(excelOutputFilePath, pdfPath);
						logger.error(`Error reading file: ${error.message}`);
						return returnErrorCode(res, 500, 'Error reading file');
					});
					pdfFileStream.pipe(res);
					// Clean up the file after the response is sent
					pdfFileStream.on('end', () => {
						fs.unlinkSync(excelOutputFilePath);
						fs.unlinkSync(pdfPath);
					});
				} catch (error) {
					logger.error('downloadReportsGraph of pdf', error);
					deleteFiles(excelOutputFilePath, pdfPath);
					res.sendStatus(500);
				}
				
			});
		}
	} catch (error) {
		//TODO delete file if fails
		logger.error('downloadReportsGraph error', error);
		deleteFiles(excelOutputFilePath, pdfPath);
		res.sendStatus(500);
	}
};

const returnErrorCode = (res, statusCode, message) => {
	res.status(statusCode).send(message);
};

const addChargeStepFromPlan = (stepIndex, planStepWrapper, testData, rowNumber, reportSheet) => {
	const planStep = planStepWrapper.planStep;
	reportSheet.cell(`C${rowNumber}`).value(stepIndex).style("bold", true);
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.charge).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.source);
	reportSheet.cell(`E${rowNumber}`).value(planStep.source);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.chargeCapacity);
	const chargeCapacity = planStepWrapper.capacitySum / 3600;
	const chargeCapacityT = chargeCapacity > 1000 ? chargeCapacity / 1000 : chargeCapacity;
	reportSheet.cell(`I${rowNumber}`).value(chargeCapacityT);
	reportSheet.cell(`J${rowNumber}`).value(chargeCapacity > 1000 ? 'Ah' : 'mAh');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.chargeCurrent);
	reportSheet.cell(`E${rowNumber}`).value(splitUnitVariants(planStep.chargeCurrent).first);
	reportSheet.cell(`F${rowNumber}`).value(splitUnitVariants(planStep.chargeCurrent).second);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.chargeEnergy);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.energySum / 3600);
	reportSheet.cell(`J${rowNumber}`).value('Wh');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.chargePerCell);
	reportSheet.cell(`E${rowNumber}`).value(planStep.chargePerCell);
	reportSheet.cell(`F${rowNumber}`).value('V');
	reportSheet.cell(`I${rowNumber}`).value(((chargeCapacity / 1000) / testData.ratedBatteryCapacity) * 100);
	reportSheet.cell(`J${rowNumber}`).value('%');

	rowNumber++;
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.duration);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.timestampArr[planStepWrapper.timestampArr.length - 1] - planStepWrapper.timestampArr[0]);

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTermperature);
	reportSheet.cell(`E${rowNumber}`).value(planStep.maxTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.initialVoltage);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.voltageArr[0]);
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTime);
	reportSheet.cell(`E${rowNumber}`).value(planStep.maxTime);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endVoltage);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.voltageArr[planStepWrapper.voltageArr.length - 1]);
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.cutOffCurrent);
	reportSheet.cell(`E${rowNumber}`).value(splitUnitVariants(planStep.cutOffCurrent).first);
	reportSheet.cell(`F${rowNumber}`).value(splitUnitVariants(planStep.cutOffCurrent).second);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endTemperature);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.temperatureArr[planStepWrapper.temperatureArr.length - 1]);
	reportSheet.cell(`J${rowNumber}`).value('°C');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.chargeLimit);
	reportSheet.cell(`E${rowNumber}`).value(splitUnitVariants(planStep.chargeLimit).first);
	reportSheet.cell(`F${rowNumber}`).value(splitUnitVariants(planStep.chargeLimit).second);

	return rowNumber + 3;
}

const addDelayStepFromPlan = (stepIndex, planStepWrapper, testData, rowNumber, reportSheet) => {
	const planStep = planStepWrapper.planStep;
	reportSheet.cell(`C${rowNumber}`).value(stepIndex).style("bold", true);
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.delay).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.time);
	reportSheet.cell(`E${rowNumber}`).value(planStep.time);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endTemperature);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.temperatureArr[planStepWrapper.temperatureArr.length - 1]);
	reportSheet.cell(`J${rowNumber}`).value('°C');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.toTemperature);
	reportSheet.cell(`E${rowNumber}`).value(planStep.waitTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');

	return rowNumber + 3;
}

const addDischargeStepFromPlan = (stepIndex, planStepWrapper, testData, rowNumber, reportSheet) => {
	const planStep = planStepWrapper.planStep;
	reportSheet.cell(`C${rowNumber}`).value(stepIndex).style("bold", true);
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.disCharge).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.source);
	reportSheet.cell(`E${rowNumber}`).value(planStep.source);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.disChargeCurrent);
	reportSheet.cell(`E${rowNumber}`).value(splitUnitVariants(planStep.dischargeCurrent).first);
	reportSheet.cell(`F${rowNumber}`).value(splitUnitVariants(planStep.dischargeCurrent).second);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.dischargeCapacity);
	const chargeCapacity = planStepWrapper.capacitySum / 3600;
	const chargeCapacityT = chargeCapacity > 1000 ? chargeCapacity / 1000 : chargeCapacity;
	reportSheet.cell(`I${rowNumber}`).value(chargeCapacityT);
	reportSheet.cell(`J${rowNumber}`).value(chargeCapacity > 1000 ? 'Ah' : 'mAh');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.minTemperature);
	reportSheet.cell(`E${rowNumber}`).value(planStep.minTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.dischargeEnergy);
	reportSheet.cell(`I${rowNumber}`).value(Math.abs(planStepWrapper.energySum / 3600));
	reportSheet.cell(`J${rowNumber}`).value('Wh');

	rowNumber++;
	reportSheet.cell(`I${rowNumber}`).value(((chargeCapacity / 1000) / testData.ratedBatteryCapacity) * 100);
	reportSheet.cell(`J${rowNumber}`).value('%');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTermperature);
	reportSheet.cell(`E${rowNumber}`).value(planStep.maxTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.duration);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.timestampArr[planStepWrapper.timestampArr.length - 1] - planStepWrapper.timestampArr[0]);

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTime);
	reportSheet.cell(`E${rowNumber}`).value(planStep.maxTime);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.initialVoltage);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.voltageArr[0]);
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.cutOffVoltage);
	reportSheet.cell(`E${rowNumber}`).value(planStep.cutOffVoltage);
	reportSheet.cell(`F${rowNumber}`).value('V');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.midPointVoltage);
	reportSheet.cell(`I${rowNumber}`).value(median(planStepWrapper.voltageArr));
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.disChargeLimit);
	reportSheet.cell(`E${rowNumber}`).value(splitUnitVariants(planStep.dischargeLimit).first);
	reportSheet.cell(`F${rowNumber}`).value(splitUnitVariants(planStep.dischargeLimit).second);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endVoltage);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.voltageArr[planStepWrapper.voltageArr.length - 1]);
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.midPointCurrent);
	reportSheet.cell(`I${rowNumber}`).value(median(planStepWrapper.currentArr));
	reportSheet.cell(`J${rowNumber}`).value('A');

	rowNumber++;
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endTemperature);
	reportSheet.cell(`I${rowNumber}`).value(planStepWrapper.temperatureArr[planStepWrapper.temperatureArr.length - 1]);
	reportSheet.cell(`J${rowNumber}`).value('°C');

	return rowNumber + 3;
}

const addSummary = (testData, lastDischarges, maxTemperature, rowNumber, reportSheet) => {
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.summary).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.lastDischarges);
	reportSheet.cell(`E${rowNumber}`).value(lastDischarges.lastDischargesCapacity);
	reportSheet.cell(`F${rowNumber}`).value('Ah');

	rowNumber++;
	reportSheet.cell(`E${rowNumber}`).value(lastDischarges.lastDischargesEnergy);
	reportSheet.cell(`F${rowNumber}`).value('Wh');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.fromRated);
	reportSheet.cell(`E${rowNumber}`).value((lastDischarges.lastDischargesCapacity / testData.ratedBatteryCapacity) * 100);
	reportSheet.cell(`F${rowNumber}`).value('%');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTermperature);
	reportSheet.cell(`E${rowNumber}`).value(maxTemperature);
	reportSheet.cell(`F${rowNumber}`).value('°C');

	rowNumber+=2;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.conductedBy);
	reportSheet.cell(`E${rowNumber}`).value(testData.conductedBy);

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.approvedBy);
	reportSheet.cell(`E${rowNumber}`).value(testData.approvedBy);
}

const splitUnitVariants = (str) => {
  if (typeof str !== 'string') return { first: undefined, second: undefined };
  const parts = str.split(':');
  return {
    first: parts[0] !== '' ? parts[0] : undefined,
    second: parts[1] !== '' ? UNIT_VARIANTS[parts[1]] : undefined
  };
}

module.exports = {
	downloadReportsGraph,
};