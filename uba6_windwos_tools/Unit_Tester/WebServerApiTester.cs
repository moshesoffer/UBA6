using BK_PRECISION9104Libary;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using UBA6Library.WebServerApi.Services;
using UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models;
using UBA6Library.WebServerApi.Services.WebConsole;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;
using UBA6Library.WebServerApi.Services.WebConsole.Model;

namespace Unit_Tester {
    [TestClass]
    public class WebServerApiTester {
        protected static WebConsoleService WebConsole;
        protected static bool isNeedToDelete = false;
        protected static readonly string mac2Test = "00:00:00:00:00:00";
        [ClassInitialize]
        public static void ClassInitialize(TestContext context) {
            // This method is called once before any tests in the class are run.
            // You can set up any necessary resources or configurations here.
            var loggerFactory = LoggerFactory.Create(builder => {
                builder.AddConsole();
                builder.SetMinimumLevel(LogLevel.Debug);
            });

            ILogger<WebConsoleService> logger = loggerFactory.CreateLogger<WebConsoleService>();


            WebConsole = new WebConsoleService(logger, "localhost", "4000");
        }

        [ClassCleanup]
        public static void ClassCleanup() {
            if (isNeedToDelete) {
                // WebConsole.Machines.WebRq.Delete<object>(WebConsole.Client, mac2Test).Wait();
            }
        }


        [TestMethod]
        public void TestWebServerApi() {
            Assert.IsTrue(WebConsole.AuthController.Login.Uri().ToString().Equals("http://localhost:4000/web-console/login"));
        }

        [TestMethod]
        public async Task TestMachineController() {
            try {
                List<MachineDTO> ml = await WebConsole.MachinesController.WebRq.Get<List<MachineDTO>>(WebConsole.Client);
                Assert.IsNotNull(ml); // Ensure the list is not null
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            }
        }

