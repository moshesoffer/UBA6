const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { machineModel } = require('../models');
const { APIS } = require('../utils/constants');
const { clearMemInServer } = require('./testHelper');

describe('Machine API Tests', () => {
    const machineToAdd = {
        mac: '00-10-FA-63-38-4A',
        name: 'Lab-1',
        ip: '141.191.237.16',
        ignoredParam: 'something',
    };
    let connection;

    beforeAll(async () => {
        await runSchema();
        connection = await mysql.createConnection(global.__MYSQL_CONFIG__);

        let response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineToAdd);
        expect(response.status).toBe(201);
        const [rows] = await connection.query(`SELECT * FROM \`${machineModel.tableName}\`;`);
        expect(rows.length).toBe(1);

        response = await request(global.__SERVER__).get(APIS.machinesApi);
        expect(response.status).toBe(200);
        expect(response.body.length).toBe(1);
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
        machineModel.createProperties.forEach(prop => {
            expect(response.body[0]).toHaveProperty(prop);
        });
    });

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));//waiting for winston server logs to finish
    });
    afterEach(async () => {
        await clearMemInServer();
    });

    test('add a new machine - fail', async () => {
        const { ip, ...machineWithoutIp } = machineToAdd;
        const response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineWithoutIp);
        expect(response.status).toBe(500);//missing ip
    });

    test('update a machine - happy flow', async () => {
        const newObj = {
            //mac: '00-10-FA-63-38-4A',
            name: 'Lab-2',
            ip: '142.191.237.11',
            ignoredParam: 'something',
        };
        let response = await request(global.__SERVER__)
            .patch(APIS.machinesApi + "/" + machineToAdd.mac)
            .send(newObj);
        expect(response.status).toBe(200);
        response = await request(global.__SERVER__).get(APIS.machinesApi);
        
        expect(response.body[0].ip).toBe(newObj.ip);
        expect(response.body[0].name).toBe(newObj.name);
        expect(response.body[0].createdTime).toBeTruthy();
        expect(response.body[0].modifiedTime).toBeTruthy();
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
    });

    test('update a machine - fail', async () => {
        let response = await request(global.__SERVER__)
            .patch(APIS.machinesApi + "/" + machineToAdd.mac)
            .send({
                notExist: 'notExist',
            });
        expect(response.status).toBe(500);//nothing to update
        
        response = await request(global.__SERVER__)
            .patch(APIS.machinesApi + "/" + 'notexist')
            .send({
                ip: '111.222.333.44',
            });
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
        let response = await request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd);
        expect(response.status).toBe(201);

        response = await request(global.__SERVER__).delete(APIS.machinesApi + "/" + machineToAdd.mac);
        expect(response.status).toBe(400);
        expect(response.body.error).toBe("Machine has 1 uba devices, can't delete.");

        //delete uba in order to delete machine
        response = await request(global.__SERVER__).delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN);
        expect(response.status).toBe(204);

        response = await request(global.__SERVER__).delete(APIS.machinesApi + "/" + machineToAdd.mac);
        expect(response.status).toBe(204);

        response = await request(global.__SERVER__).get(APIS.machinesApi);
        expect(response.body.length).toBe(0);
    });

    test('delete a machine - fail', async () => {
        let response = await request(global.__SERVER__)
            .delete(APIS.machinesApi + "/" + 'notexist');
        expect(response.status).toBe(400);//machine not exists
    });

    // co-pilot-missing-required-fields
    test('co-pilot-missing-required-fields', async () => {
        for (const field of machineModel.createProperties) {
            const machineCopy = { ...machineToAdd };
            delete machineCopy[field];
            const response = await request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send(machineCopy);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-extra-unknown-fields
    test('co-pilot-extra-unknown-fields', async () => {
        const machineWithExtra = { ...machineToAdd, extraField: 'shouldBeIgnored', mac: '00-10-FA-63-38-new', };
        const response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineWithExtra);
        expect([201]).toContain(response.status);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send({});
        expect(response.status).toBe(500);
    });

    // co-pilot-boundary-values
    test('co-pilot-boundary-values', async () => {
        const minMachine = { ...machineToAdd, name: '', ip: '', mac: '00-10-FA-63-38-new2', };//name is required
        const response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(minMachine);
        expect([500]).toContain(response.status);
    });

    // co-pilot-duplicate-primary-key
    test('co-pilot-duplicate-primary-key', async () => {
        await request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd);
        const response = await request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        // All fields are NOT NULL, so try omitting each
        for (const field of machineModel.createProperties) {
            const machineCopy = { ...machineToAdd };
            delete machineCopy[field];
            const response = await request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send(machineCopy);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(1000); // Exceeds varchar(64)
        const tooLong = { ...machineToAdd, name: longString, ip: longString };
        const response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(tooLong);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        await request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd);
        const response = await request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd);
        expect([409, 500]).toContain(response.status);
    });

});