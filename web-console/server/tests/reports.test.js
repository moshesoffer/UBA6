const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { reportModel, } = require('../models');
const { APIS, status, reportsDataPath, testResultsFileName } = require('../utils/constants');
const fs = require('fs');
const path = require('node:path');
const { readTextFromFile } = require('../utils/helper');
const { machine } = require('node:os');
const { clearMemInServer } = require('./testHelper');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Final Reports API Tests', () => {

    let connection;
    let reportIdAdded;

    const searchBody = {
        page: 0,
        rowsPerPage: 50,
        order: "asc",
        orderBy: "testName",
        filters: {
          batteryPN: "",
          batterySN: "",
          testName: "",
          ubaSN: "",
          machineName: "",
          dateRange: ""
        }
    };

    const reportToAdd = {
        ubaSN: '14530',
        channel: 'A',
        timestampStart: '2024-09-11 20:14:12',
        status: status.FINISHED,

        testName: 'C-D-C_3A_CUT OFF 2.5V',
        batteryPN: '901-24000365-1S2P-M1',
        batterySN: 'def5346536g',
        cellPN: 'ABLP75100250H300',

        noCellSerial: 1,
        noCellParallel:2 ,
        maxPerBattery: 8.7453,
        ratedBatteryCapacity: 4080.00000,
        notes: 'notes',
        customer: 'customer 0',
        workOrderNumber: 'fd',
        approvedBy: 'approvedBy',
        conductedBy: 'conductedBy',
        cellSupplier: 'cellSupplier',
       cellBatch: '',
        plan: [{"id": 0, "type": "charge", "cRate": 0.74, "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -201, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": "666:absoluteMah", "isCollapsed": false, "chargeCurrent": "3000:absoluteMa", "chargePerCell": 4, "cutOffCurrent": "50:absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true, "isIgnoreLimits": false}, {"id": 1, "type": "discharge", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -20, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": 2.5, "isChargeLimit": true, "dischargeLimit": "7:absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": "3:absoluteMa", "isDischargeLimit": true, "isIgnoreLimits": false}, {"id": 2, "type": "delay", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": null, "waitTemp": 7, "delayTime": "01:01:01", "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true, "isIgnoreLimits": false}, {"id": 3, "type": "loop", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": 0, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": 4, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true, "isIgnoreLimits": false}],
        
        testRoutineChannels: "A-and-B",
        machineName: "Lab-2",
        machineMac: "00:11:22:33:44:55",

        ignoredParam: 'something',

        testResults: [{"current": 0.343692, "voltage": 14.4064, "timestamp": 20, "temperature": ""}, {"current": 0.654142, "voltage": 14.5222, "timestamp": 40, "temperature": ""}, {"current": 0.999589, "voltage": 14.6416, "timestamp": 60, "temperature": ""}, {"current": 0.866251, "voltage": 14.7291, "timestamp": 80, "temperature": ""}, {"current": 0.901245, "voltage": 14.7533, "timestamp": 100, "temperature": ""}, {"current": 0.894454, "voltage": 14.7721, "timestamp": 120, "temperature": ""}, {"current": 0.89944, "voltage": 14.7879, "timestamp": 140, "temperature": ""}, {"current": 0.90322, "voltage": 14.8014, "timestamp": 160, "temperature": ""}, {"current": 0.89741, "voltage": 14.812, "timestamp": 180, "temperature": ""}, {"current": 0.899397, "voltage": 14.8204, "timestamp": 200, "temperature": ""}, {"current": 0.902741, "voltage": 14.8282, "timestamp": 220, "temperature": ""}, {"current": 0.90706, "voltage": 14.8346, "timestamp": 240, "temperature": ""}, {"current": 0.922276, "voltage": 14.8413, "timestamp": 260, "temperature": ""}, {"current": 0.933918, "voltage": 14.845, "timestamp": 280, "temperature": ""}, {"current": 0.938549, "voltage": 14.8511, "timestamp": 300, "temperature": ""}, {"current": 0.940365, "voltage": 14.8554, "timestamp": 320, "temperature": ""}, {"current": 0.943384, "voltage": 14.8609, "timestamp": 340, "temperature": ""}, {"current": 0.959419, "voltage": 14.8628, "timestamp": 360, "temperature": ""}, {"current": 0.940335, "voltage": 14.8682, "timestamp": 380, "temperature": ""}, {"current": 0.933241, "voltage": 14.8724, "timestamp": 400, "temperature": ""}, {"current": 0.94689, "voltage": 14.8758, "timestamp": 420, "temperature": ""}, {"current": 0.924843, "voltage": 14.8806, "timestamp": 440, "temperature": ""}, {"current": 0.936757, "voltage": 14.8849, "timestamp": 460, "temperature": ""}],
    };

    beforeAll(async () => {
        await withTimeout(runSchema(), AWAIT_TIMEOUT);
        connection = await withTimeout(mysql.createConnection(global.__MYSQL_CONFIG__), AWAIT_TIMEOUT);
        let response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send(reportToAdd), AWAIT_TIMEOUT);
        expect(response.status).toBe(201);

        let [rows] = await withTimeout(connection.query(`SELECT * FROM \`${reportModel.tableName}\`;`), AWAIT_TIMEOUT);
        expect(rows.length).toBe(1);

        const folderPath = path.join(reportsDataPath, rows[0].id);
        let entries = fs.readdirSync(folderPath, { withFileTypes: true });
        let fileCount = entries.filter(dirent => dirent.isFile()).length;
        expect(fileCount).toBe(1);
        const content = readTextFromFile(path.join(folderPath, testResultsFileName));
        const testResults = JSON.parse(content);
        expect(testResults.length).toBe(reportToAdd.testResults.length);
        
        response = await withTimeout(request(global.__SERVER__).post(APIS.getReportsApi).send(searchBody), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        expect(response.body.rows.length).toBe(1);
        expect(response.body.count).toBe(1);
        reportIdAdded = response.body.rows[0].id;
        expect(response.body.rows[0]).not.toHaveProperty('ignoredParam');
        reportModel.createProperties.forEach(prop => {
            expect(response.body.rows[0]).toHaveProperty(prop);
        });
        expect(response.body.rows[0].timeOfTest).toBe('00:07:40');

        response = await withTimeout(request(global.__SERVER__).post(APIS.reportsGraphApi).send([reportIdAdded]), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        expect(response.body.length).toBe(1);
        expect(response.body[0].testResults).toHaveLength(reportToAdd.testResults.length);
    });

    afterAll(async () => {
        console.log('Final Reports Test suite finished');
        if(connection) await withTimeout(connection.end(), AWAIT_TIMEOUT);
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });
    afterEach(async () => {
        await withTimeout(clearMemInServer(), AWAIT_TIMEOUT);
    });

    test('add a new Report - fail', async () => {
        const { testResults, ...reportWithoutTestResults } = reportToAdd;
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send(reportWithoutTestResults), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//missing testResults
    });

    test('update a Report - happy flow', async () => {
        const newObj = {
            testName: 'new',
            batteryPN: 'new',
            batterySN: 'new',
            notes: 'new',
            customer: 'new',
            workOrderNumber: 'new',
            approvedBy: 'new',
            conductedBy: 'new',
            cellSupplier: 'new',
            cellBatch: 'new',
            ubaSN: 'new',//will be ignored
            ignoredParam: 'something',
            status: 1,
        };
        response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.updateReportApi + "/" + reportIdAdded)
            .send(newObj), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        response = await withTimeout(request(global.__SERVER__).post(APIS.getReportsApi).send(searchBody), AWAIT_TIMEOUT);
        
        const obj2 = response.body.rows[0];
        expect(obj2).not.toHaveProperty('ignoredParam');
        expect(obj2.ubaSN).toBe(reportToAdd.ubaSN);
        
        const excludedKeys = ['ubaSN', 'ignoredParam'];
        const mismatches = Object.keys(newObj)
            .filter(key => !excludedKeys.includes(key)) // Exclude specified keys
            .reduce((result, key) => {
                if (newObj[key] !== obj2[key]) {
                    result.push({ key, newObjValue: newObj[key], obj2Value: obj2[key] });
                }
                return result;
            }, []);
        expect(mismatches).toHaveLength(0);
    });

    test('update a Report - fail', async () => {
        response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.updateReportApi + "/" + reportIdAdded)
            .send({
                notExist: 'notExist',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//nothing to update
        
        response = await withTimeout(request(global.__SERVER__)
            .patch(APIS.updateReportApi + "/" + 'notexist')
            .send({
                cellBatch: 'something',
            }), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);//Report not exists
    });

    // co-pilot-not-null-constraint
    test('co-pilot-not-null-constraint', async () => {
        for (const field of reportModel.createProperties) {
            if (field === 'notes' || field === 'customer' || field === 'workOrderNumber' || field === 'approvedBy' || field === 'conductedBy' || field === 'cellSupplier' || field === 'cellBatch') continue; // nullable fields
            const reportCopy = { ...reportToAdd };
            delete reportCopy[field];
            const response = await withTimeout(request(global.__SERVER__)
                .post(APIS.createReportApi)
                .send(reportCopy), AWAIT_TIMEOUT);
            expect(response.status).toBe(500);
        }
    });

    // co-pilot-varchar-max-length
    test('co-pilot-varchar-max-length', async () => {
        const longString = 'x'.repeat(300);
        const tooLong = { ...reportToAdd, testName: longString, batteryPN: longString, batterySN: longString };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send(tooLong), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-unique-constraint
    test('co-pilot-unique-constraint', async () => {
        await withTimeout(request(global.__SERVER__).post(APIS.createReportApi).send(reportToAdd), AWAIT_TIMEOUT);
        const response = await withTimeout(request(global.__SERVER__).post(APIS.createReportApi).send(reportToAdd), AWAIT_TIMEOUT);
        expect([409, 500]).toContain(response.status);
    });

    // co-pilot-invalid-data-types
    test('co-pilot-invalid-data-types', async () => {
        const invalidReport = { ...reportToAdd, noCellSerial: 'not-a-number', noCellParallel: 'not-a-number' };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send(invalidReport), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-get-nonexistent-report
    test('co-pilot-get-nonexistent-report', async () => {
        const response = await withTimeout(request(global.__SERVER__).post(APIS.getReportsApi).send({ ...searchBody, filters: { ...searchBody.filters, batterySN: 'notexist' } }), AWAIT_TIMEOUT);
        expect(response.status).toBe(200);
        expect(response.body.rows.length).toBe(0);
    });

    // co-pilot-empty-payload
    test('co-pilot-empty-payload', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send({}), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-invalid-json-plan
    test('co-pilot-invalid-json-plan', async () => {
        const invalidPlan = { ...reportToAdd, plan: 'not-a-json' };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send(invalidPlan), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

    // co-pilot-invalid-status-enum
    test('co-pilot-invalid-status-enum', async () => {
        const invalidStatus = { ...reportToAdd, status: 999 };
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.createReportApi)
            .send(invalidStatus), AWAIT_TIMEOUT);
        expect(response.status).toBe(500);
    });

});