/**
 * Each Model has createProperties and updateProperties.
 * createProperties are the properties that are used to create a new record in the database.
 * updateProperties are the properties that are used to update an existing record in the database.
 * any property that isnt in createProperties or updateProperties will not be used in the database operations.
 * pkName is the primary key of the table.
 * if uuid is true then when creating a new record the uuid will be generated automatically.
 * schema is the Joi schema that is used to validate the data before inserting or updating it in the database.
 */
const Joi = require('joi');

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
    },
    schema: {
        itemPN: Joi.string().max(64).required(),
        chemistry: Joi.string().max(64).required(),
        manufacturer: Joi.string().max(64).required(),
        minVoltage: Joi.number().required(),
        nomVoltage: Joi.number().required(),
        maxVoltage: Joi.number().required(),
        minCapacity: Joi.number().required(),
        nomCapacity: Joi.number().required(),
        minTemp: Joi.number().required(),
        maxTemp: Joi.number().required(),
        chargeOption: Joi.string().max(64).required(),
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
    },
    schema: {
        mac: Joi.string().max(64).required(),
        name: Joi.string().max(64).required(),
        ip: Joi.string().max(64).required(),
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
        'batteryPN',//in report and testroutine and in runningTests
        'batterySN',//in report and testroutine and in runningTests
        'cellPN',//foreign key to itemPN in CellPartNumbers and also in report table but without FK also in runningTests without FK
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
    ],
    planSchema: {//currently allowing null for all fields, need to talk with Or which are mandatory in each type
        id: Joi.number().allow(null),
        type: Joi.string().allow(null),
        isCollapsed: Joi.boolean().allow(null),
        source: Joi.string().allow(null),
        isMinTemp: Joi.boolean().allow(null),
        minTemp: Joi.number().allow(null),
        isMaxTemp: Joi.boolean().allow(null),
        maxTemp: Joi.number().allow(null),
        isMaxTime: Joi.boolean().allow(null),
        maxTime: Joi.string().allow(null),
        delayTime: Joi.string().allow(null),
        isChargeLimit: Joi.boolean().allow(null),
        chargeLimit: Joi.string().allow(null),
        isDischargeLimit: Joi.boolean().allow(null),
        dischargeLimit: Joi.string().allow(null),
        chargeCurrent: Joi.string().allow(null),
        dischargeCurrent: Joi.string().allow(null),
        cRate: Joi.number().allow(null),
        isCutOffCurrent: Joi.boolean().allow(null),
        cutOffCurrent: Joi.string().allow(null),
        isCutOffVoltage: Joi.boolean().allow(null),
        cutOffVoltage: Joi.number().allow(null),
        chargePerCell: Joi.number().allow(null),
        waitTemp: Joi.number().allow(null),
        goToStep: Joi.number().integer().allow(null),
        repeatStep: Joi.number().integer().allow(null),
    },
    schema: {
        cellPN: Joi.string().max(36).required(),
        testName: Joi.string().max(64).required(),
        isLocked: Joi.number().integer().min(0).max(1),
        channel: Joi.string().max(64).required(),
        batteryPN: Joi.string().max(256).required(),
        batterySN: Joi.string().max(256).required(),
        noCellSerial: Joi.number().integer().required(),
        noCellParallel: Joi.number().integer().required(),
        maxPerBattery: Joi.number().required(),
        ratedBatteryCapacity: Joi.number().required(),
        notes: Joi.string().max(256).allow(null, ''),
        customer: Joi.string().max(64).allow(null, ''),
        workOrderNumber: Joi.string().max(64).allow(null, ''),
        approvedBy: Joi.string().max(64).allow(null, ''),
        conductedBy: Joi.string().max(64).allow(null, ''),
        cellSupplier: Joi.string().max(64).allow(null, ''),
        cellBatch: Joi.string().max(64).allow(null, ''),
        plan: Joi.any().allow(null),//validatePlan is validating it. TODO Add also schema validation in validatePlan
    }
}

