const request = require('supertest');
const runSchema = require('../prepareDb');
const { APIS } = require('../../utils/constants');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Integration Tests', () => {

    beforeAll(async () => {
        await withTimeout(runSchema(), AWAIT_TIMEOUT);
    });

    afterAll(async () => {
        console.log('Integration Test suite finished');
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });

    describe('Health Check', () => {
        test('should return healthy status', async () => {
            const response = await withTimeout(request(global.__SERVER__).get('/health'), AWAIT_TIMEOUT);
            
            expect(response.status).toBe(200);
            expect(response.body).toHaveProperty('status', 'ok');
            expect(response.body).toHaveProperty('db', 'up');
        });
    });

    describe('CORS Headers', () => {
        test('should handle CORS when enabled', async () => {
            // Test with CORS enabled environment
            process.env.ENABLE_CORS_FOR_LOCALHOST = 'true';
            
            const response = await withTimeout(request(global.__SERVER__)
                .get(APIS.cellsApi)
                .set('Origin', 'http://localhost:3000'), AWAIT_TIMEOUT);
            
            expect(response.status).toBe(200);
        });
    });

    describe('Error Handling', () => {
        test('should handle malformed JSON', async () => {
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .set('Content-Type', 'application/json')
                .send('{"invalid": json}'), AWAIT_TIMEOUT);
            
            expect(response.status).toBe(400);
        });

        test('should handle unsupported content type', async () => {
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .set('Content-Type', 'text/plain')
                .send('plain text data'), AWAIT_TIMEOUT);
            
            expect([400, 415, 500]).toContain(response.status);
        });

        test('should handle very large payloads', async () => {
            const largePayload = {
                chemistry: 'x'.repeat(10000),
                manufacturer: 'x'.repeat(10000),
                itemPN: 'LARGE_TEST',
                minVoltage: 2.5,
                nomVoltage: 3.7,
                maxVoltage: 4.2,
                minCapacity: 1000,
                nomCapacity: 1100,
                minTemp: -10,
                maxTemp: 50,
                chargeOption: 'Primary'
            };

            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(largePayload), AWAIT_TIMEOUT);
            
            expect([413, 500]).toContain(response.status);
        });
    });

    describe('API Endpoints Integration', () => {
        test('should handle complex workflow', async () => {
            // Create a cell
            const cellResponse = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send({
                    chemistry: 'Li-Ion',
                    manufacturer: 'Integration Test',
                    itemPN: 'INT_TEST_001',
                    minVoltage: 2.5,
                    nomVoltage: 3.7,
                    maxVoltage: 4.2,
                    minCapacity: 1000,
                    nomCapacity: 1100,
                    minTemp: -10,
                    maxTemp: 50,
                    chargeOption: 'Primary'
                }), AWAIT_TIMEOUT);
            
            expect(cellResponse.status).toBe(201);

            // Create a machine
            const machineResponse = await withTimeout(request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send({
                    mac: '00-11-22-33-44-55',
                    name: 'Integration Test Machine',
                    ip: '192.168.1.100'
                }), AWAIT_TIMEOUT);
            
            expect(machineResponse.status).toBe(201);

            // Verify both exist
            const cellsResponse = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi), AWAIT_TIMEOUT);
            const machinesResponse = await withTimeout(request(global.__SERVER__).get(APIS.machinesApi), AWAIT_TIMEOUT);
            
            expect(cellsResponse.status).toBe(200);
            expect(machinesResponse.status).toBe(200);
            
            const createdCell = cellsResponse.body.find(c => c.itemPN === 'INT_TEST_001');
            const createdMachine = machinesResponse.body.find(m => m.mac === '00-11-22-33-44-55');
            
            expect(createdCell).toBeDefined();
            expect(createdMachine).toBeDefined();

            // Clean up
            await withTimeout(request(global.__SERVER__).delete(APIS.cellsApi + '/INT_TEST_001'), AWAIT_TIMEOUT);
            await withTimeout(request(global.__SERVER__).delete(APIS.machinesApi + '/00-11-22-33-44-55'), AWAIT_TIMEOUT);
        });
    });

    describe('Concurrent Requests', () => {
        test('should handle multiple simultaneous requests', async () => {
            const promises = [];
            
            for (let i = 0; i < 5; i++) {
                promises.push(
                    request(global.__SERVER__)
                        .get(APIS.cellsApi)
                );
            }
            
            const responses = await withTimeout(Promise.all(promises), AWAIT_TIMEOUT);
            
            responses.forEach(response => {
                expect(response.status).toBe(200);
            });
        });

        test('should handle concurrent writes', async () => {
            const promises = [];
            
            for (let i = 0; i < 3; i++) {
                promises.push(
                    request(global.__SERVER__)
                        .post(APIS.cellsApi)
                        .send({
                            chemistry: 'Li-Ion',
                            manufacturer: 'Concurrent Test',
                            itemPN: `CONCURRENT_${i}`,
                            minVoltage: 2.5,
                            nomVoltage: 3.7,
                            maxVoltage: 4.2,
                            minCapacity: 1000,
                            nomCapacity: 1100,
                            minTemp: -10,
                            maxTemp: 50,
                            chargeOption: 'Primary'
                        })
                );
            }
            
            const responses = await withTimeout(Promise.all(promises), AWAIT_TIMEOUT);
            
            responses.forEach(response => {
                expect([201]).toContain(response.status);
            });

            // Clean up
            for (let i = 0; i < 3; i++) {
                await withTimeout(request(global.__SERVER__).delete(APIS.cellsApi + `/CONCURRENT_${i}`), AWAIT_TIMEOUT);
            }
        });
    });
});