const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { cellModel } = require('../models');
const { clearMemInServer } = require('./testHelper');
const { APIS } = require('../utils/constants');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

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
        await withTimeout(runSchema(), AWAIT_TIMEOUT);
        connection = await withTimeout(mysql.createConnection(global.__MYSQL_CONFIG__), AWAIT_TIMEOUT);

        let response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);
        const [rows] = await withTimeout(connection.query(`SELECT * FROM \`${cellModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(rows.length).toBe(1);

        response = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        expect(response.body.length).toBe(1);
        expect(response.body[0]).not.toHaveProperty('ignoredParam');
        cellModel.createProperties.forEach(prop => {
            expect(response.body[0]).toHaveProperty(prop);
        });
    });

    afterAll(async () => {
        console.log('Cell Test suite finished');
        if(connection) await withTimeout(connection.end(), AWAIT_TIMEOUT);
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });
    afterEach(async () => {
        await withTimeout(clearMemInServer(), AWAIT_TIMEOUT);
    });

    test('add a new cell - fail', async () => {
        const { chargeOption, ...cellWithoutChargeOption } = cellToAdd;
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellWithoutChargeOption), AWAIT_TIMEOUT);
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
        let response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.cellsApi + "/" + cellToAdd.itemPN)
            .send(newObj), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        response = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi), AWAIT_TIMEOUT);
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
        let response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.cellsApi + "/" + cellToAdd.itemPN)
            .send({
                notExist: 'notExist',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//nothing to update
        
        response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.cellsApi + "/" + 'notexist')
            .send({
                chemistry: 'Li-Ion',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//cell not exists
    });

    test('delete a cell - happy flow', async () => {
        let response = await withTimeout(request(global.__SERVER__)
            .delete(APIS.cellsApi + "/" + cellToAdd.itemPN), AWAIT_TIMEOUT);
        expect(response.status).toBe(204);
        response = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi), AWAIT_TIMEOUT);
        expect(response.body.length).toBe(0);
    });

    test('delete a cell - fail', async () => {
        let response = await withTimeout(request(global.__SERVER__)
            .delete(APIS.cellsApi + "/" + 'notexist'), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//cell not exists
    });

    // co-pilot-missing-required-fields
    test('co-pilot-missing-required-fields', async () => {
        for (const field of cellModel.createProperties) {
            const cellCopy = { ...cellToAdd };
            delete cellCopy[field];
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(cellCopy), AWAIT_TIMEOUT);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-invalid-data-types
    test('co-pilot-invalid-data-types', async () => {
        const invalidCell = { ...cellToAdd, minVoltage: 'not-a-number' };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(invalidCell), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-extra-unknown-fields
    test('co-pilot-extra-unknown-fields', async () => {
        const cellWithExtra = { ...cellToAdd, extraField: 'shouldBeIgnored', itemPN: 'new 123' };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(cellWithExtra), AWAIT_TIMEOUT);
        expect([201]).toContain(response.status);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send({}), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-boundary-values
    test('co-pilot-boundary-values', async () => {
        const minCell = { ...cellToAdd, minVoltage: '0', maxVoltage: '1000', itemPN: 'new 1234' };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(minCell), AWAIT_TIMEOUT);
        expect([201]).toContain(response.status); 
    });

    // co-pilot-duplicate-primary-key
    test('co-pilot-duplicate-primary-key', async () => {
        await withTimeout(request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd), AWAIT_TIMEOUT);
        const response = await withTimeout(request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status); // 409 if unique constraint, 500 if generic error
    });

    // co-pilot-get-nonexistent-cell
    test('co-pilot-get-nonexistent-cell', async () => {
        const response = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi + '/notexist'), AWAIT_TIMEOUT);
        expect([301, 404, 500]).toContain(response.status);
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        for (const field of cellModel.createProperties) {
            const cellCopy = { ...cellToAdd };
            delete cellCopy[field];
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(cellCopy), AWAIT_TIMEOUT);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(1000); // Exceeds varchar(64)
        const tooLong = { ...cellToAdd, chemistry: longString, manufacturer: longString, itemPN: longString };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send(tooLong), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        await withTimeout(request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd), AWAIT_TIMEOUT);
        const response = await withTimeout(request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });
});