const reportModel = {
    uuid: false,
    tableName: `Reports`,
    pkName: `id`,
    get createProperties() {
        const excludedFields = ['isLocked'];
        const additionalFields = ['id', 'testRoutineChannels', 'machineName', 'machineMac', 'timeOfTest', 'chemistry', 'status', 'timestampStart', 'ubaSN'];
        return testRoutineModel.createProperties
            .filter(field => !excludedFields.includes(field))
            .concat(additionalFields);
    },
    updateProperties: [
        'testName',
        'batteryPN',//in report and testroutine and in runningTests
        'batterySN',//in report and testroutine and in runningTests
        'cellPN',//in report and testroutine and in runningTests
        'notes',
        'customer',
        'workOrderNumber',
        'approvedBy',
        'conductedBy',
        'cellSupplier',
        'cellBatch',
        'status',
        'timeOfTest',
    ],
    schema: {
        ubaSN: Joi.string().max(64).required(),
        channel: Joi.string().max(64).required(),
        timestampStart: Joi.date().iso().required(),
        status: Joi.number().integer().min(0).required(),
        testName: Joi.string().max(64).required(),
        batteryPN: Joi.string().max(256).required(),
        batterySN: Joi.string().max(256).required(),
        cellPN: Joi.string().max(36).required(),
        noCellSerial: Joi.number().integer().required(),
        noCellParallel: Joi.number().integer().required(),
        maxPerBattery: Joi.number().required(),
        ratedBatteryCapacity: Joi.number().required(),
        notes: Joi.string().max(256).allow(null, ''),
        customer: Joi.string().max(64).allow(null, ''),
        workOrderNumber: Joi.string().max(64).allow(null, ''),
        approvedBy: Joi.string().max(64).allow(null, ''),
        conductedBy: Joi.string().max(64).allow(null, ''),
        cellSupplier: Joi.string().max(64).allow(null, ''),
        cellBatch: Joi.string().max(64).allow(null, ''),
        plan: Joi.any().required(), // plan is JSON, can be further validated if needed
        testRoutineChannels: Joi.string().max(64).required(),
        machineName: Joi.string().max(64).allow(null, ''),
        machineMac: Joi.string().max(64).allow(null, ''),
        timeOfTest: Joi.string().max(64).allow(null, ''),
    },
};

const runningTestsModel = {
    uuid: true,
    tableName: `RunningTests`,
    pkName: `id`,
    get createProperties() {
        return testRoutineModel.createProperties.filter(field => field !== 'isLocked' && field !== 'channel');
    },
    updateProperties: [
        'status'
    ],
    schema: {
        ubaSN: Joi.string().max(64).required(),
        channel: Joi.string().max(64).required(),
        timestampStart: Joi.date().iso().required(),
        status: Joi.number().integer().min(0).required(),
        testName: Joi.string().max(64).allow(null, ''),
        batteryPN: Joi.string().max(256).allow(null, ''),
        batterySN: Joi.string().max(256).allow(null, ''),
        cellPN: Joi.string().max(36).allow(null, ''),
        noCellSerial: Joi.number().integer().allow(null),
        noCellParallel: Joi.number().integer().allow(null),
        maxPerBattery: Joi.number().allow(null),
        ratedBatteryCapacity: Joi.number().allow(null),
        notes: Joi.string().max(256).allow(null, ''),
        customer: Joi.string().max(64).allow(null, ''),
        workOrderNumber: Joi.string().max(64).allow(null, ''),
        approvedBy: Joi.string().max(64).allow(null, ''),
        conductedBy: Joi.string().max(64).allow(null, ''),
        cellSupplier: Joi.string().max(64).allow(null, ''),
        cellBatch: Joi.string().max(64).allow(null, ''),
        plan: Joi.any().allow(null), // plan is JSON, TODO Add also schema validation
        testRoutineChannels: Joi.string().max(64).allow(null, ''),
    },
};

//UBADevice-RunningTest is 1 to many. RunningTests has unique constraint on ubaSN+channel
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
    createProperties: ['ubaSN', 'ubaChannel', 'machineMac', 'name', 'comPort', 'address', 'fwVersion', 'hwVersion'],
    updateProperties: ['machineMac', 'name', 'comPort', 'address', 'fwVersion', 'hwVersion'],
    schema: {
        ubaSN: Joi.string().max(64).required(),
        machineMac: Joi.string().max(64).required(),
        name: Joi.string().max(64).required(),
        address: Joi.string().max(64).required(),
        comPort: Joi.string().max(64).required(),
        ubaChannel: Joi.string().max(64).allow(null, ''),
        fwVersion: Joi.string().max(32).required(),
        hwVersion: Joi.string().max(16).required(),
    },
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
    schema: {
        runningTestID: Joi.string().length(36).required(),
        timestamp: Joi.date().iso().required(),
        testState: Joi.string().max(64).required(),
        testCurrentStep: Joi.number().integer().min(0).required(),
        voltage: Joi.number().required(),
        current: Joi.number().required(),
        temp: Joi.number().required(),
        capacity: Joi.number().required(),
        error: Joi.number().integer(),
    },
};

module.exports = {
	cellModel,
    machineModel,
    testRoutineModel,
    reportModel,
    instantTestResultsModel,
    runningTestsModel,
    ubaDeviceModel,
};