        [TestMethod]
        public async Task TestCreateNewMachine() {
            try {
                MachineDTO nm = new MachineDTO();
                nm.Name = "Test station";
                nm.Ip = WebService.GetLocalIPv4();
                nm.Mac = mac2Test;
                await WebConsole.MachinesController.WebRq.Post<object, MachineDTO>(WebConsole.Client, nm);
                isNeedToDelete = true; // Set flag to delete the machine after the test 
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {

            }
        }

        [TestMethod]
        public async Task TestDeleteMachine() {
            try {
                await WebConsole.MachinesController.WebRq.Delete<object>(WebConsole.Client, mac2Test);
                isNeedToDelete = false;
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }


        [TestMethod]
        public async Task TestUpdateMachine() {
            try {
                MachineDTO nm = new MachineDTO();
                List<MachineDTO> ml = await WebConsole.MachinesController.WebRq.Get<List<MachineDTO>>(WebConsole.Client);
                nm.Name = "Test Station Updated";
                nm.Ip = WebService.GetLocalIPv4();
                await WebConsole.MachinesController.WebRq.Patch<object, MachineDTO>(WebConsole.Client, ml.First().Mac.ToString(), nm);
                isNeedToDelete = true;
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }
        [TestCategory("UBA_Device_Controller")]
        [TestMethod]
        public async Task TestGetUBA() {
            try {
                UBADevicesResponseDTO m = await WebConsole.UBADevices.WebRq.Get<UBADevicesResponseDTO>(WebConsole.Client);
                Assert.IsNotNull(m); // Ensure the machine is not null
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }
        [TestMethod]
        public async Task TestPostUBADevices() {
            string sn2Add = "UBA-0046";
            try {
                List<MachineDTO> t = await WebConsole.MachinesController.WebRq.Get<List<MachineDTO>>(WebConsole.Client); // Ensure the machine list is loaded

                UBA_DeviceRequestDTO uBA_DeviceRequestDTO = new UBA_DeviceRequestDTO();
                uBA_DeviceRequestDTO.UbaChannel = "AB";
                uBA_DeviceRequestDTO.UbaSN = sn2Add;
                uBA_DeviceRequestDTO.MachineMac = t.First().Mac;
                uBA_DeviceRequestDTO.Name = "UBA_" + uBA_DeviceRequestDTO.UbaSN;
                uBA_DeviceRequestDTO.ComPort = "COM3";
                uBA_DeviceRequestDTO.Address = "0x10";
                await WebConsole.UBADevices.WebRq.Post<object, UBA_DeviceRequestDTO>(WebConsole.Client, uBA_DeviceRequestDTO);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
                await WebConsole.UBADevices.WebRq.Delete<object>(WebConsole.Client, sn2Add);
            }
        }
        [TestMethod]
        public async Task TestPatchUBADevices() {
            try {
                UBADevicesResponseDTO m = await WebConsole.UBADevices.WebRq.Get<UBADevicesResponseDTO>(WebConsole.Client);
                UBA_DevicesUpdateRequestDTO uBA_DevicesUpdateRequestDTO = new UBA_DevicesUpdateRequestDTO();
                uBA_DevicesUpdateRequestDTO.Name = "UBA Device Updated";
                uBA_DevicesUpdateRequestDTO.ComPort = "COM4";
                uBA_DevicesUpdateRequestDTO.Address = "0x20";
                await WebConsole.UBADevices.WebRq.Patch<object, UBA_DevicesUpdateRequestDTO>(WebConsole.Client, m.UbaDevices.First().UbaSN, uBA_DevicesUpdateRequestDTO);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {

            }
        }

        [TestMethod]
        public async Task TestDeleteUBADevices() {
            try {
                UBADevicesResponseDTO m = await WebConsole.UBADevices.WebRq.Get<UBADevicesResponseDTO>(WebConsole.Client);
                UBA_DevicesUpdateRequestDTO uBA_DevicesUpdateRequestDTO = new UBA_DevicesUpdateRequestDTO();
                uBA_DevicesUpdateRequestDTO.Name = "UBA Device Updated";
                uBA_DevicesUpdateRequestDTO.ComPort = "COM4";
                uBA_DevicesUpdateRequestDTO.Address = "0x20";
                await WebConsole.UBADevices.WebRq.Delete<object>(WebConsole.Client, m.UbaDevices.First().UbaSN);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }

        [TestMethod]
        public async Task TestGetPendingTests() {
            try {
                List<GETPendingTestResponseDTO> pts = await WebConsole.RT_Controller.PendingTest.Get<List<GETPendingTestResponseDTO>>(WebConsole.Client);

            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }
        [TestMethod]
        public async Task TestPOSTInstantTestResults() {
            try {
                UBADevicesResponseDTO m = await WebConsole.UBADevices.WebRq.Get<UBADevicesResponseDTO>(WebConsole.Client);
                InstantTestResultsDTO instantTestResultsDTO = new InstantTestResultsDTO();
                UbaDeviceDto uba = m.UbaDevices.First();
                instantTestResultsDTO.RunningTestID = uba.RunningTestID;
                instantTestResultsDTO.Timestamp = DateTime.Now;
                instantTestResultsDTO.TestState = "Stop";
                instantTestResultsDTO.TestCurrentStep = 0;
                instantTestResultsDTO.Voltage = 1;
                instantTestResultsDTO.Current = 2;
                instantTestResultsDTO.Temp = 3;
                instantTestResultsDTO.Capacity = 4;
                instantTestResultsDTO.Error = 5;
                List<InstantTestResultsDTO> sadas = new List<InstantTestResultsDTO>() { instantTestResultsDTO };

                await WebConsole.RT_Controller.InstantTestResults.Post<object, List<InstantTestResultsDTO>>(WebConsole.Client, sadas);

            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }
        [TestMethod]
        public async Task TestPATCH_ChangeTR_Status() {
            try {
                List<GETPendingTestResponseDTO> pts = await WebConsole.RT_Controller.PendingTest.Get<List<GETPendingTestResponseDTO>>(WebConsole.Client);
                PATCH_ChangeTR_StatusRequest pATCH_ChangeTR_StatusRequest = new PATCH_ChangeTR_StatusRequest();
                foreach (GETPendingTestResponseDTO pt in pts) {
                    pATCH_ChangeTR_StatusRequest.RunningTestID = pt.Id;
                    pATCH_ChangeTR_StatusRequest.TestRoutineChannels = pt.Channel;
                    pATCH_ChangeTR_StatusRequest.UbaSN = pt.UbaSN;
                    pATCH_ChangeTR_StatusRequest.NewTestStatus = (pt.Status & (~0x100));
                    await WebConsole.RT_Controller.ChangeRunningTestStatus.Patch<object, PATCH_ChangeTR_StatusRequest>(WebConsole.Client, pATCH_ChangeTR_StatusRequest);
                }
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }
        [TestMethod]
        public async Task PendingTasks() {
            try {
                List<KeyValuePair<string, string>> querys = new List<KeyValuePair<string, string>>();
                querys.Add(new KeyValuePair<string, string>("machineMac", mac2Test));
                GETPendingTasksDTO rts = await WebConsole.PendingTasksController.PendingTasks.Get<GETPendingTasksDTO>(WebConsole.Client, querys);
                if (rts.PendingConnectionUbaDevices.Count > 0) {

                }
                Console.WriteLine(rts);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }

        [TestMethod]
        public async Task TestUpdateTestReadingData() {
            try {
                Guid guid = new Guid("07d9ad90-0df0-4cde-9482-17327b37b8a3");
                UBA_PROTO_BPT.status_message msg = new UBA_PROTO_BPT.status_message();
                msg.ChannelStatus = new UBA_PROTO_CHANNEL.status();
                msg.ChannelStatus.Data = new UBA_PROTO_CHANNEL.data_message();
                msg.ChannelStatus.Data.Voltage = 1000;
                await WebConsole.UpdateTestReadingData(guid, msg);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }

        [TestMethod]
        public async Task TestDeserialize() {
            try {
                string JsonStr = "{\"ubaDevices\":[{\"ubaSN\":\"30\",\"machineMac\":\"7478271D93AD\",\"name\":\"blabla.dkf\",\"address\":\"1\",\"comPort\":\"COM6\",\"ubaChannel\":\"AB\",\"fwVersion\":\"12.12.12.12\",\"hwVersion\":\"12.12\",\"createdTime\":\"2025-09-14T11:08:18.589Z\",\"modifiedTime\":\"2025-09-14T11:08:18.589Z\",\"machineName\":\"ORS-DELL-WORKST - 7478271D93AD\",\"runningTestID\":\"5aa996dd-6711-4716-8b4f-b51bf2f3da27\",\"testName\":\"TestFormWeb\",\"channel\":\"A\",\"timestampStart\":\"2025-09-16T14:58:03.000Z\",\"status\":288,\"batteryPN\":\"INR18650-MJ-as\",\"batterySN\":\"INR18650-M\",\"testRoutineChannels\":\"A-or-B\",\"totalStagesAmount\":1,\"testState\":null,\"testCurrentStep\":null,\"voltage\":null,\"current\":null,\"temp\":null,\"capacity\":null,\"error\":null,\"lastInstantResultsTimestamp\":null,\"ubaDeviceConnectedTimeAgoMs\":null},{\"ubaSN\":\"30\",\"machineMac\":\"7478271D93AD\",\"name\":\"blabla.dkf\",\"address\":\"1\",\"comPort\":\"COM6\",\"ubaChannel\":\"AB\",\"fwVersion\":\"12.12.12.12\",\"hwVersion\":\"12.12\",\"createdTime\":\"2025-09-14T11:08:18.589Z\",\"modifiedTime\":\"2025-09-14T11:08:18.589Z\",\"machineName\":\"ORS-DELL-WORKST - 7478271D93AD\",\"runningTestID\":\"e04d6369-d23f-4a69-9cf9-1c1a7863115f\",\"testName\":\"TestFormWeb\",\"channel\":\"B\",\"timestampStart\":\"2025-09-16T13:54:54.000Z\",\"status\":1,\"batteryPN\":\"INR18650-MJ-as\",\"batterySN\":\"INR18650-M\",\"testRoutineChannels\":\"A-or-B\",\"totalStagesAmount\":1,\"testState\":\"Standby\",\"testCurrentStep\":0,\"voltage\":\"20490.0000\",\"current\":\"0.0190\",\"temp\":\"0.0000\",\"capacity\":\"0.00000\",\"error\":0,\"lastInstantResultsTimestamp\":\"2025-09-16T11:47:32.387Z\",\"ubaDeviceConnectedTimeAgoMs\":null}],\"ubaTotal\":{\"configured\":1,\"connected\":0,\"running\":1}}";
                UBADevicesResponseDTO t = JsonSerializer.Deserialize<UBADevicesResponseDTO>(JsonStr);
                Console.WriteLine(t);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }
        }

        [TestMethod]
        public async Task TestDeserializeTestPlanCharge() {
            try {
                string JsonStr = "{\r\n  \"pendingConnectionUbaDevices\": [],\r\n  \"pendingRunningTests\": [\r\n    {\r\n      \"id\": \"119ca94f-e9f7-4c91-836a-65ffacf826ef\",\r\n      \"ubaSN\": \"30\",\r\n      \"channel\": \"A\",\r\n      \"status\": 288,\r\n      \"testRoutineChannels\": \"A-or-B\",\r\n      \"machineMac\": \"88AEDD4CD5B8\",\r\n      \"noCellSerial\": 4,\r\n      \"testName\": \"Test Charge\",\r\n      \"plan\": [\r\n        {\r\n          \"id\": 0,\r\n          \"type\": \"charge\",\r\n          \"cRate\": 0.18,\r\n          \"source\": \"internal\",\r\n          \"maxTemp\": 60,\r\n          \"maxTime\": \"00:00:02\",\r\n          \"minTemp\": -20,\r\n          \"goToStep\": null,\r\n          \"waitTemp\": null,\r\n          \"delayTime\": null,\r\n          \"isMaxTemp\": true,\r\n          \"isMaxTime\": true,\r\n          \"isMinTemp\": true,\r\n          \"repeatStep\": null,\r\n          \"chargeLimit\": \"2000:absoluteMah\",\r\n          \"isCollapsed\": false,\r\n          \"chargeCurrent\": \"1234:absoluteMa\",\r\n          \"chargePerCell\": \"4.20\",\r\n          \"cutOffCurrent\": \"500:absoluteMa\",\r\n          \"cutOffVoltage\": null,\r\n          \"isChargeLimit\": true,\r\n          \"dischargeLimit\": \":absoluteMah\",\r\n          \"isCutOffCurrent\": true,\r\n          \"isCutOffVoltage\": false,\r\n          \"dischargeCurrent\": \":absoluteMa\",\r\n          \"isDischargeLimit\": false\r\n        }\r\n      ],\r\n      \"timestampStart\": \"2025-09-18T09:08:14.000Z\",\r\n      \"reportId\": \"068ab85d-a93d-4dc7-8d32-66ae806e4150\"\r\n    }\r\n  ],\r\n  \"pendingReports\": [\r\n    {\r\n      \"id\": \"068ab85d-a93d-4dc7-8d32-66ae806e4150\",\r\n      \"ubaSN\": \"30\",\r\n      \"channel\": \"A\",\r\n      \"timestampStart\": \"2025-09-18T09:08:14.000Z\",\r\n      \"status\": 288,\r\n      \"testName\": \"Test Charge\",\r\n      \"batteryPN\": \"INR18650-MJ11asd12a\",\r\n      \"batterySN\": \"INR18650-MJ11asd12a\",\r\n      \"cellPN\": \"INR18650-MJ1\",\r\n      \"chemistry\": \"Li-Ion\",\r\n      \"noCellSerial\": 4,\r\n      \"noCellParallel\": 2,\r\n      \"maxPerBattery\": 16.8,\r\n      \"ratedBatteryCapacity\": 7000,\r\n      \"notes\": null,\r\n      \"customer\": null,\r\n      \"workOrderNumber\": null,\r\n      \"approvedBy\": null,\r\n      \"conductedBy\": null,\r\n      \"cellSupplier\": null,\r\n      \"cellBatch\": null,\r\n      \"plan\": [\r\n        {\r\n          \"id\": 0,\r\n          \"type\": \"charge\",\r\n          \"cRate\": 0.18,\r\n          \"source\": \"internal\",\r\n          \"maxTemp\": 60,\r\n          \"maxTime\": \"00:00:02\",\r\n          \"minTemp\": -20,\r\n          \"goToStep\": null,\r\n          \"waitTemp\": null,\r\n          \"delayTime\": null,\r\n          \"isMaxTemp\": true,\r\n          \"isMaxTime\": true,\r\n          \"isMinTemp\": true,\r\n          \"repeatStep\": null,\r\n          \"chargeLimit\": \"2000:absoluteMah\",\r\n          \"isCollapsed\": false,\r\n          \"chargeCurrent\": \"1234:absoluteMa\",\r\n          \"chargePerCell\": \"4.20\",\r\n          \"cutOffCurrent\": \"500:absoluteMa\",\r\n          \"cutOffVoltage\": null,\r\n          \"isChargeLimit\": true,\r\n          \"dischargeLimit\": \":absoluteMah\",\r\n          \"isCutOffCurrent\": true,\r\n          \"isCutOffVoltage\": false,\r\n          \"dischargeCurrent\": \":absoluteMa\",\r\n          \"isDischargeLimit\": false\r\n        }\r\n      ],\r\n      \"testRoutineChannels\": \"A-or-B\",\r\n      \"machineMac\": \"88AEDD4CD5B8\",\r\n      \"machineName\": \"PC-187 - 88AEDD4CD5B8\",\r\n      \"timeOfTest\": null,\r\n      \"createdTime\": \"2025-09-18T09:08:14.445Z\",\r\n      \"modifiedTime\": \"2025-09-18T09:08:14.445Z\",\r\n      \"pendingRunningTestId\": \"119ca94f-e9f7-4c91-836a-65ffacf826ef\"\r\n    }\r\n  ]\r\n}";
                GETPendingTasksDTO t = JsonSerializer.Deserialize<GETPendingTasksDTO>(JsonStr);
                util.GETPendingTestResponseDTO2TR_Message(t.PendingRunningTests.First());
                Console.WriteLine(t);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex}");
            } finally {
            }
        }
        [TestMethod]
        public async Task TestDeserializeTestPlanDischarge() {
            try {
                string JsonStr = "{\r\n  \"pendingConnectionUbaDevices\": [],\r\n  \"pendingRunningTests\": [\r\n    {\r\n      \"id\": \"d27a4596-1f1b-4b95-a7d2-a4e8ddcb5792\",\r\n      \"ubaSN\": \"0\",\r\n      \"channel\": \"A\",\r\n      \"status\": 288,\r\n      \"testRoutineChannels\": \"A-or-B\",\r\n      \"machineMac\": \"18473DB90EBB\",\r\n      \"noCellSerial\": 2,\r\n      \"testName\": \"Test Discharge\",\r\n      \"plan\": [\r\n        {\r\n          \"id\": 0,\r\n          \"type\": \"discharge\",\r\n          \"cRate\": 0,\r\n          \"source\": \"internal\",\r\n          \"maxTemp\": \"4.5\",\r\n          \"maxTime\": \"00:06:00\",\r\n          \"minTemp\": \"0.1\",\r\n          \"goToStep\": null,\r\n          \"waitTemp\": null,\r\n          \"delayTime\": null,\r\n          \"isMaxTemp\": true,\r\n          \"isMaxTime\": true,\r\n          \"isMinTemp\": true,\r\n          \"repeatStep\": null,\r\n          \"chargeLimit\": \":absoluteMah\",\r\n          \"isCollapsed\": false,\r\n          \"chargeCurrent\": \":absoluteMa\",\r\n          \"chargePerCell\": null,\r\n          \"cutOffCurrent\": \":absoluteMa\",\r\n          \"cutOffVoltage\": \"7.8\",\r\n          \"isChargeLimit\": false,\r\n          \"dischargeLimit\": \"9.10:absoluteMah\",\r\n          \"isCutOffCurrent\": false,\r\n          \"isCutOffVoltage\": true,\r\n          \"dischargeCurrent\": \"2.3:absoluteMa\",\r\n          \"isDischargeLimit\": true\r\n        }\r\n      ],\r\n      \"timestampStart\": \"2025-09-18T08:01:43.000Z\",\r\n      \"reportId\": \"e993ae83-9c05-470c-b628-8150fa3f00b1\"\r\n    }\r\n  ],\r\n  \"pendingReports\": [\r\n    {\r\n      \"id\": \"e993ae83-9c05-470c-b628-8150fa3f00b1\",\r\n      \"ubaSN\": \"0\",\r\n      \"channel\": \"A\",\r\n      \"timestampStart\": \"2025-09-18T08:01:43.000Z\",\r\n      \"status\": 288,\r\n      \"testName\": \"Test Discharge\",\r\n      \"batteryPN\": \"INR18650-MJ1asqw1\",\r\n      \"batterySN\": \"INR18650-MJ1as1\",\r\n      \"cellPN\": \"INR18650-MJ1\",\r\n      \"chemistry\": \"Li-Ion\",\r\n      \"noCellSerial\": 2,\r\n      \"noCellParallel\": 2,\r\n      \"maxPerBattery\": 8.4,\r\n      \"ratedBatteryCapacity\": 7000,\r\n      \"notes\": null,\r\n      \"customer\": null,\r\n      \"workOrderNumber\": null,\r\n      \"approvedBy\": null,\r\n      \"conductedBy\": null,\r\n      \"cellSupplier\": null,\r\n      \"cellBatch\": null,\r\n      \"plan\": [\r\n        {\r\n          \"id\": 0,\r\n          \"type\": \"discharge\",\r\n          \"cRate\": 0,\r\n          \"source\": \"internal\",\r\n          \"maxTemp\": \"4.5\",\r\n          \"maxTime\": \"00:06:00\",\r\n          \"minTemp\": \"0.1\",\r\n          \"goToStep\": null,\r\n          \"waitTemp\": null,\r\n          \"delayTime\": null,\r\n          \"isMaxTemp\": true,\r\n          \"isMaxTime\": true,\r\n          \"isMinTemp\": true,\r\n          \"repeatStep\": null,\r\n          \"chargeLimit\": \":absoluteMah\",\r\n          \"isCollapsed\": false,\r\n          \"chargeCurrent\": \":absoluteMa\",\r\n          \"chargePerCell\": null,\r\n          \"cutOffCurrent\": \":absoluteMa\",\r\n          \"cutOffVoltage\": \"7.8\",\r\n          \"isChargeLimit\": false,\r\n          \"dischargeLimit\": \"9.10:absoluteMah\",\r\n          \"isCutOffCurrent\": false,\r\n          \"isCutOffVoltage\": true,\r\n          \"dischargeCurrent\": \"2.3:absoluteMa\",\r\n          \"isDischargeLimit\": true\r\n        }\r\n      ],\r\n      \"testRoutineChannels\": \"A-or-B\",\r\n      \"machineMac\": \"18473DB90EBB\",\r\n      \"machineName\": \"ORS-DELL-WORKST - 18473DB90EBB\",\r\n      \"timeOfTest\": null,\r\n      \"createdTime\": \"2025-09-18T08:01:43.948Z\",\r\n      \"modifiedTime\": \"2025-09-18T08:01:43.948Z\",\r\n      \"pendingRunningTestId\": \"d27a4596-1f1b-4b95-a7d2-a4e8ddcb5792\"\r\n    }\r\n  ]\r\n}";
                GETPendingTasksDTO t = JsonSerializer.Deserialize<GETPendingTasksDTO>(JsonStr);
                util.GETPendingTestResponseDTO2TR_Message(t.PendingRunningTests.First());
                Console.WriteLine(t);
            } catch (Exception ex) {
                Assert.Fail($"Test aborted due to an exception: {ex.Message}");
            } finally {
            }

        }
    }
}
