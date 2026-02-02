const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { machineModel } = require('../models');
const { APIS } = require('../utils/constants');

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
        let response = await request(global.__SERVER__)
            .delete(APIS.machinesApi + "/" + machineToAdd.mac);
        expect(response.status).toBe(204);
        response = await request(global.__SERVER__).get(APIS.machinesApi);
        expect(response.body.length).toBe(0);
    });

    test('delete a machine - fail', async () => {
        let response = await request(global.__SERVER__)
            .delete(APIS.machinesApi + "/" + 'notexist');
        expect(response.status).toBe(500);//machine not exists
    });

});