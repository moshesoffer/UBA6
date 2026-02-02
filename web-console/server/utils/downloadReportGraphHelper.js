const logger = require('../utils/logger');
const {generalConsts, cells} = require('../utils/excelConstants');
const { getReportWithTestResults,} = require('../services/reportService');
const XlsxPopulate = require('xlsx-populate');
const path = require('path');
const fs = require('fs');
const { exec } = require('child_process');
const { v4: uuidv4 } = require('uuid');

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
		logger.info(`downloadReportsGraph [${req.params?.reportID}] going to getReportWithTestResults`);
		const reportWithTestResults = await getReportWithTestResults(req.params?.reportID);
		logger.info(`downloadReportsGraph [${req.params?.reportID}] finished getReportWithTestResults`);
		if(exportType === 'XSLX' || exportType === 'PDF') {
			const templatePath = path.join(__dirname, generalConsts.excelTemplateFilePath);
			const workbook = await XlsxPopulate.fromFileAsync(templatePath);
			const dataSheet = workbook.sheet(generalConsts.DataSheetName);
			const reportSheet = workbook.sheet(generalConsts.reportSheetName);
			const testResults = reportWithTestResults[0]?.testResults;
			const testData = reportWithTestResults[0];
			logger.info(`downloadReportsGraph [${req.params?.reportID}] [${templatePath}] start to fill testResults`);
			if(testResults) {
				testResults.forEach((result, index) => {
					const row = index + 2;
					dataSheet.cell(`A${row}`).value(result.current);
					dataSheet.cell(`B${row}`).value(result.voltage);
					dataSheet.cell(`C${row}`).value(result.timestamp);
					dataSheet.cell(`D${row}`).value(result.temperature);
				});
			}
			logger.info(`downloadReportsGraph [${req.params?.reportID}] start to fill testData`);
			if(testData) {
				reportSheet.cell(cells.reportName).value(testData.testName);
				reportSheet.cell(cells.lastDischarges1).value('TODO');//last stepp of discharge G3=capacity I3=capacity*
				reportSheet.cell(cells.lastDischarges2).value('TODO');
				
				//Test Data:
				reportSheet.cell(cells.startTime).value(testData.timestampStart);
				reportSheet.cell(cells.ubaSN).value('TODO');//not exists today testData.ubaSN
				reportSheet.cell(cells.ubaChannel).value(testData.channel);
				
				reportSheet.cell(cells.customer).value(testData.customer);
				reportSheet.cell(cells.workOrderNo).value(testData.workOrderNumber);

				//Battery Rated data:
				reportSheet.cell(cells.batteryPN).value(testData.batteryPN);
				reportSheet.cell(cells.batterySN).value(testData.batterySN);
				reportSheet.cell(cells.chemistry).value('TODO');//currently we dont have testData.chemistry
				reportSheet.cell(cells.noCellsInSerial).value(testData.noCellSerial);
				reportSheet.cell(cells.noCellsInParallel).value(testData.noCellParallel);
				reportSheet.cell(cells.ratedCapacity).value(testData.ratedBatteryCapacity);
				reportSheet.cell(cells.cellPN).value('TODO');//currently we dont have testData.cellPN
				reportSheet.cell(cells.cellBatchDateCode).value('TODO');//currently we dont have date code testData.cellBatch
				reportSheet.cell(cells.cellSupplier).value('TODO');//currently we dont have supplier testData.cellSupplier

				const planArr = testData.plan;
				let stepNumber = 1;
				let rowNumber = 41;
				
				if(planArr) {
					for (let index = 0; index < planArr.length; index++) {
						const step = planArr[index];
						if(step.type==='charge') {
							rowNumber = addChargeStepFromPlan(stepNumber, step, rowNumber, reportSheet);
							stepNumber++;
						} else if (step.type==='discharge') {
							rowNumber = addDischargeStepFromPlan(stepNumber, step, rowNumber, reportSheet);
							stepNumber++;
						} else if (step.type==='delay') {
							rowNumber = addDelayStepFromPlan(stepNumber, step, rowNumber, reportSheet);
							stepNumber++;
						}
					}
					//add summary
					addSummary(testData.summary, rowNumber, reportSheet);
				}
				
			}
			
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
					res.status(500);
					return;
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
						res.status(500).send('Error converting file');
						return;
					}
					if (stderr) {
						logger.error(`*****Stderr: ${stderr}`);
						deleteFiles(excelOutputFilePath, pdfPath);
						res.status(500).send('Error in conversion process');
						return;
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
							res.status(500).send('Error reading file');;
							return;
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

			
		}
	} catch (error) {
		//TODO delete file if fails
		logger.error('downloadReportsGraph error', error);
		deleteFiles(excelOutputFilePath, pdfPath);
		res.sendStatus(500);
	}
};

