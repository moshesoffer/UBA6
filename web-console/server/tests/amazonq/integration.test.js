const request = require('supertest');
const runSchema = require('../prepareDb');
const { APIS } = require('../../utils/constants');

describe('Integration Tests', () => {

    beforeAll(async () => {
        await runSchema();
    });

    afterAll(async () => {
        console.log('Integration Test suite finished');
        await new Promise(resolve => setTimeout(resolve, 500));
    });

    describe('Health Check', () => {
        test('should return healthy status', async () => {
            const response = await request(global.__SERVER__).get('/health');
            
            expect(response.status).toBe(200);
            expect(response.body).toHaveProperty('status', 'ok');
            expect(response.body).toHaveProperty('db', 'up');
        });
    });

    describe('CORS Headers', () => {
        test('should handle CORS when enabled', async () => {
            // Test with CORS enabled environment
            process.env.ENABLE_CORS_FOR_LOCALHOST = 'true';
            
            const response = await request(global.__SERVER__)
                .get(APIS.cellsApi)
                .set('Origin', 'http://localhost:3000');
            
            expect(response.status).toBe(200);
        });
    });

    describe('Error Handling', () => {
        test('should handle malformed JSON', async () => {
            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .set('Content-Type', 'application/json')
                .send('{"invalid": json}');
            
            expect(response.status).toBe(400);
        });

        test('should handle unsupported content type', async () => {
            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .set('Content-Type', 'text/plain')
                .send('plain text data');
            
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

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(largePayload);
            
            expect([413, 500]).toContain(response.status);
        });
    });

    describe('API Endpoints Integration', () => {
        test('should handle complex workflow', async () => {
            // Create a cell
            const cellResponse = await request(global.__SERVER__)
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
                });
            
            expect(cellResponse.status).toBe(201);

            // Create a machine
            const machineResponse = await request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send({
                    mac: '00-11-22-33-44-55',
                    name: 'Integration Test Machine',
                    ip: '192.168.1.100'
                });
            
            expect(machineResponse.status).toBe(201);

            // Verify both exist
            const cellsResponse = await request(global.__SERVER__).get(APIS.cellsApi);
            const machinesResponse = await request(global.__SERVER__).get(APIS.machinesApi);
            
            expect(cellsResponse.status).toBe(200);
            expect(machinesResponse.status).toBe(200);
            
            const createdCell = cellsResponse.body.find(c => c.itemPN === 'INT_TEST_001');
            const createdMachine = machinesResponse.body.find(m => m.mac === '00-11-22-33-44-55');
            
            expect(createdCell).toBeDefined();
            expect(createdMachine).toBeDefined();

            // Clean up
            await request(global.__SERVER__).delete(APIS.cellsApi + '/INT_TEST_001');
            await request(global.__SERVER__).delete(APIS.machinesApi + '/00-11-22-33-44-55');
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
            
            const responses = await Promise.all(promises);
            
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
            
            const responses = await Promise.all(promises);
            
            responses.forEach(response => {
                expect([201]).toContain(response.status);
            });

            // Clean up
            for (let i = 0; i < 3; i++) {
                await request(global.__SERVER__).delete(APIS.cellsApi + `/CONCURRENT_${i}`);
            }
        });
    });
});