const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { ubaDeviceModel, runningTestsModel, instantTestResultsModel, reportModel } = require('../models');
const {status,TEST_ROUTINE_CHANNELS, isTestRunning, isTestInPending, APIS} = require('../utils/constants');
const { clearMemInServer } = require('./testHelper');

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
        fwVersion: "7.5.3.0",
        hwVersion: "5.2",
    };
    const ubaDeviceToAdd2 = {
        ubaSN: "14567",
        name: "uba6-_1-7",
        ubaChannel: "AB",
        machineMac: machineToAdd.mac,
        comPort: "COM7",
        address: "967897",
        fwVersion: "7.5.3.0",
        hwVersion: "5.2",
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
        plan: [{"id": 0, "type": "charge", "cRate": 0.74, "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -201, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": "666:absoluteMah", "isCollapsed": false, "chargeCurrent": "3000:absoluteMa", "chargePerCell": 4, "cutOffCurrent": "50:absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 1, "type": "discharge", "source": "internal", "maxTemp": 60, "maxTime": "01:01:01", "minTemp": -20, "goToStep": null, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": 2.5, "isChargeLimit": true, "dischargeLimit": "7:absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": "3:absoluteMa", "isDischargeLimit": true}, {"id": 2, "type": "delay", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": null, "waitTemp": 7, "delayTime": "01:01:01", "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": null, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}, {"id": 3, "type": "loop", "source": "internal", "maxTemp": null, "maxTime": null, "minTemp": null, "goToStep": 0, "waitTemp": null, "delayTime": null, "isMaxTemp": true, "isMaxTime": true, "isMinTemp": true, "repeatStep": 4, "chargeLimit": ":absoluteMah", "isCollapsed": false, "chargeCurrent": ":absoluteMa", "chargePerCell": 0, "cutOffCurrent": ":absoluteMa", "cutOffVoltage": null, "isChargeLimit": true, "dischargeLimit": ":absoluteMah", "isCutOffCurrent": true, "isCutOffVoltage": true, "dischargeCurrent": ":absoluteMa", "isDischargeLimit": true}],
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
        await sqlValidateAmounts(runningTestsModel.tableName, 3);//there are 3 because ubaDeviceToAdd2 has 2 channels
        await sqlValidateAmounts(instantTestResultsModel.tableName, 0);
    });

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));//waiting for winston server logs to finish
    });
    afterEach(async () => {
        await clearMemInServer();
    });

    test('start runningTest', async () => {
        let ubaDevices = await validateRunningTests(3, 0, 0);
        let runningTestOfDevice1 = ubaDevices.find(obj => obj.ubaSN===ubaDeviceToAdd1.ubaSN);
        let pendingTasksRes = await request(global.__SERVER__).get(APIS.pendingTasksApi + '?machineMac=' + machineToAdd.mac);
        expect(pendingTasksRes.body.pendingRunningTests.length).toBe(0);
        expect(pendingTasksRes.body.pendingReports.length).toBe(0);

        //start a single channel test
        await startRunningTest(runningTestOfDevice1.runningTestID, runningTestOfDevice1.ubaSN, runningTestOfDevice1.channel);
        ubaDevices = await validateRunningTests(2, 1, 1);
        pendingTasksRes = await request(global.__SERVER__).get(APIS.pendingTasksApi + '?machineMac=' + machineToAdd.mac);
        expect(pendingTasksRes.body.pendingRunningTests.length).toBe(1);
        expect(pendingTasksRes.body.pendingReports.length).toBe(1);
        expect(pendingTasksRes.body.pendingRunningTests[0].reportId).toBe(pendingTasksRes.body.pendingReports[0].id);
        expect(pendingTasksRes.body.pendingReports[0].pendingRunningTestId).toBe(pendingTasksRes.body.pendingRunningTests[0].id);

        runningTestOfDevice1 = ubaDevices.find(obj => obj.ubaSN===ubaDeviceToAdd1.ubaSN);
        const allPending = await getAllPendingRunningTests();
        expect(allPending.length).toBe(1);
        expect(allPending[0].channel).toBe(runningTestOfDevice1.channel);
        expect(allPending[0].status).toBe(status.PENDING_RUNNING);
        expect(allPending[0].ubaSN).toBe(runningTestOfDevice1.ubaSN);
        expect(allPending[0].id).toBe(runningTestOfDevice1.runningTestID);
        expect(allPending[0].machineMac).toBe(machineToAdd.mac);
        expect(allPending[0].testRoutineChannels).toBe(TEST_ROUTINE_CHANNELS.A_AND_B);
        const [instantTestResultsRows] = await connection.query(`SELECT * FROM \`${instantTestResultsModel.tableName}\`;`);
        expect(instantTestResultsRows.length).toBe(0);
        
        //should be 1 report without testResults
        let [reportsRows] = await connection.query(`SELECT * FROM \`${reportModel.tableName}\`;`);
        expect(reportsRows.length).toBe(1);
        verifyReportObj(reportsRows[0]);

        let testResultsRes = await request(global.__SERVER__).post(APIS.reportsGraphApi).send([reportsRows[0].id]);
        expect(testResultsRes.body[0].reportID).toBe(reportsRows[0].id);
        expect(testResultsRes.body[0].testResults.length).toBe(0);

        //now updating the report with status and testResults
        const reportUpdateObj = { status: 0x0008, testResults: [{"current": 0.343692, "voltage": 14.4064, "timestamp": 20, "temperature": ""},{"current": 0.143692, "voltage": 12.4064, "timestamp": 40, "temperature": ""}] };
        const updateReportRes = await request(global.__SERVER__)
                    .patch(APIS.updateReportApi + "/" + reportsRows[0].id)
                    .send(reportUpdateObj);
        expect(updateReportRes.status).toBe(200);
        //verify that update worked
        [reportsRows] = await connection.query(`SELECT * FROM \`${reportModel.tableName}\`;`);
        expect(reportsRows[0].status).toBe(0x0008);
        expect(reportsRows[0].timeOfTest).toBe('00:00:40');
        testResultsRes = await request(global.__SERVER__).post(APIS.reportsGraphApi).send([reportsRows[0].id]);
        expect(testResultsRes.body[0].reportID).toBe(reportsRows[0].id);
        expect(testResultsRes.body[0].testResults.length).toBe(2);
        
        pendingTasksRes = await request(global.__SERVER__).get(APIS.pendingTasksApi + '?machineMac=' + machineToAdd.mac);
        expect(pendingTasksRes.body.pendingRunningTests.length).toBe(1);
        expect(pendingTasksRes.body.pendingReports.length).toBe(0);//because report was marked as finished
        expect(pendingTasksRes.body.pendingRunningTests[0].reportId).toBeUndefined();
        
        //running a and b channels together
        let runningTestOfDevice2 = ubaDevices.filter(obj => obj.ubaSN===ubaDeviceToAdd2.ubaSN);
        await startRunningTest(runningTestOfDevice2[0].runningTestID, runningTestOfDevice2[0].ubaSN, runningTestOfDevice2[0].channel, runningTestOfDevice2[1].channel);
        pendingTasksRes = await request(global.__SERVER__).get(APIS.pendingTasksApi + '?machineMac=' + machineToAdd.mac);
        expect(pendingTasksRes.body.pendingRunningTests.length).toBe(3);
        expect(pendingTasksRes.body.pendingReports.length).toBe(2);//because report was marked as finished
        ubaDevices = await validateRunningTests(0, 3, 3);
        
        await sqlValidateAmounts(runningTestsModel.tableName, 3);
        await sqlValidateAmounts(reportModel.tableName, 3);

        runningTestOfDevice1 = ubaDevices.find(obj => obj.ubaSN===ubaDeviceToAdd1.ubaSN);//because when started in first time it was deleted and recreated
        await changeRunningTestStatus(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B, status.RUNNING);//in order to be able to pause
        ubaDevices = await validateRunningTests(0, 3, 2);
        //pause 1 test
        await pendingPauseRunningTest(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B);
        ubaDevices = await validateRunningTests(0, 3, 3);
        
        await changeRunningTestStatus(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B, status.PAUSED);//fully paused for resume
        //resume 1 test
        await pendingResumeRunningTest(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B);
        ubaDevices = await validateRunningTests(0, 3, 3);

        await changeRunningTestStatus(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B, status.RUNNING);//fully started for doing stop
        //stop 1 test
        await pendingStopRunningTest(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B);
        await changeRunningTestStatus(runningTestOfDevice1.runningTestID, ubaDeviceToAdd1.ubaSN, TEST_ROUTINE_CHANNELS.A_OR_B, status.STOPPED);//fully stopped
        ubaDevices = await validateRunningTests(0, 2, 2);

        //now going to pause,resume, stop the parallel test
        runningTestOfDevice2 = ubaDevices.filter(obj => obj.ubaSN===ubaDeviceToAdd2.ubaSN);//because when started in first time it was deleted and recreated
        await changeRunningTestStatus(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B, status.RUNNING);//in order to be able to pause
        
        //pause 2 tests
        await pendingPauseRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        ubaDevices = await validateRunningTests(0, 2, 2);
        
        await changeRunningTestStatus(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B, status.PAUSED);//fully paused for resume

        //resume 2 tests
        await pendingResumeRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        ubaDevices = await validateRunningTests(0, 2, 2);

        await changeRunningTestStatus(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B, status.RUNNING);//fully started for doing stop
        
        //stop 2 tests
        await pendingStopRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        await changeRunningTestStatus(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B, status.STOPPED);//fully stopped

        ubaDevices = await validateRunningTests(0, 0, 0);
        
        await pendingConfirmRunningTest(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B);
        ubaDevices = await validateRunningTests(0, 2, 2);
        await changeRunningTestStatus(runningTestOfDevice2[0].runningTestID, ubaDeviceToAdd2.ubaSN, TEST_ROUTINE_CHANNELS.A_AND_B, status.STANDBY);

        ubaDevices = await validateRunningTests(2, 0, 0);

        //check graph data for the instantTestResults
        response = await request(global.__SERVER__).get(APIS.instantTestResultsApi + "/" + runningTestOfDevice1.runningTestID);
        expect(response.body.length).toBe(0);

        //test add instantTestResults
        let arr = [
                {runningTestID: runningTestOfDevice1.runningTestID, timestamp: '2024-08-15 09:01:07.001', testState: 'charging', testCurrentStep: 1, 
                voltage: 3.5, current: 2.4, temp: 25.5, capacity: 0, error: undefined, isLogData: 1},
                {runningTestID: runningTestOfDevice1.runningTestID, timestamp: '2024-08-15 09:01:28.000', testState: 'charging', testCurrentStep: 1, 
                    voltage: 3.6, current: 2.5, temp: 26.6, capacity: 0, error: undefined, isLogData: 1},
                {runningTestID: runningTestOfDevice1.runningTestID, timestamp: '2024-08-15 09:01:29.000', testState: 'charging', testCurrentStep: 1, 
                    voltage: 3.6, current: 2.55, temp: 26.6, capacity: 0, error: undefined, isLogData: 0},//last one isnt for adding to db
                {runningTestID: runningTestOfDevice2[0].runningTestID, timestamp: '2024-08-15 09:01:30.000', testState: 'charging', testCurrentStep: 1, 
                    voltage: 3.6, current: 2.6, temp: 26.6, capacity: 0, error: undefined, isLogData: 1},
            ];
        response = await request(global.__SERVER__).post(APIS.instantTestResultsApi).send(arr);
        expect(response.status).toBe(201);
        await sqlValidateAmounts(instantTestResultsModel.tableName, 3);
        response = await request(global.__SERVER__).get(APIS.instantTestResultsApi + "/" + runningTestOfDevice1.runningTestID);
        expect(response.body.length).toBe(2);
        response = await request(global.__SERVER__).get(APIS.instantTestResultsApi + "/" + runningTestOfDevice2[0].runningTestID);
        expect(response.body.length).toBe(1);
        response = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        expect(response.body.ubaTotal.connected).toBe(2);
        const enrichedRunningTest1 = response.body.ubaDevices.find(obj => obj.runningTestID===runningTestOfDevice1.runningTestID);
        const enrichedRunningTest2 = response.body.ubaDevices.find(obj => obj.runningTestID===runningTestOfDevice2[0].runningTestID);
        let enrichedRunningTest3 = response.body.ubaDevices.find(obj => obj.runningTestID===runningTestOfDevice2[1].runningTestID);

        expect(new Date(enrichedRunningTest1.lastInstantResultsTimestamp).getTime()).toBe(new Date('2024-08-15 09:01:29.000 UTC').getTime());
        expect(enrichedRunningTest1.ubaDeviceConnectedTimeAgoMs < 60000).toBeTruthy();
        expect(enrichedRunningTest1.current).toBe(2.55);

        expect(new Date(enrichedRunningTest2.lastInstantResultsTimestamp).getTime()).toBe(new Date('2024-08-15 09:01:30.000 UTC').getTime());
        expect(enrichedRunningTest2.ubaDeviceConnectedTimeAgoMs < 60000).toBeTruthy();
        expect(enrichedRunningTest2.current).toBe(2.6);

        expect(enrichedRunningTest3.lastInstantResultsTimestamp).toBe(null);
        expect(enrichedRunningTest3.ubaDeviceConnectedTimeAgoMs).toBe(null);
        expect(enrichedRunningTest3.current).toBe(null);

        //now send instantTestResults to enrichedRunningTest3 which are the first ones and are with isLogData 0
        arr = [ {runningTestID: runningTestOfDevice2[1].runningTestID, timestamp: '2024-08-15 09:01:31.000', testState: 'charging', testCurrentStep: 1, 
                    voltage: 3.1, current: 2.1, temp: 26.1, capacity: 0, error: undefined, isLogData: 0},
                {runningTestID: runningTestOfDevice2[1].runningTestID, timestamp: '2024-08-15 09:01:32.000', testState: 'charging', testCurrentStep: 1, 
                    voltage: 3.2, current: 2.2, temp: 26.2, capacity: 0, error: undefined, isLogData: 0}
            ];
        response = await request(global.__SERVER__).post(APIS.instantTestResultsApi).send(arr);
        expect(response.status).toBe(201);
        await sqlValidateAmounts(instantTestResultsModel.tableName, 4);//in first time, only one more added because both are isLogData 0 and only the first one is added
        response = await request(global.__SERVER__).get(APIS.instantTestResultsApi + "/" + runningTestOfDevice2[1].runningTestID);
        expect(response.body.length).toBe(1);
        response = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        enrichedRunningTest3 = response.body.ubaDevices.find(obj => obj.runningTestID===runningTestOfDevice2[1].runningTestID);
        expect(new Date(enrichedRunningTest3.lastInstantResultsTimestamp).getTime()).toBe(new Date('2024-08-15 09:01:32.000 UTC').getTime());
        expect(enrichedRunningTest3.ubaDeviceConnectedTimeAgoMs < 60000).toBeTruthy();
        expect(enrichedRunningTest3.current).toBe(2.2);

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
        await sqlValidateAmounts(reportModel.tableName, 3);

    });

    /*test('executeMe', async () => {
        const json = [{"current": 0.450929, "voltage": 3.52155, "timestamp": 20, "temperature": ""}];
        json.forEach((item, index) => {
            item.stepIndex = index<1500? 0 : 1;
            item.planIndex = index<1500? 0 : 1;
            item.temperature = index;
        });
        console.log('Starting running test with data: ', JSON.stringify(json));
    });*/

    const validateRunningTests = async (amountOfStandBy, amountOfRunning, amountOfPending) => {
        let { body: {ubaDevices, ubaTotal} } = await request(global.__SERVER__).get(APIS.ubaDevicesApi);
        let standByTests = ubaDevices.filter(obj => obj.status===status.STANDBY);
        let runningStatusTests = ubaDevices.filter(obj => isTestRunning(obj.status));
        let pendingStatusTest = ubaDevices.filter(obj => isTestInPending(obj.status));
        expect(pendingStatusTest.length).toBe(amountOfPending);
        expect(standByTests.length).toBe(amountOfStandBy);
        expect(runningStatusTests.length).toBe(amountOfRunning);
        expect(ubaTotal.running).toBe(amountOfRunning);

        const allPending = await getAllPendingRunningTests();
        expect(allPending.length).toBe(amountOfPending);
        return ubaDevices;
    }

    const startRunningTest = async (runningTestID, ubaSN, channel1, channel2) => {
        let obj = {//TODO! check ubaTotal.connected
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

    const pendingPauseRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.changeRunningTestStatusApi).send({runningTestID, testRoutineChannels, ubaSN, newTestStatus: status.PENDING_PAUSE});
        expect(response.status).toBe(200);
    };

    const pendingResumeRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.changeRunningTestStatusApi).send({runningTestID, testRoutineChannels, ubaSN, newTestStatus: status.PENDING_RUNNING});
        expect(response.status).toBe(200);
    };

    const pendingConfirmRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.changeRunningTestStatusApi).send({runningTestID, testRoutineChannels, ubaSN, newTestStatus: status.PENDING_STANDBY});
        expect(response.status).toBe(200);
    };

    const pendingStopRunningTest = async (runningTestID, ubaSN, testRoutineChannels) => {
        let response = await request(global.__SERVER__).patch(APIS.changeRunningTestStatusApi).send({runningTestID, testRoutineChannels, ubaSN, newTestStatus: status.PENDING_STOP});
        expect(response.status).toBe(200);
    };

    const changeRunningTestStatus = async (runningTestID, ubaSN, testRoutineChannels, newTestStatus) => {
        let response = await request(global.__SERVER__).patch(APIS.changeRunningTestStatusApi).send({runningTestID, testRoutineChannels, ubaSN, newTestStatus});
        expect(response.status).toBe(200);
    };

    const sqlValidateAmounts = async (tableName, amount) => {
        const [rows] = await connection.query(`SELECT * FROM \`${tableName}\`;`);
        expect(rows.length).toBe(amount);
    };

    const getAllPendingRunningTests = async () => {
        const response = await request(global.__SERVER__).get(APIS.getAllPendingRunningTestsApi);
        expect(response.status).toBe(200);
        return response.body;
    }

    const verifyReportObj = (reportObj1) => {
        expect(reportObj1.batteryPN).toBe("901-24000365-1S2P-M1");
        expect(reportObj1.batterySN).toBe("def5346536g");
        expect(reportObj1.cellPN).toBe("ABLP75100250H300");
        expect(reportObj1.cellSupplier).toBe("cellSupplier");
        expect(reportObj1.cellBatch).toBe("18K021");
        expect(reportObj1.testRoutineChannels).toBe("A-and-B");
        expect(reportObj1.timeOfTest).toBeNull();
        expect(reportObj1.status).toBe(288);
        expect(reportObj1.ubaSN).toBe("14565");
        expect(reportObj1.testName).toBe("C-D-C_3A_CUT OFF 2.5V");
        expect(reportObj1.notes).toBe("notes");
        expect(reportObj1.customer).toBe("customer 0");
        expect(reportObj1.workOrderNumber).toBe("fd");
        expect(reportObj1.approvedBy).toBe("approvedBy");
        expect(reportObj1.conductedBy).toBe("conductedBy");
        expect(reportObj1.channel).toBe("A");
        expect(reportObj1.maxPerBattery).toBe('8.7453');
        expect(reportObj1.ratedBatteryCapacity).toBe('4080.00000');
        expect(reportObj1.noCellParallel).toBe(2);
        expect(reportObj1.noCellSerial).toBe(1);
        expect(reportObj1.timestampStart).not.toBeNull();
        expect(reportObj1.id).not.toBeNull();
        expect(reportObj1.chemistry).toBe("Li-Ion Polymer");
        expect(reportObj1.machineName).toBe("Lab-1");
        expect(reportObj1.machineMac).toBe("00-10-FA-63-38-4A");
    };

});