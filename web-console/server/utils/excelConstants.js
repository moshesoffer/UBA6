const generalConsts = {
	excelTemplateFilePath: '../assets/excel_template.xlsx',
	DataSheetName: 'Data',
	reportSheetName: 'Report',

	//----START STEPS----
	//CHARGE
	charge: 'CHARGE',
	source: 'Source',
	chargeCurrent: 'Charge Current',
	chargePerCell: 'Charge Per Cell',
	maxTermperature: 'Max Temperature',
	maxTime: 'Max Time',
	cutOffCurrent: 'Cut-off Current',
	chargeLimit: 'Charge Limit',
	//charge Results
	chargeCapacity: 'Charge Capacity',
	chargeEnergy: 'Charge Energy',
	duration: 'Duration',
	initialVoltage: 'Initial Voltage',
	endVoltage: 'End Voltage',
	endTemperature: 'End Temperature',

	//DELAY
	delay: 'DELAY',
	time: 'Time',
	toTemperature: 'To Temperature',

	//DISCHARGE
	disCharge: 'DISCHARGE',
	//Source, Max Temperature, Max Time, Discharge Limit
	disChargeCurrent: 'Discharge Current',
	minTemperature: 'Min Temperature',
	cutOffVoltage: 'Cut-off Voltage',
	disChargeLimit: 'DisCharge Limit',
	//discharge Results
	dischargeCapacity: 'Discharge Capacity',
	dischargeEnergy: 'Discharge Energy',
	midPointVoltage: 'Mid. Point Voltage',
	midPointCurrent: 'Mid. Point Current',
	//----END STEPS----

	//----SUMMARY PART----
	summary: 'SUMMARY',
	lastDischarges: 'Last Discharges:',
	fromRated: '% From rated',
	conductedBy: 'Conducted by:',
	approvedBy: 'Approved by:',
};

const cells = {
	lastDischarges1: 'G3',
	lastDischarges2: 'I3',
	reportName: 'E3',

	//Test Data:
	startTime: 'E29',
	ubaSN: 'E31',
	ubaChannel: 'E32',
	customer: 'E36',
	workOrderNo: 'E37',

	//Battery Rated data:
	batteryPN: 'I29',
	batterySN: 'I30',
	chemistry: 'I31',
	noCellsInSerial: 'I32',
	noCellsInParallel: 'I33',
	ratedCapacity: 'I34',
	cellPN: 'I35',
	cellBatchDateCode: 'I36',
	cellSupplier: 'I37',
};

module.exports = {
	generalConsts,
	cells
};
