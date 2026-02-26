const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { machineModel } = require('../models');
const { APIS } = require('../utils/constants');
const { clearMemInServer } = require('./testHelper');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Machine API Tests', () => {
    const machineToAdd = {
        mac: '00-10-FA-63-38-4A',
        name: 'Lab-1',
        ip: '141.191.237.16',
        ignoredParam: 'something',
    };
    let connection;

    beforeAll(async () => {
        await withTimeout( runSchema(), AWAIT_TIMEOUT);
        connection = await withTimeout( mysql.createConnection(global.__MYSQL_CONFIG__), AWAIT_TIMEOUT);

        let response = await withTimeout( request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);
        const [rows] = await withTimeout( connection.query(`SELECT * FROM \`${machineModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(rows.length).toBe(1);

        response = await withTimeout( request(global.__SERVER__).get(APIS.machinesApi), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        expect(response.body.length).toBe(1);
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
        machineModel.createProperties.forEach(prop => {
            expect(response.body[0]).toHaveProperty(prop);
        });
    });

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await withTimeout( connection.end(), AWAIT_TIMEOUT);
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });
    afterEach(async () => {
        await withTimeout( clearMemInServer(), AWAIT_TIMEOUT);
    });

    test('add a new machine - fail', async () => {
        const { ip, ...machineWithoutIp } = machineToAdd;
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineWithoutIp), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//missing ip
    });

    test('update a machine - happy flow', async () => {
        const newObj = {
            //mac: '00-10-FA-63-38-4A',
            name: 'Lab-2',
            ip: '142.191.237.11',
            ignoredParam: 'something',
        };
        let response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.machinesApi + "/" + machineToAdd.mac)
            .send(newObj), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        response = await withTimeout( request(global.__SERVER__).get(APIS.machinesApi), AWAIT_TIMEOUT);
        
        expect(response.body[0].ip).toBe(newObj.ip);
        expect(response.body[0].name).toBe(newObj.name);
        expect(response.body[0].createdTime).toBeTruthy();
        expect(response.body[0].modifiedTime).toBeTruthy();
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
    });

    test('update a machine - fail', async () => {
        let response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.machinesApi + "/" + machineToAdd.mac)
            .send({
                notExist: 'notExist',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//nothing to update
        
        response = await withTimeout( request(global.__SERVER__)
            .patch(APIS.machinesApi + "/" + 'notexist')
            .send({
                ip: '111.222.333.44',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//machine not exists
    });

    test('delete a machine - happy flow', async () => {

        //adding uba in order to check that cant delete if machine has ubas
        const ubaDeviceToAdd = {
            ubaSN: "3478",
            name: "uba4-_3-2",
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM667",
            address: "5665",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        let response = await withTimeout( request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);

        response = await withTimeout( request(global.__SERVER__).delete(APIS.machinesApi + "/" + machineToAdd.mac), AWAIT_TIMEOUT);
        expect(response.status).toBe(400);
        expect(response.body.error).toBe("Machine has 1 uba devices, can't delete.");

        //delete uba in order to delete machine
        response = await withTimeout( request(global.__SERVER__).delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN), AWAIT_TIMEOUT);
        expect(response.status).toBe(204);

        response = await withTimeout( request(global.__SERVER__).delete(APIS.machinesApi + "/" + machineToAdd.mac), AWAIT_TIMEOUT);
        expect(response.status).toBe(204);

        response = await withTimeout( request(global.__SERVER__).get(APIS.machinesApi), AWAIT_TIMEOUT);
        expect(response.body.length).toBe(0);
    });

    test('delete a machine - fail', async () => {
        let response = await withTimeout( request(global.__SERVER__)
            .delete(APIS.machinesApi + "/" + 'notexist'), AWAIT_TIMEOUT);
        expect(response.status).toBe(400);//machine not exists
    });

    // co-pilot-missing-required-fields
    test('co-pilot-missing-required-fields', async () => {
        for (const field of machineModel.createProperties) {
            const machineCopy = { ...machineToAdd };
            delete machineCopy[field];
            const response = await withTimeout( request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send(machineCopy), AWAIT_TIMEOUT);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-extra-unknown-fields
    test('co-pilot-extra-unknown-fields', async () => {
        const machineWithExtra = { ...machineToAdd, extraField: 'shouldBeIgnored', mac: '00-10-FA-63-38-new', };
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineWithExtra), AWAIT_TIMEOUT);
        expect([201]).toContain(response.status);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send({}), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-boundary-values
    test('co-pilot-boundary-values', async () => {
        const minMachine = { ...machineToAdd, name: '', ip: '', mac: '00-10-FA-63-38-new2', };//name is required
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(minMachine), AWAIT_TIMEOUT);
        expect([500]).toContain(response.status);
    });

    // co-pilot-duplicate-primary-key
    test('co-pilot-duplicate-primary-key', async () => {
        await withTimeout( request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd), AWAIT_TIMEOUT);
        const response = await withTimeout( request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        // All fields are NOT NULL, so try omitting each
        for (const field of machineModel.createProperties) {
            const machineCopy = { ...machineToAdd };
            delete machineCopy[field];
            const response = await withTimeout( request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send(machineCopy), AWAIT_TIMEOUT);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(1000); // Exceeds varchar(64)
        const tooLong = { ...machineToAdd, name: longString, ip: longString };
        const response = await withTimeout( request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(tooLong), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        await withTimeout( request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd), AWAIT_TIMEOUT);
        const response = await withTimeout( request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

});