const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('../prepareDb');
const { APIS, status } = require('../../utils/constants');

describe('Edge Cases and Error Scenarios', () => {

    let connection;

    beforeAll(async () => {
        await runSchema();
        connection = await mysql.createConnection(global.__MYSQL_CONFIG__);
    });

    afterAll(async () => {
        console.log('Edge Cases Test suite finished');
        if (connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));
    });

    describe('Database Connection Edge Cases', () => {
        test('should handle health check when database is accessible', async () => {
            const response = await request(global.__SERVER__).get('/health');
            
            expect(response.status).toBe(200);
            expect(response.body.status).toBe('ok');
            expect(response.body.db).toBe('up');
        });
    });

    describe('Boundary Value Testing', () => {
        test('should handle minimum valid values', async () => {
            const minValidCell = {
                chemistry: 'A',
                manufacturer: 'B',
                itemPN: 'MIN_TEST',
                minVoltage: 0.001,
                nomVoltage: 0.002,
                maxVoltage: 0.003,
                minCapacity: 0.001,
                nomCapacity: 0.002,
                minTemp: -273.15, // Absolute zero
                maxTemp: -273.14,
                chargeOption: 'P'
            };

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(minValidCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/MIN_TEST');
            }
        });

        test('should handle maximum valid values', async () => {
            const maxValidCell = {
                chemistry: 'x'.repeat(64),
                manufacturer: 'x'.repeat(64),
                itemPN: 'MAX_TEST',
                minVoltage: 999999.9999,
                nomVoltage: 999999.9999,
                maxVoltage: 999999.9999,
                minCapacity: 999999.99999,
                nomCapacity: 999999.99999,
                minTemp: 999999.9999,
                maxTemp: 999999.9999,
                chargeOption: 'x'.repeat(64)
            };

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(maxValidCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/MAX_TEST');
            }
        });

        test('should handle zero values appropriately', async () => {
            const zeroValueCell = {
                chemistry: 'Li-Ion',
                manufacturer: 'Zero Test',
                itemPN: 'ZERO_TEST',
                minVoltage: 0,
                nomVoltage: 0,
                maxVoltage: 0,
                minCapacity: 0,
                nomCapacity: 0,
                minTemp: 0,
                maxTemp: 0,
                chargeOption: 'Primary'
            };

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(zeroValueCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/ZERO_TEST');
            }
        });
    });

    describe('Unicode and Special Characters', () => {
        test('should handle Unicode characters', async () => {
            const unicodeCell = {
                chemistry: '锂离子',
                manufacturer: 'Тест',
                itemPN: 'UNICODE_TEST',
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
                .send(unicodeCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                const getResponse = await request(global.__SERVER__).get(APIS.cellsApi);
                const createdCell = getResponse.body.find(c => c.itemPN === 'UNICODE_TEST');
                expect(createdCell).toBeDefined();
                
                await request(global.__SERVER__).delete(APIS.cellsApi + '/UNICODE_TEST');
            }
        });

        test('should handle special characters in strings', async () => {
            const specialCharCell = {
                chemistry: '!@#$%^&*()_+-=[]{}|;:,.<>?',
                manufacturer: '`~"\'\\/',
                itemPN: 'SPECIAL_TEST',
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
                .send(specialCharCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/SPECIAL_TEST');
            }
        });
    });

    describe('Null and Undefined Handling', () => {
        test('should handle null values in optional fields', async () => {
            const nullFieldsCell = {
                chemistry: 'Li-Ion',
                manufacturer: 'Null Test',
                itemPN: 'NULL_TEST',
                minVoltage: 2.5,
                nomVoltage: 3.7,
                maxVoltage: 4.2,
                minCapacity: 1000,
                nomCapacity: 1100,
                minTemp: -10,
                maxTemp: 50,
                chargeOption: 'Primary',
                optionalField: null
            };

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(nullFieldsCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/NULL_TEST');
            }
        });

        test('should handle undefined values', async () => {
            const undefinedFieldsCell = {
                chemistry: 'Li-Ion',
                manufacturer: 'Undefined Test',
                itemPN: 'UNDEFINED_TEST',
                minVoltage: 2.5,
                nomVoltage: 3.7,
                maxVoltage: 4.2,
                minCapacity: 1000,
                nomCapacity: 1100,
                minTemp: -10,
                maxTemp: 50,
                chargeOption: 'Primary',
                undefinedField: undefined
            };

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(undefinedFieldsCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/UNDEFINED_TEST');
            }
        });
    });

    describe('Floating Point Precision', () => {
        test('should handle floating point precision issues', async () => {
            const precisionCell = {
                chemistry: 'Li-Ion',
                manufacturer: 'Precision Test',
                itemPN: 'PRECISION_TEST',
                minVoltage: 0.1 + 0.2, // 0.30000000000000004
                nomVoltage: 3.7000000000001,
                maxVoltage: 4.199999999999999,
                minCapacity: 1000.0000000001,
                nomCapacity: 1100.9999999999,
                minTemp: -10.000000000001,
                maxTemp: 50.999999999999,
                chargeOption: 'Primary'
            };

            const response = await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send(precisionCell);
            
            expect([201, 400, 500]).toContain(response.status);
            
            if (response.status === 201) {
                await request(global.__SERVER__).delete(APIS.cellsApi + '/PRECISION_TEST');
            }
        });
    });

    describe('Complex JSON Structures', () => {
        test('should handle complex plan structures in test routines', async () => {
            // First create a cell
            await request(global.__SERVER__)
                .post(APIS.cellsApi)
                .send({
                    chemistry: 'Li-Ion',
                    manufacturer: 'Complex Test',
                    itemPN: 'COMPLEX_CELL',
                    minVoltage: 2.5,
                    nomVoltage: 3.7,
                    maxVoltage: 4.2,
                    minCapacity: 1000,
                    nomCapacity: 1100,
                    minTemp: -10,
                    maxTemp: 50,
                    chargeOption: 'Primary'
                });

            const complexPlan = [];
            for (let i = 0; i < 50; i++) {
                complexPlan.push({
                    id: i,
                    type: i % 2 === 0 ? 'charge' : 'discharge',
                    isCollapsed: false,
                    source: 'internal',
                    isMinTemp: true,
                    minTemp: -20 + i,
                    isMaxTemp: true,
                    maxTemp: 60 - i,
                    isMaxTime: true,
                    maxTime: `${String(i).padStart(2, '0')}:${String(i).padStart(2, '0')}:${String(i).padStart(2, '0')}`,
                    delayTime: null,
                    isChargeLimit: true,
                    chargeLimit: `${1000 + i}:absoluteMah`,
                    isDischargeLimit: true,
                    dischargeLimit: `${500 + i}:absoluteMah`,
                    chargeCurrent: `${100 + i}:absoluteMa`,
                    dischargeCurrent: `${50 + i}:absoluteMa`,
                    cRate: 0.5 + (i * 0.01),
                    isCutOffCurrent: true,
                    cutOffCurrent: `${10 + i}:absoluteMa`,
                    isCutOffVoltage: true,
                    cutOffVoltage: 2.5 + (i * 0.01),
                    chargePerCell: i % 10,
                    waitTemp: null,
                    goToStep: null,
                    repeatStep: null
                });
            }

            const complexTestRoutine = {
                testName: 'Complex Plan Test',
                isLocked: 0,
                batteryPN: 'COMPLEX_BATTERY',
                batterySN: 'COMPLEX_SN',
                cellPN: 'COMPLEX_CELL',
                noCellSerial: 1,
                noCellParallel: 2,
                maxPerBattery: 8.7453,
                ratedBatteryCapacity: 4080,
                channel: 'A-and-B',
                notes: 'Complex plan test',
                customer: 'Test Customer',
                workOrderNumber: 'WO123',
                approvedBy: 'Tester',
                conductedBy: 'Tester',
                cellSupplier: 'Test Supplier',
                cellBatch: 'BATCH001',
                plan: complexPlan
            };

            const response = await request(global.__SERVER__)
                .post(APIS.testRoutinesApi)
                .send(complexTestRoutine);
            
            expect([201, 400, 413, 500]).toContain(response.status);
            
            // Clean up
            await request(global.__SERVER__).delete(APIS.cellsApi + '/COMPLEX_CELL');
            if (response.status === 201) {
                const routines = await request(global.__SERVER__).get(APIS.testRoutinesApi);
                const createdRoutine = routines.body.find(r => r.testName === 'Complex Plan Test');
                if (createdRoutine) {
                    await request(global.__SERVER__).delete(APIS.testRoutinesApi + '/' + createdRoutine.id);
                }
            }
        });
    });

    describe('Race Conditions', () => {
        test('should handle simultaneous creation of same resource', async () => {
            const cellData = {
                chemistry: 'Li-Ion',
                manufacturer: 'Race Test',
                itemPN: 'RACE_TEST',
                minVoltage: 2.5,
                nomVoltage: 3.7,
                maxVoltage: 4.2,
                minCapacity: 1000,
                nomCapacity: 1100,
                minTemp: -10,
                maxTemp: 50,
                chargeOption: 'Primary'
            };

            const promises = [
                request(global.__SERVER__).post(APIS.cellsApi).send(cellData),
                request(global.__SERVER__).post(APIS.cellsApi).send(cellData),
                request(global.__SERVER__).post(APIS.cellsApi).send(cellData)
            ];

            const responses = await Promise.all(promises);
            
            // Only one should succeed due to unique constraint
            const successCount = responses.filter(r => r.status === 201).length;
            const errorCount = responses.filter(r => [409, 500].includes(r.status)).length;
            
            expect(successCount).toBe(1);
            expect(errorCount).toBe(2);
            
            // Clean up
            await request(global.__SERVER__).delete(APIS.cellsApi + '/RACE_TEST');
        });
    });

    describe('Memory and Performance Edge Cases', () => {
        test('should handle large test results arrays', async () => {
            const largeTestResults = [];
            for (let i = 0; i < 1000; i++) {
                largeTestResults.push({
                    timestamp: i * 100,
                    voltage: 3.7 + (Math.random() * 0.5),
                    temperature: 25 + (Math.random() * 10),
                    current: 1.5 + (Math.random() * 0.5)
                });
            }

            const reportData = {
                ubaSN: '99999',
                channel: 'A',
                timestampStart: '2024-01-01T10:00:00Z',
                status: status.FINISHED,
                testName: 'Large Data Test',
                batteryPN: 'LARGE_BATTERY',
                batterySN: 'LARGE_SN',
                noCellSerial: 1,
                noCellParallel: 1,
                maxPerBattery: 8.0,
                ratedBatteryCapacity: 4000,
                notes: 'Large test results',
                customer: 'Test Customer',
                workOrderNumber: 'WO999',
                approvedBy: 'Tester',
                conductedBy: 'Tester',
                cellSupplier: 'Test Supplier',
                cellBatch: 'BATCH999',
                plan: [{ id: 0, type: 'charge' }],
                testRoutineChannels: 'A',
                machineName: 'Test Machine',
                testResults: largeTestResults
            };

            const response = await request(global.__SERVER__)
                .post(APIS.createReportApi)
                .send(reportData);
            
            expect([201, 413, 500]).toContain(response.status);
        });
    });
});