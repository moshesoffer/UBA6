const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { ubaDeviceModel, runningTestsModel, instantTestResultsModel } = require('../models');
const { clearMemInServer } = require('./testHelper');
const { APIS } = require('../utils/constants');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Machine API Tests', () => {
    const machineToAdd = {
        mac: '00-10-FA-63-38-4A',
        name: 'Lab-1',
        ip: '141.191.237.16',
    };
    const ubaDeviceToAdd = {
        ubaSN: "14566",
        name: "uba6-_1-5",
        ubaChannel: "AB",
        machineMac: machineToAdd.mac,
        comPort: "COM7",
        address: "967896",
        fwVersion: "7.5.3.0",
        hwVersion: "5.2",
    };
    let connection;

    beforeAll(async () => {
        await withTimeout(runSchema(), AWAIT_TIMEOUT);
        connection = await withTimeout(mysql.createConnection(global.__MYSQL_CONFIG__), AWAIT_TIMEOUT);

        let response = await withTimeout(request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);

        response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(ubaDeviceToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);
        expect(response.body.length).toBe(2);//2 ids of running tests

        const [ubaRows] = await withTimeout(connection.query(`SELECT * FROM \`${ubaDeviceModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(ubaRows.length).toBe(1);
        const [runningTestsRows] = await withTimeout(connection.query(`SELECT * FROM \`${runningTestsModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(runningTestsRows.length).toBe(2);
        const [instantTestResultsRows] = await withTimeout(connection.query(`SELECT * FROM \`${instantTestResultsModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(instantTestResultsRows.length).toBe(0);
        
    });

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await withTimeout(connection.end(), AWAIT_TIMEOUT);
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });
    afterEach(async () => {
        await withTimeout(clearMemInServer(), AWAIT_TIMEOUT);
    });

    test('add a new ubaDevice - fail', async () => {
        const { ubaChannel, ...ubaDeviceWithoutUbaChannel } = ubaDeviceToAdd;
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(ubaDeviceWithoutUbaChannel), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//missing ubaChannel
    });

    test('update a ubaDevice - happy flow', async () => {
        const newObj = {
            //machineMac: '',
            name: 'uba6_1-2',
            comPort: 'COM5',
            address: '123456',
            ignoredParam: 'something',
        };
        let response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN)
            .send(newObj), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        response = await withTimeout(request(global.__SERVER__).get(APIS.ubaDevicesApi), AWAIT_TIMEOUT);
        
        const ubaDevices = response.body.ubaDevices;
        const ubaTotal = response.body.ubaTotal;
        expect(ubaDevices[0].comPort).toBe(newObj.comPort);
        expect(ubaDevices[1].comPort).toBe(newObj.comPort);
        expect(ubaDevices[0].name).toBe(newObj.name);
        expect(ubaDevices[1].name).toBe(newObj.name);
        expect(ubaDevices[0].address).toBe(newObj.address);
        expect(ubaDevices[1].address).toBe(newObj.address);
        expect(ubaDevices[0].createdTime).toBeTruthy();
        expect(ubaDevices[1].createdTime).toBeTruthy();
        expect(ubaDevices[0].modifiedTime).toBeTruthy();
        expect(ubaDevices[1].modifiedTime).toBeTruthy();
        expect(ubaDevices[0]).not.toHaveProperty('ignoredParam');
        expect(ubaDevices[1]).not.toHaveProperty('ignoredParam');

        expect(ubaTotal.configured).toBe(1);
        expect(ubaTotal.connected).toBe(0);
        expect(ubaTotal.running).toBe(0);
    });

    test('update a ubaDevice - fail', async () => {
        let response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN)
            .send({
                notExist: 'notExist',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//nothing to update
        
        response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.ubaDevicesApi + "/" + 'notexist')
            .send({
                name: 'new',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//ubaDevice not exists
    });

    test('delete a ubaDevice - happy flow', async () => {
        let response = await withTimeout(request(global.__SERVER__)
            .delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN), AWAIT_TIMEOUT);
        expect(response.status).toBe(204);
        response = await withTimeout(request(global.__SERVER__).get(APIS.ubaDevicesApi), AWAIT_TIMEOUT);
        const ubaDevices = response.body.ubaDevices;
        const ubaTotal = response.body.ubaTotal;
        expect(ubaDevices.length).toBe(0);
        expect(ubaTotal.configured).toBe(0);
        expect(ubaTotal.connected).toBe(0);
        expect(ubaTotal.running).toBe(0);

        const [ubaRows] = await withTimeout(connection.query(`SELECT * FROM \`${ubaDeviceModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(ubaRows.length).toBe(0);
        const [runningTestsRows] = await withTimeout(connection.query(`SELECT * FROM \`${runningTestsModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(runningTestsRows.length).toBe(0);
    });

    test('delete a ubaDevice - fail', async () => {
        let response = await withTimeout(request(global.__SERVER__)
            .delete(APIS.ubaDevicesApi + "/" + 'notexist'), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//ubaDevice not exists
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        const ubaDeviceToAdd2 = {
            ubaSN: "232323",
            name: "uba3-_1-4",
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM4",
            address: "111",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        for (const field of ubaDeviceModel.createProperties) {
            if (field === 'ubaChannel') continue; // nullable fields
            const ubaCopy = { ...ubaDeviceToAdd2 };
            delete ubaCopy[field];
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.ubaDevicesApi)
                .send(ubaCopy), AWAIT_TIMEOUT);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(300);
        const tooLong = {
            ubaSN: "232323",
            name: longString,
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: longString,
            address: longString,
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(tooLong), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        const ubaDeviceToAdd3 = {
            ubaSN: "112233",
            name: "uba3-_2-4",
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM1",
            address: "222",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        await withTimeout(request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd3), AWAIT_TIMEOUT);
        const response = await withTimeout(request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd3), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-invalid-data-types
    test('co-pilot-invalid-data-types', async () => {
        const invalidUBA = {
            ubaSN: "2344",
            name: 123,
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM123",
            address: "3434",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(invalidUBA), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-foreign-key-constraint
    test('co-pilot-foreign-key-constraint', async () => {
        const invalidUBA = {
            ubaSN: "234",
            name: "uba3-_3333-4",
            ubaChannel: "AB",
            machineMac: 'notexist',
            comPort: "COM3",
            address: "3434",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(invalidUBA), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-extra-unknown-fields
    test('co-pilot-extra-unknown-fields', async () => {
        const ubaWithExtra = {
            ubaSN: "233",
            name: "uba3-_33-4",
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM34",
            address: "222",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
            extraField: 'shouldBeIgnored'
        };

        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(ubaWithExtra), AWAIT_TIMEOUT);
        expect([201]).toContain(response.status);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send({}), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-get-nonexistent-ubaDevice
    test('co-pilot-get-nonexistent-ubaDevice', async () => {
        const response = await withTimeout(request(global.__SERVER__).get(APIS.ubaDevicesApi + '/notexist'), AWAIT_TIMEOUT);
        expect([301, 404, 500]).toContain(response.status);
    });

});