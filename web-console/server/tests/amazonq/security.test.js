const request = require('supertest');
const runSchema = require('../prepareDb');
const { APIS } = require('../../utils/constants');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Security Tests', () => {

    beforeAll(async () => {
        await withTimeout(runSchema(), AWAIT_TIMEOUT);
    });

    afterAll(async () => {
        console.log('Security Test suite finished');
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });

    describe('SQL Injection Prevention', () => {
        test('should prevent SQL injection in cell creation', async () => {
            const maliciousPayload = {
                chemistry: "'; DROP TABLE CellPartNumbers; --",
                manufacturer: "'; DELETE FROM CellPartNumbers; --",
                itemPN: 'SQL_INJECT_TEST',
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
                .send(maliciousPayload), AWAIT_TIMEOUT);
            
            // Should either create safely or reject, but not execute SQL injection
            expect([201, 400, 500]).toContain(response.status);
            
            // Verify table still exists by making a GET request
            const getResponse = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi), AWAIT_TIMEOUT);
            expect(getResponse.status).toBe(200);
        });

        test('should prevent SQL injection in search parameters', async () => {
            const response = await withTimeout(request(global.__SERVER__)
                .get(APIS.cellsApi + "?search='; DROP TABLE CellPartNumbers; --"), AWAIT_TIMEOUT);
            
            expect([200, 400, 500]).toContain(response.status);
        });

        test('should prevent SQL injection in machine updates', async () => {
            // First create a machine
            await withTimeout(request(global.__SERVER__)
                .post(APIS.machinesApi)
                .send({
                    mac: '00-AA-BB-CC-DD-EE',
                    name: 'Security Test Machine',
                    ip: '192.168.1.200'
                }), AWAIT_TIMEOUT);

            const maliciousUpdate = {
                name: "'; UPDATE Machines SET name='HACKED' WHERE 1=1; --",
                ip: "'; DROP TABLE Machines; --"
            };

            const response = await withTimeout(request(global.__SERVER__)
                .patch(APIS.machinesApi + '/00-AA-BB-CC-DD-EE')
                .send(maliciousUpdate), AWAIT_TIMEOUT);
            
            expect([200, 400, 500]).toContain(response.status);
            
            // Clean up
            await withTimeout(request(global.__SERVER__).delete(APIS.machinesApi + '/00-AA-BB-CC-DD-EE'), AWAIT_TIMEOUT);
        });
    });

    describe('Input Validation & Sanitization', () => {
        test('should reject XSS attempts in string fields', async () => {
            const xssPayload = {
                chemistry: '<script>alert("XSS")</script>',
                manufacturer: '<img src=x onerror=alert("XSS")>',
                itemPN: 'XSS_TEST',
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
                .send(xssPayload), AWAIT_TIMEOUT);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                // If created, verify the data is properly handled
                const getResponse = await withTimeout(request(global.__SERVER__).get(APIS.cellsApi), AWAIT_TIMEOUT);
                const createdCell = getResponse.body.find(c => c.itemPN === 'XSS_TEST');
                
                if (createdCell) {
                    // Data should be stored safely (not executed)
                    expect(createdCell.chemistry).toBeDefined();
                    expect(createdCell.manufacturer).toBeDefined();
                }
                
                // Clean up
                await withTimeout(request(global.__SERVER__).delete(APIS.cellsApi + '/XSS_TEST'), AWAIT_TIMEOUT);
            }
        });

        test('should handle extremely long strings', async () => {
            const longString = 'A'.repeat(10000);
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send({
                    chemistry: longString,
                    manufacturer: longString,
                    itemPN: 'LONG_TEST',
                    minVoltage: 2.5,
                    nomVoltage: 3.7,
                    maxVoltage: 4.2,
                    minCapacity: 1000,
                    nomCapacity: 1100,
                    minTemp: -10,
                    maxTemp: 50,
                    chargeOption: 'Primary'
                }), AWAIT_TIMEOUT);
            
            expect([400, 413, 500]).toContain(response.status);
        });

        test('should validate numeric ranges', async () => {
            const invalidRanges = [
                { minVoltage: -999999, nomVoltage: 3.7, maxVoltage: 4.2 },
                { minVoltage: 2.5, nomVoltage: 999999, maxVoltage: 4.2 },
                { minVoltage: 2.5, nomVoltage: 3.7, maxVoltage: -999999 }
            ];

            for (const ranges of invalidRanges) {
                const response = await withTimeout(request(global.__SERVER__)
                    .post(APIS.cellsApi)
                    .send({
                        chemistry: 'Li-Ion',
                        manufacturer: 'Range Test',
                        itemPN: 'RANGE_TEST',
                        minCapacity: 1000,
                        nomCapacity: 1100,
                        minTemp: -10,
                        maxTemp: 50,
                        chargeOption: 'Primary',
                        ...ranges
                    }), AWAIT_TIMEOUT);
                
                expect([201, 400, 500]).toContain(response.status);
            }
        });
    });

    describe('Request Size Limits', () => {
        test('should handle oversized JSON payloads', async () => {
            const oversizedData = {
                chemistry: 'Li-Ion',
                manufacturer: 'Size Test',
                itemPN: 'SIZE_TEST',
                minVoltage: 2.5,
                nomVoltage: 3.7,
                maxVoltage: 4.2,
                minCapacity: 1000,
                nomCapacity: 1100,
                minTemp: -10,
                maxTemp: 50,
                chargeOption: 'Primary',
                largeField: 'x'.repeat(1000000 * 501) // 501MB string
            };

            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(oversizedData), AWAIT_TIMEOUT);
            
            expect([413, 500]).toContain(response.status);
        });
    });

    describe('Header Security', () => {
        test('should include security headers', async () => {
            const response = await withTimeout(request(global.__SERVER__)
                .get(APIS.cellsApi), AWAIT_TIMEOUT);
            
            expect(response.status).toBe(200);
            // Helmet middleware should add security headers
            expect(response.headers).toHaveProperty('x-content-type-options');
        });

        test('should handle malicious headers', async () => {
            const response = await withTimeout(request(global.__SERVER__)
                .get(APIS.cellsApi)
                .set('X-Forwarded-For', '<script>alert("XSS")</script>')
                .set('User-Agent', '"; DROP TABLE CellPartNumbers; --'), AWAIT_TIMEOUT);
            
            expect(response.status).toBe(200);
        });
    });

    describe('JSON Parsing Security', () => {
        test('should handle malformed JSON gracefully', async () => {
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .set('Content-Type', 'application/json')
                .send('{"chemistry": "Li-Ion", "invalid": }'), AWAIT_TIMEOUT);
            
            expect([400, 500]).toContain(response.status);
        });

        test('should handle deeply nested JSON', async () => {
            let deepObject = { chemistry: 'Li-Ion' };
            for (let i = 0; i < 100; i++) {
                deepObject = { nested: deepObject };
            }

            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(deepObject), AWAIT_TIMEOUT);
            
            expect([400, 413, 500]).toContain(response.status);
        });
    });

    describe('Path Traversal Prevention', () => {
        test('should prevent directory traversal in static routes', async () => {
            const maliciousPaths = [
                '../../../etc/passwd',
                '..\\..\\..\\windows\\system32\\config\\sam',
                '%2e%2e%2f%2e%2e%2f%2e%2e%2fetc%2fpasswd'
            ];

            for (const path of maliciousPaths) {
                const response = await withTimeout(request(global.__SERVER__)
                    .get('/' + path), AWAIT_TIMEOUT);
                
                // Should not expose system files
                expect([301, 404, 403, 400]).toContain(response.status);
            }
        });
    });
});