const mysql = require('mysql2/promise');
const request = require('supertest');
const runSchema = require('./prepareDb');
const { ubaDeviceModel, runningTestsModel, instantTestResultsModel } = require('../models');
const { UI_FLOWS, UBA_DEVICE_ACTIONS, PENDING_TASKS_TYPES, ACTION_RESULT } = require('../utils/ubaCommunicatorHelper');
const { clearMemInServer } = require('./testHelper');
const { APIS } = require('../utils/constants');

describe('Machine API Tests', () => {
    const machineToAdd = {
        mac: '00-10-FA-63-38-4A',
        name: 'Lab-1',
        ip: '141.191.237.16',
    };
    const ubaDeviceToAdd1 = {
        ubaSN: "123456",
        name: "uba1-_1-4",
        ubaChannel: "AB",
        machineMac: machineToAdd.mac,
        comPort: "COM4",
        address: "944444",
        fwVersion: "7.5.3.0",
        hwVersion: "5.2",
    };

    let connection;

    beforeAll(async () => {
        await runSchema();
        connection = await mysql.createConnection(global.__MYSQL_CONFIG__);

        let response = await request(global.__SERVER__)
            .post(APIS.machinesApi)
            .send(machineToAdd);
        expect(response.status).toBe(201);

        response = await request(global.__SERVER__)
                    .post(APIS.ubaDevicesApi)
                    .send(ubaDeviceToAdd1);
        expect(response.status).toBe(201);

        //just in order to release the ADD_TO_WATCH_LIST
        const pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd1, actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);
    });

    afterAll(async () => {
        console.log('Machine Test suite finished');
        if(connection) await connection.end();
        await new Promise(resolve => setTimeout(resolve, 500));//waiting for winston server logs to finish
    });
    afterEach(async () => {
        await clearMemInServer();
    });

    test('pendingTasksExecuted fail flows', async () => {
        let pendingUbaDevice = {
            machineMac: ubaDeviceToAdd1.machineMac,
            comPort: ubaDeviceToAdd1.comPort,
            address: ubaDeviceToAdd1.address,
            ubaSN: ubaDeviceToAdd1.ubaSN,
        };
        const requestPromise = request(global.__SERVER__)
            .post(APIS.queryUbaDevicesApi)//this is like the web ui call
            .send(pendingUbaDevice)
            .then(res => {
                queryUbaDeviceRes = res;
            })
            .catch(err => {
                queryUbaDeviceError = err;
            });
        await new Promise(resolve => setTimeout(resolve, 1500));

        const pendingTask = { pendingUbaDevice: { ...pendingUbaDevice, fwVersion: "7.5.3.0", hwVersion: "5.2",
                ubaSN: pendingUbaDevice.ubaSN, ubaChannel: pendingUbaDevice.ubaChannel, name: pendingUbaDevice.name, action: UBA_DEVICE_ACTIONS.QUERY } };
        await expectedFailedExecutePendingTask(pendingTask, 'Missing required parameters.');
        pendingTask.pendingUbaDevice.actionResult = ACTION_RESULT.SUCCESS.code;
        pendingTask.pendingUbaDevice.ubaChannel = undefined;
        await expectedFailedExecutePendingTask(pendingTask, 'Missing parameters for SUCCESS uba device operation.');
        pendingTask.pendingUbaDevice.ubaChannel = ubaDeviceToAdd1.ubaChannel;
        await ubaDeviceExecutePendingTask(pendingTask);//in order to release the waiter for next tests

    });

    test('query fail flows ', async () => {
        let pendingUbaDevice = {
            machineMac: ubaDeviceToAdd1.machineMac,
            comPort: ubaDeviceToAdd1.comPort,
            address: ubaDeviceToAdd1.address,
            //ubaSN: "some new ubaSN",
        };

        await expectedFailedQueryUbaDevice(pendingUbaDevice, "ubaDeviceByConstraint already exists.");
        pendingUbaDevice.ubaSN = "not exist ubaSN";
        await expectedFailedQueryUbaDevice(pendingUbaDevice, "ubaSN does not exist.");

        const ubaDeviceToAdd = {
            ubaSN: "123211223",
            name: "uba21-_12-51",
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM4221",
            address: "1221234",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        await addUbaAndExecutePendingTask(ubaDeviceToAdd);
        pendingUbaDevice.ubaSN = ubaDeviceToAdd.ubaSN;
        await expectedFailedQueryUbaDevice(pendingUbaDevice, "ubaSN already exists.");

        pendingUbaDevice.machineMac = "not exist machineMac";
        await expectedFailedQueryUbaDevice(pendingUbaDevice, "Machine with mac");
        delete pendingUbaDevice.address;
        await expectedFailedQueryUbaDevice(pendingUbaDevice, "Missing parameters.");

        //flow of uba service cant find the ubaDevice and message returned to end user through web ui
        const pendingUbaDevice3 = {//a new uba that isnt in the system yet
            machineMac: machineToAdd.mac,
            comPort: "COM323",
            address: "1114423236",
        };
        let queryUbaDeviceRes, queryUbaDeviceError;
        const requestPromise = request(global.__SERVER__)
            .post(APIS.queryUbaDevicesApi)//this is like the web ui call
            .send(pendingUbaDevice3)
            .then(res => {
                queryUbaDeviceRes = res;
            })
            .catch(err => {
                queryUbaDeviceError = err;
            });
        await new Promise(resolve => setTimeout(resolve, 1500));
        await verifyAmountOfPendingTasks(machineToAdd.mac, 1);

        let pendingTask = { pendingUbaDevice: { ...pendingUbaDevice3, fwVersion: "7.5.3.0", hwVersion: "5.2", actionResult: ACTION_RESULT.UBA_DEVICE_NOT_FOUND.code, action: UBA_DEVICE_ACTIONS.QUERY} };
        await ubaDeviceExecutePendingTask(pendingTask);
        await new Promise(resolve => setTimeout(resolve, 1500));
        expect(queryUbaDeviceRes).toBeDefined();
        expect(queryUbaDeviceRes.status).toBe(400);
        expect(queryUbaDeviceRes.body.error).toContain(ACTION_RESULT.UBA_DEVICE_NOT_FOUND.message);
    });

    test('add uba flow - queryUbaDevice & pendingTasksExecuted & ADD_TO_WATCH_LIST full flow success', async () => {
        const pendingUbaDevice = {//a new uba that isnt in the system yet
            machineMac: machineToAdd.mac,
            comPort: "COM7",
            address: "967896",
        };
        const pendingExecutedRequestInfo = { ubaSN: "14566", ubaChannel: "AB", actionResult: ACTION_RESULT.SUCCESS.code, action: "query" };
        await addOrEditQueryUbaFlow(pendingUbaDevice, pendingExecutedRequestInfo);

        //add the uba device to the system
        const ubaDeviceToAdd = {
            ubaSN: pendingExecutedRequestInfo.ubaSN,
            name: "uba6-_1-5",
            ubaChannel: pendingExecutedRequestInfo.ubaChannel,
            machineMac: machineToAdd.mac,
            comPort: pendingUbaDevice.comPort,
            address: pendingUbaDevice.address,
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        await addUba(ubaDeviceToAdd);

        //check that the ubadevice will receive a pending task to add to watch list
        const pendingTasksRes = await verifyAmountOfPendingTasks(machineToAdd.mac, 1);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].machineMac).toBe(ubaDeviceToAdd.machineMac);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].comPort).toBe(ubaDeviceToAdd.comPort);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].address).toBe(ubaDeviceToAdd.address);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].action).toBe(UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].name).toBe(ubaDeviceToAdd.name);

        //ubaDevice reports that it executed the pending task
        const pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd, actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);

        await verifyAmountOfPendingTasks(machineToAdd.mac, 0);
    });

    test('edit uba flow - queryUbaDevice & pendingTasksExecuted & ADD_TO_WATCH_LIST & REMOVE_FROM_WATCH_LIST full flow success', async () => {
        const pendingUbaDevice = {
            machineMac: ubaDeviceToAdd1.machineMac,
            comPort: ubaDeviceToAdd1.comPort,
            address: ubaDeviceToAdd1.address,
            ubaSN: ubaDeviceToAdd1.ubaSN,
        };
        const pendingExecutedRequestInfo = { ubaSN: ubaDeviceToAdd1.ubaSN, ubaChannel: ubaDeviceToAdd1.ubaChannel, actionResult: ACTION_RESULT.SUCCESS.code, action: "query" };
        await addOrEditQueryUbaFlow(pendingUbaDevice, pendingExecutedRequestInfo);

        //change only the name of the uba device
        await editUba(ubaDeviceToAdd1.ubaSN, {name: 'changeOnlyName 123'});

        let pendingTasksRes = await verifyAmountOfPendingTasks(machineToAdd.mac, 1);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].machineMac).toBe(ubaDeviceToAdd1.machineMac);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].comPort).toBe(ubaDeviceToAdd1.comPort);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].address).toBe(ubaDeviceToAdd1.address);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].action).toBe(UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].name).toBe('changeOnlyName 123');

        //ubaDevice reports that it executed the pending task
        let pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd1, actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);
        await verifyAmountOfPendingTasks(machineToAdd.mac, 0);

        //now edit the port
        await editUba(ubaDeviceToAdd1.ubaSN, {comPort: 'COM11'});
        //verify that there are 2 pending tasks: one to remove from watch list and one to add to watch list
        pendingTasksRes = await verifyAmountOfPendingTasks(machineToAdd.mac, 2);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].machineMac).toBe(ubaDeviceToAdd1.machineMac);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].comPort).toBe(ubaDeviceToAdd1.comPort);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].address).toBe(ubaDeviceToAdd1.address);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].action).toBe(UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST);
        
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[1].machineMac).toBe(ubaDeviceToAdd1.machineMac);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[1].comPort).toBe('COM11');
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[1].address).toBe(ubaDeviceToAdd1.address);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[1].action).toBe(UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[1].name).toBe('changeOnlyName 123');//brought from db because was changed before already

        //mark them as executed
        pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd1, actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);
        pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd1, comPort: 'COM11', actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);

        await verifyAmountOfPendingTasks(machineToAdd.mac, 0);
    });

    test('delete uba flow - pendingTasksExecuted & REMOVE_FROM_WATCH_LIST full flow success', async () => {
        const ubaDeviceToAdd = {
            ubaSN: "234423",
            name: "uba2-_3-5",
            ubaChannel: "AB",
            machineMac: machineToAdd.mac,
            comPort: "COM44",
            address: "944444",
            fwVersion: "7.5.3.0",
            hwVersion: "5.2",
        };
        await addUbaAndExecutePendingTask(ubaDeviceToAdd);

        let response = await request(global.__SERVER__).delete(APIS.ubaDevicesApi + "/" + ubaDeviceToAdd.ubaSN);
        expect(response.status).toBe(204);
        let pendingTasksRes = await verifyAmountOfPendingTasks(machineToAdd.mac, 1);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].machineMac).toBe(ubaDeviceToAdd.machineMac);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].comPort).toBe(ubaDeviceToAdd.comPort);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].address).toBe(ubaDeviceToAdd.address);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].action).toBe(UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST);

        let pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd, actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.REMOVE_FROM_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);
    });

    const expectedFailedExecutePendingTask = async (pendingTask, expectedBody) => {
        const res = await request(global.__SERVER__).post(APIS.pendingTasksApi).send(pendingTask)
        expect(res.status).toBe(400);
        expect(res.body.error).toContain(expectedBody);
    }

    const addUbaAndExecutePendingTask = async (ubaDeviceToAdd) => {
        await addUba(ubaDeviceToAdd);
        let pendingTask = { pendingUbaDevice: { ...ubaDeviceToAdd, actionResult: ACTION_RESULT.SUCCESS.code, action: UBA_DEVICE_ACTIONS.ADD_TO_WATCH_LIST} };
        await ubaDeviceExecutePendingTask(pendingTask);
        await verifyAmountOfPendingTasks(ubaDeviceToAdd.machineMac, 0);
    };

    const expectedFailedQueryUbaDevice = async (pendingUbaDevice, expectedBody) => {
        const requestPromise = await request(global.__SERVER__).post(APIS.queryUbaDevicesApi).send(pendingUbaDevice);
        expect(requestPromise.status).toBe(400);
        expect(requestPromise.body.error).toContain(expectedBody);
    }


    const editUba = async (ubaSN, ubaDeviceToEdit) => {
        const response = await request(global.__SERVER__).patch(APIS.ubaDevicesApi + "/" + ubaSN).send(ubaDeviceToEdit);
        expect(response.status).toBe(200);
    }

     const addUba = async (ubaDeviceToAdd) => {
        const response = await request(global.__SERVER__).post(APIS.ubaDevicesApi).send(ubaDeviceToAdd);
        expect(response.status).toBe(201);
        expect(response.body.length).toBe(2);
    }

    const ubaDeviceExecutePendingTask = async (pendingTask) => {
        const pendingTaskExecutedRes = await request(global.__SERVER__).post(APIS.pendingTasksApi).send(pendingTask)
        expect(pendingTaskExecutedRes.status).toBe(200);
    }

    const verifyAmountOfPendingTasks = async (machineMac, pendingConnectionUbaDevicesNum) => {
        let pendingTasksRes = await request(global.__SERVER__).get(APIS.pendingTasksApi + '?machineMac=' + machineMac);
        expect(pendingTasksRes.status).toBe(200);
        expect(pendingTasksRes.body.pendingRunningTests.length).toBe(0);
        expect(pendingTasksRes.body.pendingReports.length).toBe(0);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices.length).toBe(pendingConnectionUbaDevicesNum);
        return pendingTasksRes;
    };

    const addOrEditQueryUbaFlow = async (pendingUbaDevice, pendingExecutedRequestInfo) => {
        await verifyAmountOfPendingTasks(machineToAdd.mac, 0);

        let queryUbaDeviceRes, queryUbaDeviceError;
        const requestPromise = request(global.__SERVER__)
            .post(APIS.queryUbaDevicesApi)//this is like the web ui call
            .send(pendingUbaDevice)
            .then(res => {
                queryUbaDeviceRes = res;
            })
            .catch(err => {
                queryUbaDeviceError = err;
            });
        await new Promise(resolve => setTimeout(resolve, 1500));
        
        //this is like the uba service call that happens every second
        pendingTasksRes = await verifyAmountOfPendingTasks(machineToAdd.mac, 1);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].machineMac).toBe(pendingUbaDevice.machineMac);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].comPort).toBe(pendingUbaDevice.comPort);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].address).toBe(pendingUbaDevice.address);
        expect(pendingTasksRes.body.pendingConnectionUbaDevices[0].action).toBe(UBA_DEVICE_ACTIONS.QUERY);

        const pendingTask = { pendingUbaDevice: { machineMac: machineToAdd.mac, address: pendingUbaDevice.address, comPort: pendingUbaDevice.comPort,
                ubaSN: pendingExecutedRequestInfo.ubaSN, ubaChannel: pendingExecutedRequestInfo.ubaChannel, name: "uba name", fwVersion: "7.5.3.0", hwVersion: "5.2",
                actionResult: pendingExecutedRequestInfo.actionResult, action: pendingExecutedRequestInfo.action } };
        //this is the uba service call that quries the uba device and then calls pendingTasksExecuted
        await ubaDeviceExecutePendingTask(pendingTask);

        await requestPromise;
        if (queryUbaDeviceError) {
            console.error('Request failed:', queryUbaDeviceError);
            expect(queryUbaDeviceRes.status).toBe(200);//in order to fail the test if it was not successful
        } else {
            console.log('Request succeeded:', queryUbaDeviceRes.body);
            expect(queryUbaDeviceRes.status).toBe(201);
            expect(queryUbaDeviceRes.body.address).toBe(pendingUbaDevice.address);
            expect(queryUbaDeviceRes.body.comPort).toBe(pendingUbaDevice.comPort);
            expect(queryUbaDeviceRes.body.machineMac).toBe(pendingUbaDevice.machineMac);
            expect(queryUbaDeviceRes.body.ubaSN).toBe(pendingTask.pendingUbaDevice.ubaSN);
            expect(queryUbaDeviceRes.body.ubaChannel).toBe(pendingTask.pendingUbaDevice.ubaChannel);
            expect(queryUbaDeviceRes.body.actionResult).toBe(pendingTask.pendingUbaDevice.actionResult);
            expect(queryUbaDeviceRes.body.action).toBe(pendingTask.pendingUbaDevice.action);
            expect(queryUbaDeviceRes.body.name).toBe(pendingTask.pendingUbaDevice.name);
            expect(queryUbaDeviceRes.body.fwVersion).toBe(pendingTask.pendingUbaDevice.fwVersion);
            expect(queryUbaDeviceRes.body.hwVersion).toBe(pendingTask.pendingUbaDevice.hwVersion);
        }

        //verify there are no other pending tasks
        await verifyAmountOfPendingTasks(machineToAdd.mac, 0);
    };

});