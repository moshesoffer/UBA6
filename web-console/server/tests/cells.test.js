const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { cellModel } = require('../models');

const { APIS } = require('../utils/constants');

describe('Cell API Tests', () => {

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

    beforeAll(async () => {
        await runSchema();
        connection = await mysql.createConnection(global.__MYSQL_CONFIG__);

        let response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellToAdd);
        expect(response.status).toBe(201);
        const [rows] = await connection.query(`SELECT * FROM \`${cellModel.tableName}\`;`);
        expect(rows.length).toBe(1);

        response = await request(global.__SERVER__).get(APIS.cellsApi);
        expect(response.status).toBe(200);
        expect(response.body.length).toBe(1);
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
        cellModel.createProperties.forEach(prop => {
            expect(response.body[0]).toHaveProperty(prop);
        });
    });

    afterAll(async () => {
        console.log('Cell Test suite finished');
        if(connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));//waiting for winston server logs to finish
    });

    test('add a new cell - fail', async () => {
        const { chargeOption, ...cellWithoutChargeOption } = cellToAdd;
        const response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellWithoutChargeOption);
        expect(response.status).toBe(500);//missing charge option
    });

    test('update a cell - happy flow', async () => {
        const newObj = {
            chemistry: 'Li-Ion',
            manufacturer: 'Samsung',
            //itemPN: 'ABLP75100250H300',
            minVoltage: '1.7500',
            nomVoltage: '2.7000',
            maxVoltage: '3.3000',
            minCapacity: '12400.00000',
            nomCapacity: '10000.00000',
            minTemp: '-30.0000',
            maxTemp: '40.0000',
            chargeOption: 'Secondary',
            ignoredParam: 'something',
        };
        let response = await request(global.__SERVER__)
            .patch(APIS.cellsApi + "/" + cellToAdd.itemPN)
            .send(newObj);
        expect(response.status).toBe(200);
        response = await request(global.__SERVER__).get(APIS.cellsApi);
        delete newObj.ignoredParam;
        newObj.itemPN = cellToAdd.itemPN;
        expect(response.body[0]).toEqual(newObj);
        expect(response.body[0].chemistry).toBe(newObj.chemistry);
    });

    test('update a cell - fail', async () => {
        let response = await request(global.__SERVER__)
            .patch(APIS.cellsApi + "/" + cellToAdd.itemPN)
            .send({
                notExist: 'notExist',
            });
        expect(response.status).toBe(500);//nothing to update
        
        response = await request(global.__SERVER__)
            .patch(APIS.cellsApi + "/" + 'notexist')
            .send({
                chemistry: 'Li-Ion',
            });
        expect(response.status).toBe(500);//cell not exists
    });

    test('delete a cell - happy flow', async () => {
        let response = await request(global.__SERVER__)
            .delete(APIS.cellsApi + "/" + cellToAdd.itemPN);
        expect(response.status).toBe(204);
        response = await request(global.__SERVER__).get(APIS.cellsApi);
        expect(response.body.length).toBe(0);
    });

    test('delete a cell - fail', async () => {
        let response = await request(global.__SERVER__)
            .delete(APIS.cellsApi + "/" + 'notexist');
        expect(response.status).toBe(500);//cell not exists
    });
});