const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { ubaDeviceModel, runningTestsModel, instantTestResultsModel } = require('../models');

const { APIS } = require('../utils/constants');

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
        isConnected: 0,
    };

    let connection;

    beforeAll(async () => {
        await runSchema();
        connection = await mysql.createConnection(global.__MYSQL_CONFIG__);

        let response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineToAdd);
        expect(response.status).toBe(201);

        response = await request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(ubaDeviceToAdd);
        expect(response.status).toBe(201);
        expect(response.body.length).toBe(2);//2 ids of running tests

        const [ubaRows] = await connection.query(`SELECT * FROM \`${ubaDeviceModel.tableName}\`;`);
        expect(ubaRows.length).toBe(1);
        const [runningTestsRows] = await connection.query(`SELECT * FROM \`${runningTestsModel.tableName}\`;`);
        expect(runningTestsRows.length).toBe(2);
        const [instantTestResultsRows] = await connection.query(`SELECT * FROM \`${instantTestResultsModel.tableName}\`;`);
        expect(instantTestResultsRows.length).toBe(0);
        
    });

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));//waiting for winston server logs to finish
    });

    test('add a new ubaDevice - fail', async () => {
        const { ubaChannel, ...ubaDeviceWithoutUbaChannel } = ubaDeviceToAdd;
        const response = await request(global.__SERVER__)
            .post(APIS.ubaDevicesApi)
            .send(ubaDeviceWithoutUbaChannel);
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
        let response = await request(global.__SERVER__)
            .patch(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN)
            .send(newObj);
        expect(response.status).toBe(200);
        response = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        
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
        let response = await request(global.__SERVER__)
            .patch(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN)
            .send({
                notExist: 'notExist',
            });
        expect(response.status).toBe(500);//nothing to update
        
        response = await request(global.__SERVER__)
            .patch(APIS.ubaDevicesApi + "/" + 'notexist')
            .send({
                name: 'new',
            });
        expect(response.status).toBe(500);//ubaDevice not exists
    });

    test('delete a ubaDevice - happy flow', async () => {
        let response = await request(global.__SERVER__)
            .delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN);
        expect(response.status).toBe(204);
        response = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        const ubaDevices = response.body.ubaDevices;
        const ubaTotal = response.body.ubaTotal;
        expect(ubaDevices.length).toBe(0);
        expect(ubaTotal.configured).toBe(0);
        expect(ubaTotal.connected).toBe(0);
        expect(ubaTotal.running).toBe(0);

        const [ubaRows] = await connection.query(`SELECT * FROM \`${ubaDeviceModel.tableName}\`;`);
        expect(ubaRows.length).toBe(0);
        const [runningTestsRows] = await connection.query(`SELECT * FROM \`${runningTestsModel.tableName}\`;`);
        expect(runningTestsRows.length).toBe(0);
    });

    test('delete a ubaDevice - fail', async () => {
        let response = await request(global.__SERVER__)
            .delete(APIS.ubaDevicesApi + "/" + 'notexist');
        expect(response.status).toBe(500);//ubaDevice not exists
    });

});