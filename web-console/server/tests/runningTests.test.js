const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { ubaDeviceModel, runningTestsModel, instantTestResultsModel } = require('../models');
const {status,TEST_ROUTINE_CHANNELS, isTestRunning, isTestInPending, APIS} = require('../utils/constants');

const { changeTestStatus, } = require('../services/runningTestService');

describe('Machine API Tests', () => {
    const machineToAdd = {
        mac: '00-10-FA-63-38-4A',
        name: 'Lab-1',
        ip: '141.191.237.16',
    };
    const ubaDeviceToAdd1 = {
        ubaSN: "14565",
        name: "uba6-_1-5",
        ubaChannel: "A",
        machineMac: machineToAdd.mac,
        comPort: "COM5",
        address: "967895",
        isConnected: 1,
    };
    const ubaDeviceToAdd2 = {
        ubaSN: "14567",
        name: "uba6-_1-7",
        ubaChannel: "AB",
        machineMac: machineToAdd.mac,
        comPort: "COM7",
        address: "967897",
        isConnected: 0,
    };
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
    const testRoutineToAdd = {
        testName: 'C-D-C_3A_CUT OFF 2.5V',
        isLocked: 0,
        batteryPN: '901-24000365-1S2P-M1',
        batterySN: 'def5346536g',
        cellPN: cellToAdd.itemPN,
        noCellSerial: 1,
        noCellParallel:2 ,
        maxPerBattery: 8.7453,
        ratedBatteryCapacity: 4080.00000,
        channel: 'A-and-B',
        notes: 'notes',
        customer: 'customer 0',
        workOrderNumber: 'fd',
        approvedBy: 'approvedBy',
        conductedBy: 'conductedBy',
        cellSupplier: 'cellSupplier',
        cellBatch: '18K021',
        plan: [{"id": 0, "type": "charge", "cRate": 0.74, "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -201, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": "666:absoluteMah", "isCollapsed": false, "chargeCurrent": "3000:absoluteMa", "chargePerCell": 4, "cutOffCurrent": "50:absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 1, "type": "discharge", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -20, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": "2.5", "isChargeLimit": true, "dischargeLimit": "7:absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": "3:absoluteMa", "isDischargeLimit": true}, {"id": 2, "type": "delay", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": null, "waitTemp": 7, "delayTime": "01:01:01", "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 3, "type": "loop", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": 0, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": 4, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}],
        ignoredParam: 'something',
    };

    let connection;

    beforeAll(async () => {
        await runSchema();
        connection = await mysql.createConnection(global.__MYSQL_CONFIG__);

        let response = await request(global.__SERVER__).post(APIS.machinesApi).send(machineToAdd);
        expect(response.status).toBe(201);

        response = await request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd1);
        expect(response.status).toBe(201);
        response = await request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd2);
        expect(response.status).toBe(201);
        response = await request(global.__SERVER__).post(APIS.cellsApi).send(cellToAdd);
        expect(response.status).toBe(201);
        response = await request(global.__SERVER__).post(APIS.testRoutinesApi).send(testRoutineToAdd);
        expect(response.status).toBe(201);

        await sqlValidateAmounts(ubaDeviceModel.tableName, 2);
        await sqlValidateAmounts(runningTestsModel.tableName, 3);
        await sqlValidateAmounts(instantTestResultsModel.tableName, 0);
    });

    

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));//waiting for winston server logs to finish
    });

    test('start runningTest', async () => {
        let ubaDevices = await validateRunningTests(3, 0, 0);
        let runningTestOfDevice1 = ubaDevices.find(obj => obj.ubaSN===ubaDeviceToAdd1.ubaSN);
        //start a single channel test
        await startRunningTest(runningTestOfDevice1.runningTestID, runningTestOfDevice1.ubaSN, runningTestOfDevice1.channel);
        ubaDevices = await validateRunningTests(2, 1, 1);
        
        const [instantTestResultsRows] = await connection.query(`SELECT * FROM \`${instantTestResultsModel.tableName}\`;`);
        expect(instantTestResultsRows.length).toBeGreaterThan(0);//this is only because mock data entered and will be deleted in the future
        
        //running a and b channels together
        let runningTestOfDevice2 = ubaDevices.filter(obj => obj.ubaSN===ubaDeviceToAdd2.ubaSN);
        await startRunningTest(runningTestOfDevice2[0].runningTestID, runningTestOfDevice2[0].ubaSN, runningTestOfDevice2[0].channel, runningTestOfDevice2[1].channel);
        ubaDevices = await validateRunningTests(0, 3, 3);
        
        await sqlValidateAmounts(runningTestsModel.tableName, 3);

        runningTestOfDevice1 = ubaDevices.find(obj => obj.ubaSN===ubaDeviceToAdd1.ubaSN);//because when started in first time it was deleted and recreated
        await changeTestStatus(runningTestOfDevice1.runningTestID, status.RUNNING);//in order to be able to pause
        ubaDevices = await validateRunningTests(0, 3, 2);
        //pause 1 test
        await pauseRunningTest(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B);
        ubaDevices = await validateRunningTests(0, 3, 3);
        
        await changeTestStatus(runningTestOfDevice1.runningTestID, status.PAUSED);//fully paused for resume
        //resume 1 test
        await resumeRunningTest(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B);
        ubaDevices = await validateRunningTests(0, 3, 3);

        await changeTestStatus(runningTestOfDevice1.runningTestID, status.RUNNING);//fully started for doing stop
        //stop 1 test
        await stopRunningTest(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B);
        await changeTestStatus(runningTestOfDevice1.runningTestID, status.STOPPED);//fully stopped
        ubaDevices = await validateRunningTests(0, 2, 2);

        //now going to pause,resume, stop the parallel test
        runningTestOfDevice2 = ubaDevices.filter(obj => obj.ubaSN===ubaDeviceToAdd2.ubaSN);//because when started in first time it was deleted and recreated
        await changeTestStatus(runningTestOfDevice2[0].runningTestID, status.RUNNING);//in order to be able to pause
        await changeTestStatus(runningTestOfDevice2[1].runningTestID, status.RUNNING);
        //pause 2 tests
        await pauseRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        ubaDevices = await validateRunningTests(0, 2, 2);
        
        await changeTestStatus(runningTestOfDevice2[0].runningTestID, status.PAUSED);//fully paused for resume
        await changeTestStatus(runningTestOfDevice2[1].runningTestID, status.PAUSED);
        //resume 2 tests
        await resumeRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        ubaDevices = await validateRunningTests(0, 2, 2);

        await changeTestStatus(runningTestOfDevice2[0].runningTestID, status.RUNNING);//fully started for doing stop
        await changeTestStatus(runningTestOfDevice2[1].runningTestID, status.RUNNING);
        //stop 2 tests
        await stopRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        await changeTestStatus(runningTestOfDevice2[0].runningTestID, status.STOPPED);//fully stopped
        await changeTestStatus(runningTestOfDevice2[1].runningTestID, status.STOPPED);//fully stopped
        ubaDevices = await validateRunningTests(0, 0, 0);
        
        await confirmRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        ubaDevices = await validateRunningTests(0, 2, 2);
        await changeTestStatus(runningTestOfDevice2[0].runningTestID, status.STANDBY);
        await changeTestStatus(runningTestOfDevice2[1].runningTestID, status.STANDBY);
        ubaDevices = await validateRunningTests(2, 0, 0);

        //check graph data for the instantTestResults
        response = await request(global.__SERVER__).get(APIS.graphDataApi + "/" + runningTestOfDevice1.runningTestID);
        const amountOfData = response.body.length;
        expect(amountOfData).toBeGreaterThan(0);

        //test add instantTestResults
        const arr = [
                {runningTestID: runningTestOfDevice1.runningTestID, timestamp: '2024-08-15 09:01:07.001', testState: 'charging', testCurrentStep: 1, 
                voltage: 3.5, current: 2.5, temp: 25.5, capacity: 0, error: undefined},
                {runningTestID: runningTestOfDevice1.runningTestID, timestamp: '2024-08-15 09:01:27.000', testState: 'charging', testCurrentStep: 1, 
                    voltage: 3.6, current: 2.6, temp: 26.6, capacity: 0, error: undefined},
            ];
        response = await request(global.__SERVER__).post(APIS.graphDataApi).send(arr);
        expect(response.status).toBe(201);
        response = await request(global.__SERVER__).get(APIS.graphDataApi + "/" + runningTestOfDevice1.runningTestID);
        expect(response.body.length).toBe(amountOfData + 2);
        
        //check that when deleting ubaDevice, the running tests are deleted and instantTestResults are deleted
        response = await request(global.__SERVER__).delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd1.ubaSN);
        expect(response.status).toBe(204);
        response = await request(global.__SERVER__).delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd2.ubaSN);
        expect(response.status).toBe(204);
        let { body: {ubaDevices1, ubaTotal1} } = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        expect(ubaDevices1).toBe(undefined);
        await sqlValidateAmounts(ubaDeviceModel.tableName, 0);
        await sqlValidateAmounts(runningTestsModel.tableName, 0);
        await sqlValidateAmounts(instantTestResultsModel.tableName, 0);

    });

    const validateRunningTests = async (amountOfStandBy, amountOfRunning, amountOfPending) => {
        let { body: {ubaDevices, ubaTotal} } = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        let standByTests = ubaDevices.filter(obj => obj.status===status.STANDBY);
        let runningStatusTests = ubaDevices.filter(obj => isTestRunning(obj.status));
        let pendingStatusTest = ubaDevices.filter(obj => isTestInPending(obj.status));
        expect(pendingStatusTest.length).toBe(amountOfPending);
        expect(standByTests.length).toBe(amountOfStandBy);
        expect(runningStatusTests.length).toBe(amountOfRunning);
        expect(ubaTotal.running).toBe(amountOfRunning);

        return ubaDevices;
    }

    const startRunningTest = async (runningTestID, ubaSN, channel1, channel2) => {
        let obj = {//TODO!!!! check ubaTotal.connected
            ...testRoutineToAdd,
            id: runningTestID,
            ubaSNs: [{
                "ubaSN": ubaSN,
                "channel": channel1,
              },
            ],
        };
        if(channel2) {
            obj.ubaSNs.push({
            "ubaSN": ubaSN,
            "channel": channel2,
          });
        }
        let response = await request(global.__SERVER__).post(APIS.startTestApi).send(obj);
        expect(response.status).toBe(200);
    }

    const pauseRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.pauseTestApi).send({runningTestID, testRoutineChannels, ubaSN});
        expect(response.status).toBe(200);
    };

    const resumeRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.resumeTestApi).send({runningTestID, testRoutineChannels, ubaSN});
        expect(response.status).toBe(200);
    };

    const confirmRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.confirmTestApi).send({runningTestID, testRoutineChannels, ubaSN});
        expect(response.status).toBe(200);
    };

    const stopRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.stopTestApi).send({runningTestID, testRoutineChannels, ubaSN});
        expect(response.status).toBe(200);
    };

    const sqlValidateAmounts = async (tableName, amount) => {
        const [rows] = await connection.query(`SELECT * FROM \`${tableName}\`;`);
        expect(rows.length).toBe(amount);
    };

});