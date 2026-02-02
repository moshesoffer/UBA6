
const cellModel = {
    uuid: false,
    tableName: `CellPartNumbers`,
    selectAllQuery: `SELECT * FROM \`CellPartNumbers\`;`,
    pkName: `itemPN`,
    createProperties: [
        'itemPN',
        'chemistry',
        'manufacturer',
        'minVoltage',
        'nomVoltage',
        'maxVoltage',
        'minCapacity',
        'nomCapacity',
        'minTemp',
        'maxTemp',
        'chargeOption',
    ],
    get updateProperties() {
        return this.createProperties.filter(prop => prop !== this.pkName);
    }
};

const machineModel = {
    uuid: false,
    tableName: `Machines`,
    selectAllQuery: `SELECT * FROM \`Machines\`;`,
    pkName: `mac`,
    createProperties: [
        'mac',
        'name',
        'ip',
    ],
    get updateProperties() {
        return this.createProperties.filter(prop => prop !== this.pkName);
    }
};

const testRoutineModel = {
    uuid: true,
    tableName: `TestRoutines`,
    selectAllQuery: `SELECT t.*, c.* FROM \`TestRoutines\` AS t JOIN \`${cellModel.tableName}\` AS c ON c.\`itemPN\` = t.\`cellPN\`;`,
    pkName: `id`,
    createProperties: [
        'testName',
        'isLocked',
        'batteryPN',
        'batterySN',
        'cellPN',
        'noCellSerial',
        'noCellParallel',
        'maxPerBattery',
        'ratedBatteryCapacity',
        'channel',
        'notes',
        'customer',
        'workOrderNumber',
        'approvedBy',
        'conductedBy',
        'cellSupplier',
        'cellBatch',
        'plan',
    ],
    get updateProperties() {
        return this.createProperties;
    },
    planProperties: [
        'id',
        'type',
        'isCollapsed',
        'source',
        'isMinTemp',
        'minTemp',
        'isMaxTemp',
        'maxTemp',
        'isMaxTime',
        'maxTime',
        'delayTime',
        'isChargeLimit',
        'chargeLimit',
        'isDischargeLimit',
        'dischargeLimit',
        'chargeCurrent',
        'dischargeCurrent',
        'cRate',
        'isCutOffCurrent',
        'cutOffCurrent',
        'isCutOffVoltage',
        'cutOffVoltage',
        'chargePerCell',
        'waitTemp',
        'goToStep',
        'repeatStep',
    ]
}

const reportModel = {
    uuid: false,
    tableName: `Reports`,
    pkName: `id`,
    get createProperties() {
        const excludedFields = ['cellPN', 'isLocked'];
        const additionalFields = ['id', 'testRoutineChannels', 'machineName', 'timeOfTest', 'status', 'timestampStart', 'ubaSN'];
        return testRoutineModel.createProperties
            .filter(field => !excludedFields.includes(field))
            .concat(additionalFields);
    },
    updateProperties: [
        'testName',
        'batteryPN',
        'batterySN',
        'notes',
        'customer',
        'workOrderNumber',
        'approvedBy',
        'conductedBy',
        'cellSupplier',
        'cellBatch',
    ],
};

const reportsDataModel = {
    uuid: false,
    tableName: `ReportsData`,
    pkName: `reportID`,
    createProperties: [
        'reportID',
        'testResults',
    ],
    updateProperties: [
        'testResults',
    ],
    testResultsProperties: [
        'timestamp',
        'voltage',
        'temperature',
        'current',
        //'currentStep',
    ],
};

const runningTestsModel = {
    uuid: true,
    tableName: `RunningTests`,
    pkName: `id`,
    get createProperties() {
        return testRoutineModel.createProperties.filter(field => field !== 'cellPN' && field !== 'isLocked' && field !== 'channel');
    },
    updateProperties: [
        'status'
    ],
};

const ubaDeviceModel = {
    uuid: false,
    tableName: `UBADevices`,
    pkName: `ubaSN`,
    selectAllQuery: `
	SELECT 
		d.*,
		m.\`name\` AS \`machineName\`, 
		t.\`id\` AS \`runningTestID\`, t.\`testName\`, t.\`channel\`, t.\`timestampStart\`, t.\`status\`, t.\`plan\`,
		t.\`batteryPN\`, t.\`batterySN\`, t.\`testRoutineChannels\`
	FROM \`UBADevices\` AS d
	JOIN \`Machines\` AS m ON m.\`mac\` = d.\`machineMac\`
	JOIN \`RunningTests\` AS t ON t.\`ubaSN\` = d.\`ubaSN\`;
	`,
    createProperties: ['ubaSN', 'ubaChannel', 'machineMac', 'name', 'comPort', 'address',],
    updateProperties: ['machineMac', 'name', 'comPort', 'address',],
};

const instantTestResultsModel = {
    uuid: true,
    tableName: `InstantTestResults`,
    pkName: `id`,
    selectAllQuery: `
        SELECT main.*
        FROM \`InstantTestResults\` AS main
        JOIN (
            SELECT \`runningTestID\`, MAX(\`timestamp\`) AS \`latest\`
            FROM \`InstantTestResults\`
            GROUP BY \`runningTestID\`
        ) AS auxiliary ON auxiliary.\`runningTestID\` = main.\`runningTestID\` AND auxiliary.\`latest\` = main.\`timestamp\`;
        `,
    createProperties: ['runningTestID', 'timestamp', 'testState', 'testCurrentStep', 'voltage', 'current', 'temp', 'capacity', 'error',],
};

module.exports = {
	cellModel,
    machineModel,
    testRoutineModel,
    reportModel,
    instantTestResultsModel,
    reportsDataModel,
    runningTestsModel,
    ubaDeviceModel,
};