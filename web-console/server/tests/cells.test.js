const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { cellModel } = require('../models');
const { clearMemInServer } = require('./testHelper');
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
    afterEach(async () => {
        await clearMemInServer();
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

        newObj.minVoltage = Number(newObj.minVoltage);
        newObj.nomVoltage = Number(newObj.nomVoltage);
        newObj.maxVoltage = Number(newObj.maxVoltage);
        newObj.minCapacity = Number(newObj.minCapacity);
        newObj.nomCapacity = Number(newObj.nomCapacity);
        newObj.minTemp = Number(newObj.minTemp);
        newObj.maxTemp = Number(newObj.maxTemp);

        const { createdTime, modifiedTime, ...restExpected } = response.body[0];
        expect(restExpected).toEqual(newObj);
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

    // co-pilot-missing-required-fields
    test('co-pilot-missing-required-fields', async () => {
        for (const field of cellModel.createProperties) {
            const cellCopy = { ...cellToAdd };
            delete cellCopy[field];
            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(cellCopy);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-invalid-data-types
    test('co-pilot-invalid-data-types', async () => {
        const invalidCell = { ...cellToAdd, minVoltage: 'not-a-number' };
        const response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(invalidCell);
        expect(response.status).toBe(500);
    });

    // co-pilot-extra-unknown-fields
    test('co-pilot-extra-unknown-fields', async () => {
        const cellWithExtra = { ...cellToAdd, extraField: 'shouldBeIgnored', itemPN: 'new 123' };
        const response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellWithExtra);
        expect([201]).toContain(response.status);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send({});
        expect(response.status).toBe(500);
    });

    // co-pilot-boundary-values
    test('co-pilot-boundary-values', async () => {
        const minCell = { ...cellToAdd, minVoltage: '0', maxVoltage: '1000', itemPN: 'new 1234' };
        const response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(minCell);
        expect([201]).toContain(response.status); 
    });

    // co-pilot-duplicate-primary-key
    test('co-pilot-duplicate-primary-key', async () => {
        await request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd);
        const response = await request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd);
        expect([409, 500]).toContain(response.status); // 409 if unique constraint, 500 if generic error
    });

    // co-pilot-get-nonexistent-cell
    test('co-pilot-get-nonexistent-cell', async () => {
        const response = await request(global.__SERVER__).get(APIS.cellsApi + '/notexist');
        expect([301, 404, 500]).toContain(response.status);
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        for (const field of cellModel.createProperties) {
            const cellCopy = { ...cellToAdd };
            delete cellCopy[field];
            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(cellCopy);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(1000); // Exceeds varchar(64)
        const tooLong = { ...cellToAdd, chemistry: longString, manufacturer: longString, itemPN: longString };
        const response = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(tooLong);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        await request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd);
        const response = await request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd);
        expect([409, 500]).toContain(response.status);
    });
});