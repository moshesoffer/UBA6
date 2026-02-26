const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { testRoutineModel } = require('../models');
const { TEST_ROUTINE_CHANNELS, APIS } = require('../utils/constants');
const { clearMemInServer } = require('./testHelper');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('TestRoutines API Tests', () => {

    let connection;
    const cellToAdd = {
        chemistry: 'Li-Ion Polymer',
        manufacturer: 'Amicell',
        itemPN: 'ABLP75100250H300',
        minVoltage: '2.7500',
        nomVoltage: '3.7000',
        maxVoltage: '4.3000',
        minCapacity: '29400.00000',
        nomCapacity: '30000.00000',
        minTemp: '-20.0000',
        maxTemp: '60.0000',
        chargeOption: 'Primary',
        ignoredParam: 'something',
    };
    const testRoutineToAdd = {
        testName: 'C-D-C_3A_CUT OFF 2.5V',
        isLocked: 0,
        batteryPN: '901-24000365-1S2P-M1',
        batterySN: 'def5346536g',
        cellPN: cellToAdd.itemPN,
        noCellSerial: 1,
        noCellParallel:2 ,
        maxPerBattery: 8.7453,
        ratedBatteryCapacity: 4080.00000,
        channel: 'A-and-B',
        notes: 'notes',
        customer: 'customer 0',
        workOrderNumber: 'fd',
        approvedBy: 'approvedBy',
        conductedBy: 'conductedBy',
        cellSupplier: 'cellSupplier',
        cellBatch: '',
        //cRate is string number and is accepted, server will convert it to number
        plan: [{"id": 0, "type": "charge", "cRate": "0.74", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -201, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": "666:absoluteMah", "isCollapsed": false, "chargeCurrent": "3000:absoluteMa", "chargePerCell": 4, "cutOffCurrent": "50:absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 1, "type": "discharge", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -20, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": 2.5, "isChargeLimit": true, "dischargeLimit": "7:absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": "3:absoluteMa", "isDischargeLimit": true}, {"id": 2, "type": "delay", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": null, "waitTemp": 7, "delayTime": "01:01:01", "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 3, "type": "loop", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": 0, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": 4, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}],
        ignoredParam: 'something',
    };

    beforeAll(async () => {
        await withTimeout( runSchema(), AWAIT_TIMEOUT);
        connection = await withTimeout( mysql.createConnection(global.__MYSQL_CONFIG__), AWAIT_TIMEOUT);
        let response = await withTimeout( request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);

        response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send(testRoutineToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);
        const [rows] = await withTimeout( connection.query(`SELECT * FROM \`${testRoutineModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(rows.length).toBe(1);

        response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        expect(response.body.length).toBe(1);
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
        testRoutineModel.createProperties.forEach(prop => {
            expect(response.body[0]).toHaveProperty(prop);
        });
    });

    afterAll(async () => {
        console.log('TestRoutines Test suite finished');
        if(connection) await withTimeout( connection.end(), AWAIT_TIMEOUT);
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });
    afterEach(async () => {
        await withTimeout( clearMemInServer(), AWAIT_TIMEOUT);
    });

    test('add a new TestRoutine - fail', async () => {
        const { batterySN, ...trWithoutBatterySN } = testRoutineToAdd;
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send(trWithoutBatterySN), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//missing batterySN
        //TODO! unique test name and fk cellPN
    });

    test('update a TestRoutine - happy flow', async () => {
        let response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi), AWAIT_TIMEOUT);
        const id = response.body[0].id;
        const newObj = {
            testName: 'new',
            isLocked: 1,
            batteryPN: 'new',
            batterySN: 'new',
            //cellPN: cellToAdd.itemPN,
            noCellSerial: 1,
            noCellParallel:1 ,
            maxPerBattery: '1.0000',
            ratedBatteryCapacity: '1.00000',
            channel: TEST_ROUTINE_CHANNELS.A_OR_B,
            notes: 'new',
            customer: 'new',
            workOrderNumber: 'new',
            approvedBy: 'new',
            conductedBy: 'new',
            cellSupplier: 'new',
            cellBatch: 'new',
            //plan: [{"id": 0, "type": "charge", "cRate": 0.74, "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -201, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": "666:absoluteMah", "isCollapsed": false, "chargeCurrent": "3000:absoluteMa", "chargePerCell": 4, "cutOffCurrent": "50:absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 1, "type": "discharge", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -20, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": 2.5, "isChargeLimit": true, "dischargeLimit": "7:absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": "3:absoluteMa", "isDischargeLimit": true}, {"id": 2, "type": "delay", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": null, "waitTemp": 7, "delayTime": "01:01:01", "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 3, "type": "loop", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": 0, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": 4, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}],
            ignoredParam: 'something',
        };
        response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.testRoutinesApi + "/" + id)
            .send(newObj), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi), AWAIT_TIMEOUT);
        
        newObj.id = id;
        newObj.cellPN = cellToAdd.itemPN;
        Object.assign(newObj, cellToAdd);
        newObj.plan = testRoutineToAdd.plan;
        delete newObj.ignoredParam;
        newObj.maxPerBattery = Number(newObj.maxPerBattery);
        newObj.ratedBatteryCapacity = Number(newObj.ratedBatteryCapacity);
        newObj.nomVoltage = Number(newObj.nomVoltage);
        newObj.maxVoltage = Number(newObj.maxVoltage);
        newObj.minCapacity = Number(newObj.minCapacity);
        newObj.nomCapacity = Number(newObj.nomCapacity);
        newObj.minTemp = Number(newObj.minTemp);
        newObj.maxTemp = Number(newObj.maxTemp);
        newObj.minVoltage = Number(newObj.minVoltage);
        newObj.plan[0].cRate = 0.74;//this will verify that string number was converted to a number
        const { createdTime, modifiedTime, ...restExpected } = response.body[0];
        expect(restExpected).toEqual(newObj);
    });

    test('update a TestRoutine - fail', async () => {
        let response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi), AWAIT_TIMEOUT);
        const id = response.body[0].id;
        response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.testRoutinesApi + "/" + id)
            .send({
                notExist: 'notExist',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//nothing to update
        
        response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.testRoutinesApi + "/" + 'notexist')
            .send({
                cellBatch: 'something',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//TestRoutine not exists

        //cRate is not a number
        const newObj = {
            plan: [{"id": 0, "type": "charge", "cRate": "asd", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -201, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": "666:absoluteMah", "isCollapsed": false, "chargeCurrent": "3000:absoluteMa", "chargePerCell": 4, "cutOffCurrent": "50:absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 1, "type": "discharge", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -20, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": 2.5, "isChargeLimit": true, "dischargeLimit": "7:absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": "3:absoluteMa", "isDischargeLimit": true}, {"id": 2, "type": "delay", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": null, "waitTemp": 7, "delayTime": "01:01:01", "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 3, "type": "loop", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": 0, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": 4, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}],
        };
        response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.testRoutinesApi + "/" + id)
            .send(newObj), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    test('delete a TestRoutine - happy flow', async () => {
        let response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi), AWAIT_TIMEOUT);
        const id = response.body[0].id;
        response = await withTimeout( request(global.__SERVER__).delete(APIS.testRoutinesApi + "/" + id), AWAIT_TIMEOUT);
        expect(response.status).toBe(204);
        response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi), AWAIT_TIMEOUT);
        expect(response.body.length).toBe(0);
    });

    test('delete a TestRoutine - fail', async () => {
        let response = await withTimeout( request(global.__SERVER__).delete(APIS.testRoutinesApi + "/" + 'notexist'), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//TestRoutine not exists
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        const trCopy = { ...testRoutineToAdd };
        delete trCopy['testName'];
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send(trCopy), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(300);
        const tooLong = { ...testRoutineToAdd, testName: longString, batteryPN: longString, batterySN: longString };
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send(tooLong), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        await withTimeout( request(global.__SERVER__).post(APIS.testRoutinesApi).send(testRoutineToAdd), AWAIT_TIMEOUT);
        const response = await withTimeout( request(global.__SERVER__).post(APIS.testRoutinesApi).send(testRoutineToAdd), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-invalid-data-types
    test('co-pilot-invalid-data-types', async () => {
        const invalidTR = { ...testRoutineToAdd, noCellSerial: 'not-a-number', noCellParallel: 'not-a-number' };
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send(invalidTR), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-foreign-key-constraint
    test('co-pilot-foreign-key-constraint', async () => {
        const invalidTR = { ...testRoutineToAdd, cellPN: 'notexist' };
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send(invalidTR), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.testRoutinesApi)
            .send({}), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-get-nonexistent-testRoutine
    test('co-pilot-get-nonexistent-testRoutine', async () => {
        const response = await withTimeout( request(global.__SERVER__).get(APIS.testRoutinesApi + '/notexists'), AWAIT_TIMEOUT);
        expect([301, 404, 500]).toContain(response.status);
    });
});