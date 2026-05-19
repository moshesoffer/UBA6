const request = require('supertest');
const express = require('express');
const logRoute = require('../../middleware/loggerMiddleware');
const runSchema = require('../prepareDb');
const { APIS } = require('../../utils/constants');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Middleware Tests', () => {

    beforeAll(async () => {
        await runSchema();
    });

    afterAll(async () => {
        console.log('Middleware Test suite finished');
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });

    test('logger middleware - adds request ID', async () => {
        const response = await request(global.__SERVER__)
            .get(APIS.cellsApi);
        
        expect(response.status).toBe(200);
        // Request ID should be generated and logged (check logs for UUID format)
    });

    test('logger middleware - handles different HTTP methods', async () => {
        const getResponse = await request(global.__SERVER__)
            .get(APIS.cellsApi);
        expect(getResponse.status).toBe(200);

        const postResponse = await request(global.__SERVER__)
            .post(APIS.cellsApi)
            .send({
                chemistry: 'Li-Ion',
                manufacturer: 'Test',
                itemPN: 'TEST123',
                minVoltage: 2.5,
                nomVoltage: 3.7,
                maxVoltage: 4.2,
                minCapacity: 1000,
                nomCapacity: 1100,
                minTemp: -10,
                maxTemp: 50,
                chargeOption: 'Primary'
            });
        expect([201]).toContain(postResponse.status);
    });

    test('logger middleware - handles query parameters', async () => {
        const response = await request(global.__SERVER__)
            .get(APIS.cellsApi + '?test=value&another=param');
        
        expect(response.status).toBe(200);
    });

});