const addChargeStepFromPlan = (stepNumber, step, rowNumber, reportSheet) => {
	reportSheet.cell(`C${rowNumber}`).value(stepNumber).style("bold", true);
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.charge).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.source);
	reportSheet.cell(`E${rowNumber}`).value(step.source);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.chargeCapacity);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//step.chargeCapacity doesnt exist
	reportSheet.cell(`J${rowNumber}`).value('Ah / mAh');//TODO!!!

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.chargeCurrent);
	reportSheet.cell(`E${rowNumber}`).value(step.chargeCurrent);
	reportSheet.cell(`F${rowNumber}`).value('mA / A / W');//TODO!!!
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.chargeEnergy);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//step.chargeEnergy
	reportSheet.cell(`J${rowNumber}`).value('Wh');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.chargePerCell);
	reportSheet.cell(`E${rowNumber}`).value(step.chargePerCell);
	reportSheet.cell(`F${rowNumber}`).value('V');
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('%');

	rowNumber++;
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.duration);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTermperature);
	reportSheet.cell(`E${rowNumber}`).value(step.maxTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.initialVoltage);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTime);
	reportSheet.cell(`E${rowNumber}`).value(step.maxTime);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endVoltage);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.cutOffCurrent);
	reportSheet.cell(`E${rowNumber}`).value(step.cutOffCurrent);
	reportSheet.cell(`F${rowNumber}`).value('mA');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endTemperature);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('°C');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.chargeLimit);
	reportSheet.cell(`E${rowNumber}`).value(step.chargeLimit);
	reportSheet.cell(`F${rowNumber}`).value('mA / A / W');//TODO!!!

	return rowNumber + 3;
}

const addDelayStepFromPlan = (stepNumber, step, rowNumber, reportSheet) => {
	reportSheet.cell(`C${rowNumber}`).value(stepNumber).style("bold", true);
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.delay).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.time);
	reportSheet.cell(`E${rowNumber}`).value(step.time);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endTemperature);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('°C');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.toTemperature);
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`F${rowNumber}`).value('°C');

	return rowNumber + 3;
}

const addDischargeStepFromPlan = (stepNumber, step, rowNumber, reportSheet) => {
	reportSheet.cell(`C${rowNumber}`).value(stepNumber).style("bold", true);
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.disCharge).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.source);
	reportSheet.cell(`E${rowNumber}`).value(step.source);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.disChargeCurrent);
	reportSheet.cell(`E${rowNumber}`).value(step.dischargeCurrent);
	reportSheet.cell(`F${rowNumber}`).value('mA / A / W');//TODO!!!
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.dischargeCapacity);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('Ah / mAh');//TODO!!!

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.minTemperature);
	reportSheet.cell(`E${rowNumber}`).value(step.minTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.dischargeEnergy);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('Wh');

	rowNumber++;
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('%');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTermperature);
	reportSheet.cell(`E${rowNumber}`).value(step.maxTemp);
	reportSheet.cell(`F${rowNumber}`).value('°C');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.duration);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTime);
	reportSheet.cell(`E${rowNumber}`).value(step.maxTime);
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.initialVoltage);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.cutOffVoltage);
	reportSheet.cell(`E${rowNumber}`).value(step.cutOffVoltage);
	reportSheet.cell(`F${rowNumber}`).value('V');
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.midPointVoltage);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.disChargeLimit);
	reportSheet.cell(`E${rowNumber}`).value(step.dischargeLimit);
	reportSheet.cell(`F${rowNumber}`).value('mA / A / W');//TODO!!!
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endVoltage);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('V');

	rowNumber++;
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.midPointCurrent);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('A');

	rowNumber++;
	reportSheet.cell(`H${rowNumber}`).value(generalConsts.endTemperature);
	reportSheet.cell(`I${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`J${rowNumber}`).value('°C');

	return rowNumber + 3;
}

const addSummary = (summaryObj, rowNumber, reportSheet) => {
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.summary).style("bold", true);
	
	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.lastDischarges);
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`F${rowNumber}`).value('Ah');

	rowNumber++;
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`F${rowNumber}`).value('Wh');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.fromRated);
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`F${rowNumber}`).value('%');

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.maxTermperature);
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!
	reportSheet.cell(`F${rowNumber}`).value('°C');

	rowNumber+=2;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.conductedBy);
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!

	rowNumber++;
	reportSheet.cell(`D${rowNumber}`).value(generalConsts.approvedBy);
	reportSheet.cell(`E${rowNumber}`).value('TODO');//TODO!!!
}

module.exports = {
	downloadReportsGraph,
};