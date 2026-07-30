using Grpc.Core;
using Microsoft.AspNetCore.Authentication;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.ObjectPool;
using Microsoft.Extensions.Options;
using System.Collections.Generic;
using System.Net.Http.Headers;
using System.Runtime;
using System.Text.Json;
using System.Text.RegularExpressions;
using UBA_MSG;
using UBA_PROTO_CHANNEL;
using UBA_PROTO_CMD;
using UBA_PROTO_TR;
using UBA6Library;
using UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models;
using UBA6Library.WebServerApi.Services.WebConsole;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;
using UBA6Library.WebServerApi.Services.WebConsole.Model;

namespace UBAService {
    public class Worker : BackgroundService {
        private readonly ILogger<Worker> _logger;
        private readonly MyLocalSettings _settings;
        private readonly ILogger<UBA6> _ubaLogger;
        private readonly ILogger<UBA_Interface> _comLoger;
        private bool isInitialized = false;
        private List<UBA6> UBAs { get; set; } = new List<UBA6>();
        private List<UBA_Interface> UBA_Interfaces { get; set; } = new List<UBA_Interface>();
        private WebConsoleService wcs;
        private static TimeSpan delay = TimeSpan.FromSeconds(1);
        private int[] channelStatus = { 0, 0 };
        private bool[] testInProgress = { false, false};
        private readonly SemaphoreSlim _semaphore = new SemaphoreSlim(1, 1);

        public Worker(ILogger<Worker> logger, ILogger<UBA6> ubaLogger, ILogger<WebConsoleService> webConsoleLogger, ILogger<UBA_Interface> COM_logger, IOptions<MyLocalSettings> settings) {
            _logger = logger;
            _ubaLogger = ubaLogger;
            _settings = settings.Value;
            _comLoger = COM_logger;
            wcs = new WebConsoleService(webConsoleLogger, _settings.ServerIpAddress, "4000");
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken) {
            try {
                _logger.LogInformation("Service starting. Log path: {Path}, Retry: {Retry}", _settings.LogPath, _settings.RetryCount);
                while (!stoppingToken.IsCancellationRequested) {
                    if (!isInitialized) {
                        await InitAsync(stoppingToken);

                        //AddUBAsAsync(stoppingToken);

                        //periodic query message to UBA for running test data - instantTestResults (state, startTime, step, voltage, current, temp, capacity, ..)
                        StartPeriodicRunningTestUpdate(stoppingToken);    //StopPeriodicRunningTestUpdate();

                        //periodic received message from UBA Device
                        StartPeriodicUBAUpdate(stoppingToken);              //StopPeriodicUBAUpdate();

                    } else {
                        try {
                            GETPendingTasksDTO pt = await wcs.GetPendingTasks();
                            if (pt != null) {
//_logger.LogInformation("PendingRunningTests {count}", pt?.PendingRunningTests?.Count);
                                //_logger.LogInformation("PendingRunningTests {count}", pt?.PendingRunningTests?.Count);
                                if (pt?.PendingRunningTests?.Count > 0) {
                                    //pole RunningTestsController.Status change (GETPendingTestResponse) - STANDBY,RUNNING,PAUSED,..
                                    await resolvePendingRunningTest(pt?.PendingRunningTests);
                                }
                            }

                        } catch {
                            _logger.LogError("GetPendingTasks failed");
                            continue;
                        }
                    }
                    await Task.Delay(300/*msec delay*/, stoppingToken);
                }
            } catch (Exception ex) {
                _logger.LogError(ex, "An error occurred in the UBA Service: {Message}", ex.Message);
            } finally {
                _logger.LogInformation("Service stopping.");
            }
        }

        private CancellationTokenSource? _cts1sec;
        //periodic query message to UBA for running test data - instantTestResults (state, startTime, step, voltage, current, temp, capacity, ..)
        public async Task StartPeriodicRunningTestUpdate(CancellationToken stoppingToken) {
            int timeout = 1000;
            var tcs = new TaskCompletionSource<Message?>();

            while (true) {
                CancellationTokenSource timeoutCts = new(timeout);
                var stopwatch = System.Diagnostics.Stopwatch.StartNew();

                try { 
                    using (timeoutCts) {
                        var delayTask = Task.Delay(timeout);
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                        stopwatch.Stop();

                        await _semaphore.WaitAsync();
                        //query message to UBA for running test data - instantTestResults (state, startTime, step, voltage, current, temp, capacity, ..)
                        await updateRunningTestData();

                        //Change Running test status (uba.Status) - done manually in web-console (confirm click)
                        await refreshChannelReading();
                        _semaphore.Release();

                    }
                } finally {
                }
            }
        }
        public void StopPeriodicRunningTestUpdate()
        {
            _cts1sec?.Cancel();
        }


        private CancellationTokenSource? _cts3sec;
        //periodic received message from UBA Device
        public async Task StartPeriodicUBAUpdate(CancellationToken stoppingToken) {
            int timeout = 500;
            var tcs = new TaskCompletionSource<Message?>();

            while (true) {
                CancellationTokenSource timeoutCts = new(timeout);
                var stopwatch = System.Diagnostics.Stopwatch.StartNew();

                try { 
                    using (timeoutCts) {
                        var delayTask = Task.Delay(timeout);
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                        stopwatch.Stop();


                    await _semaphore.WaitAsync();
                    GETPendingTasksDTO pt = await wcs.GetPendingTasks();
                    if (pt != null) {
                        if (pt?.PendingConnectionUbaDevices?.Count > 0) {
                            //handle received message from UBA Device (PendingConnectionUbaDevice, uba.ComPort)
                            await resolvePendingUBA(pt?.PendingConnectionUbaDevices);
                        }

                        //add UBA device to UBAs list (GETPendingTestResponse)
                        await updateUBA2List(pt?.PendingRunningTests);
                    }
                    _semaphore.Release();
                } finally {
                }
            }
        }
       
        public void StopPeriodicUBAUpdate()
        {
            _cts1sec?.Cancel();
        }
    
        private UBA6 getUbaFromList(string SN) {
            UBA6? existingUba = UBAs.FirstOrDefault(uba => uba.SerialNumber.Equals(SN));
            return existingUba;            
        }

        private async Task refreshChannelReading() {
            ////_logger.LogInformation("refreshChannelReading: Refresh UBA Channel");
            List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
            if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                _logger.LogWarning("1 No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);
            }
            else {
                ////_logger.LogInformation("Found {Count} UBA devices.", ubaDeviceDtos.Count);
                foreach (var ubaDto in ubaDeviceDtos) {
                    try {
//_logger.LogInformation("3. {Count}-UBAs channel {Channel} status {Status}", ubaDeviceDtos.Count, ubaDto.Channel, ubaDto.Status);
                        if (ubaDto.Channel.Equals("A") || ubaDto.Channel.Equals("Ab")) {
                            Message message = null;// await UBAs.First().GetMessage(UBA_PROTO_QUERY.RECIPIENT.BptA);
                            for (int i = 0; i < UBAs.Count; i++)
                            {
                                if (UBAs[i].Address.ToString() == ubaDto.Address)
                                {
_logger.LogInformation("3.1 refreshChannelReading {recipent} {address}", UBA_PROTO_QUERY.RECIPIENT.BptA, ubaDto.Address);
                                    message = await UBAs[i].GetMessage(UBA_PROTO_QUERY.RECIPIENT.BptA);
                                    break;
                                }
                            }
                            if (message != null) {
                                await wcs.UpdateTestReadingData(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);

                                if (channelStatus [0] != (int)message.QueryResponse.Bpt.State) {
                                if (((((RunningTestsController.Status)ubaDto.Status) & RunningTestsController.Status.STOPPED) == 0) &&
                                    ((((RunningTestsController.Status)ubaDto.Status) & RunningTestsController.Status.PENDING) == 0)) {
                                    ////_logger.LogInformation("Change PENDING Test Status {State}", message.QueryResponse.Bpt.State);
_logger.LogInformation("4.1.Pending test {Channel} {Status}, set to {newState} {channelStatus[0]}", ubaDto.Channel, ubaDto.Status, message.QueryResponse.Bpt.State, channelStatus[0]);
                                    await wcs.ChangeRunningTestStatus(ubaDto, Bptstate2DTOstate(message.QueryResponse.Bpt.State, message.QueryResponse.Bpt.StepType));
                                }
                                channelStatus [0] = (int)message.QueryResponse.Bpt.State;
                                }
                            }

                        } else if (ubaDto.Channel.Equals("B") || ubaDto.Channel.Equals("Ab")) {
                            Message message = null;// await UBAs.First().GetMessage(UBA_PROTO_QUERY.RECIPIENT.BptB);
                            for (int i = 0; i < UBAs.Count; i++)
                            {
                                if (UBAs[i].Address.ToString() == ubaDto.Address)
                                {
_logger.LogInformation("3.1 refreshChannelReading {recipent} {address}", UBA_PROTO_QUERY.RECIPIENT.BptB, ubaDto.Address);
                                    message = await UBAs[i].GetMessage(UBA_PROTO_QUERY.RECIPIENT.BptB);
                                    break;
                                }
                            }
                            if (message != null) {
                                await wcs.UpdateTestReadingData(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);

                                if (channelStatus [1] != (int)message.QueryResponse.Bpt.State) {
                                if (((((RunningTestsController.Status)ubaDto.Status) & RunningTestsController.Status.STOPPED) == 0) &&
                                    ((((RunningTestsController.Status)ubaDto.Status) & RunningTestsController.Status.PENDING) == 0)) {
                                    ////_logger.LogInformation("Change PENDING Test Status {State}", message.QueryResponse.Bpt.State);
_logger.LogInformation("4.2.Pending test {Channel} {Status}, set to {newState}", ubaDto.Channel, ubaDto.Status, message.QueryResponse.Bpt.State);
                                    await wcs.ChangeRunningTestStatus(ubaDto, Bptstate2DTOstate(message.QueryResponse.Bpt.State, message.QueryResponse.Bpt.StepType));
                                }
                                channelStatus [1] = (int)message.QueryResponse.Bpt.State;
                                }
                            }
                        }

                    } catch {
                        _logger.LogError($"1-No Response from UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");
                    }
                }
            }
        }

        private async Task SaveTestAsync(GETPendingTestResponseDTO  pendingTest)
        {
            UBA6 uba = getUbaFromList(pendingTest.UbaSN);
            if (uba == null)
            {
                _logger.LogInformation("1-Pending tests list empty.");
                return;
            }

            string filename = await uba.GetRunningTestFileName(util.GetChannelFormDTO(pendingTest));

            _logger.LogInformation(
                "Stopping BPT on channel {Channel} with file {FileName}",
                util.GetChannelFormDTO(pendingTest),
                filename);

            await _semaphore.WaitAsync();
            try
            {
                await wcs.TestResultUpdateStatus(
                    pendingTest.ReportId,
                    RunningTestsController.Status.PENDING |
                    RunningTestsController.Status.SAVED);

                byte[] file = await uba.FeatchFileToByteArray(filename);
                await wcs.TestResultUpload(pendingTest.ReportId, file);
            }
            finally
            {
                _semaphore.Release();
            }

            uba.ClearBPT(util.GetChannelFormDTO(pendingTest));

            if (pendingTest.Channel == "A" || pendingTest.Channel == "Ab")
                testInProgress[0] = false;
            else if (pendingTest.Channel == "B")
                testInProgress[1] = false;
        }

        private async Task resolvePendingRunningTest(List<GETPendingTestResponseDTO>? pt) {
            try {
                //_logger.LogInformation("Resolving pending UBA Tests... {count}", pt.Count);
                if (pt == null || pt.Count == 0) {
                    _logger.LogInformation("No pending tests found.");
                    return;
                }

                foreach (GETPendingTestResponseDTO pendingTest in pt) {
//_logger.LogInformation("==> pendingTest: {id} {status}", pendingTest.Id, pendingTest.Status);
                    //UBA6 uba = getUbaFromList(pendingTest.UbaSN);
                    //if (uba == null)
                    //{
                    //    _logger.LogInformation("Pending tests list empty.");
                    //    return;
                    //}

                    UBA6 uba = getUbaFromList(pendingTest.UbaSN);
                    if (uba == null)
                    {
                        _logger.LogInformation("2-Pending tests list empty.");
                        return;
                    }

                    //verify UBA responding
                    try {
                        var dev = UBA_PROTO_QUERY.RECIPIENT.Device;
_logger.LogInformation("==> resolvePendingRunningTest: get message 2 {device}", dev);
                        await uba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device);
                        //_logger.LogInformation("Response from UBA Device: {uba.Address}");
                    } catch {
                        _logger.LogError("2-No Response from UBA Device: {uba.Address}");
                    }

                    if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.PENDING) > 0) {
                        _logger.LogInformation("==> PENDING, ch {channel} ...", pendingTest.Channel);
                        ////uba.PendingBPT(util.GetChannelFormDTO(pendingTest));

//_logger.LogInformation("2.Pending test {Channel} {Status}, set to {Status}", pendingTest.Channel, pendingTest.Status, pendingTest.Status & ~((uint)RunningTestsController.Status.PENDING));
                        await wcs.ChangeRunningTestStatus(pendingTest, (int)((uint)pendingTest.Status & ~((uint)RunningTestsController.Status.PENDING)));
                    } /*else*/ if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.RUNNING) > 0) {
                        _logger.LogInformation("==> RUNNING, ch {channel} ...", pendingTest.Channel);
                        await uba.UpdatedTime();
                        Test_Routine_Message msg = new Test_Routine_Message();
                        msg.Index = util.GetIndexFormDTO(pendingTest);
                        msg.Tr = util.GETPendingTestResponseDTO2TR_Message(pendingTest);
                        try { 
                            await uba.SentMessageAsync(UBA_Message_Factory.CreateMessage(uba.Address, msg), UBA_Interface.MessagePriority.TEST_ROUTINE);
                        }
                        catch {
                            _logger.LogInformation($"SentMessageAsync fail");
                        };
                        //await Task.Delay(500);
//_logger.LogInformation("1.Pending test ch= {Channel} code= {chcode} st= {Status}, set to RUNNING", pendingTest.Channel, util.GetChannelFormDTO(pendingTest), pendingTest.Status);
                        uba.StartBPT(util.GetChannelFormDTO(pendingTest), util.GetIndexFormDTO(pendingTest));
                        await wcs.ChangeRunningTestStatus(pendingTest, (int)RunningTestsController.Status.RUNNING);
                        if (pendingTest.Channel.Equals("A") || pendingTest.Channel.Equals("Ab")) {
                            testInProgress[0] = true;
                        } else if (pendingTest.Channel.Equals("B")) {
                            testInProgress[1] = true;                                
                        }

                    } /*else*/ if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.STOPPED) > 0) {
                        _logger.LogInformation("==> STOPPED, ch {channel} ...", pendingTest.Channel);
                        uba.StopBPT(util.GetChannelFormDTO(pendingTest));

                    } /*else*/ if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.PAUSED) > 0) {
                        _logger.LogInformation("==> PAUSED, ch {channel} ...", pendingTest.Channel);
                        uba.PasueBPT(util.GetChannelFormDTO(pendingTest));

                    } /*else*/ if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.NEXTSTEP) > 0) {
                        _logger.LogInformation("==> NEXTSTEP, ch {channel} ...", pendingTest.Channel);
                        uba.StepBPT(util.GetChannelFormDTO(pendingTest));

                    } /*else*/ if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.STANDBY) > 0) {
                        _logger.LogInformation("==> STANDBY, ch {channel} ...", pendingTest.Channel);

//                        string filename = await uba.GetRunningTestFileName(util.GetChannelFormDTO(pendingTest));
//                        _logger.LogInformation("Stopping BPT on channel {Channel} with file {FileName}", util.GetChannelFormDTO(pendingTest), filename);
//
//                        await _semaphore.WaitAsync();
//                        // TODO: input filename
//                        await wcs.TestResultUpdateStatus(pendingTest.ReportId,RunningTestsController.Status.PENDING | RunningTestsController.Status.SAVED);
//                            _ = uba.FeatchFileToByteArray(filename).ContinueWith(task => {
//                            byte[] file = task.Result;
//                            _ = wcs.TestResultUpload(pendingTest.ReportId, file);
//                        });
//                        _semaphore.Release();
//
//                        uba.ClearBPT(util.GetChannelFormDTO(pendingTest));
//                        if (pendingTest.Channel.Equals("A") || pendingTest.Channel.Equals("Ab")) {
//                            testInProgress[0] = false;
//                        } else if (pendingTest.Channel.Equals("B")) {
//                            testInProgress[1] = false;                                
//                        }
                    
                        _ = SaveTestAsync(pendingTest);

                    } /*else*/ if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.ABORTED) > 0) {
                        _logger.LogInformation("==> ABORTED, ch {channel} ...", pendingTest.Channel);
                        ////uba.AbortedBPT(util.GetChannelFormDTO(pendingTest));
                    }

                    //UpdateChannelReadingData(pendingTest.Id, UBA_PROTO_CHANNEL.status msg)
                    //try {
                    //    Guid guid = new Guid("07d9ad90-0df0-4cde-9482-17327b37b8a3");
                    //    UBA_PROTO_BPT.status_message msg = new UBA_PROTO_BPT.status_message();
                    //    msg.ChannelStatus = new UBA_PROTO_CHANNEL.status();
                    //    msg.ChannelStatus.Data = new UBA_PROTO_CHANNEL.data_message();
                    //    msg.ChannelStatus.Data.Voltage = 3;
                    //    msg.ChannelStatus.Data.Temperature = 4;
                    //    msg.ChannelStatus.Data.Current = 5;
                    //    msg.ChannelStatus.Data.current
                    //    await wcs.UpdateTestReadingData(guid, msg);
                    //} catch (Exception ex) {
                    //    _logger.LogInformation("==> Test aborted due to an exception: {ex.Message}");
                    //} finally {
                    //}
                }
            } catch (Exception ex) {
                _logger.LogError(ex, "An error occurred while resolving pending tests: {Message}", ex.Message);
            }
        }

        private async Task resolvePendingReports(List<PendingReportDTO>? PR_List, List<GETPendingTestResponseDTO>? pendingTest_List) {
            try {
                if (PR_List == null || PR_List.Count == 0) {
                    _logger.LogInformation("No pending tests found.");
                    return;
                }
                foreach (PendingReportDTO pr in PR_List) {
                    if (UBAs.Count > 0) {
                    //    _logger.LogInformation("pr {channel} {status}", pr.Channel, pr.Status);
                        //List<string> filelist = await UBAs.First().FeatchFileList();
                        //byte[]? file = await UBAs.First().FeatchFileToByteArray(filelist.Last());
                        //await wcs.TestResultUpload(pr, file);
                    }
                }
                } catch (Exception e) {
                _logger.LogError("Error in resolvePendingReports: {Message}", e.Message);
            }
        }

        private async Task resolveUBAFileList() {
            try {
                if (UBAs.Count > 0) {
                    List<string> filelist = await UBAs.First().FeatchFileList();
                    byte[]? file = await UBAs.First().FeatchFileToByteArray(filelist.Last());
                    _logger.LogInformation("resolveUBAFileList {file}", file);
                }
            } catch (Exception e) {
                _logger.LogError("Error in resolveUBAFileList: {Message}", e.Message);
            }
         }

        protected async Task resolvePendingUBA(List<PendingConnectionUbaDevice>? connecedList) {
            try {
                if (connecedList == null || connecedList.Count == 0) {
                    _logger.LogInformation("No connected UBA devices found.");
                    return;
                }
                foreach (var pendingDevice in connecedList) {
                    bool exists = UBA_Interfaces.Any(ui => ui.PortName == pendingDevice.ComPort);
                    if (exists == false) {
_logger.LogInformation("==> 1-Add Interface {address} {comPort}", pendingDevice.Address, pendingDevice.ComPort);
                        UBA_Interfaces.Add(new UBA_Interface(_comLoger, pendingDevice.ComPort));
                    }
                    
                    UBA_Interface? UbaComInterface = UBA_Interfaces.FirstOrDefault(ui => ui.PortName == pendingDevice.ComPort);
                    if (UbaComInterface != null) {
                        try {
//_logger.LogInformation("==> resolvePendingUBA: get message 3: {address}", pendingDevice.Address);
                            //verify UBA already exist
                            if (UBAs.Any(uba => pendingDevice.Address == uba.Address.ToString()))
                            {
                                break;
                            }

                            if (!pendingDevice.Action.Equals("removeFromWatchList")) {
                                Message? t = await UbaComInterface.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device, Convert.ToUInt32(pendingDevice.Address));
                                if (t != null) {
                                    _logger.LogInformation($"2 Received message from UBA Device '{t?.QueryResponse.Recipient}' {t?.QueryResponse.Device.Settings}");
                                    await wcs.DeviceFound(t.QueryResponse, pendingDevice.ComPort);
                                } else {
                                    _logger.LogWarning($"wrong message from UBA Device on Port {pendingDevice.ComPort} at Address {pendingDevice.Address}");
                                }
                            }
                        } catch
                        {
                            _logger.LogError($"3-No Response from UBA Device on Port {pendingDevice.ComPort} at Address {pendingDevice.Address}");
                        }
                    }
                }
            } catch (Exception ex) {
                _logger.LogError(ex, "An error occurred while resolving pending UBA connections: {Message}", ex.Message);
            }
        }

        private int Bptstate2DTOstate(UBA_PROTO_BPT.STATE bpt_state, UBA_PROTO_BPT.STEP_TYPE step_type) {
            RunningTestsController.Status retState = RunningTestsController.Status.STANDBY;
            if (bpt_state == UBA_PROTO_BPT.STATE.RunStep) {
                retState = RunningTestsController.Status.RUNNING;
            }
            switch (bpt_state) {
                case UBA_PROTO_BPT.STATE.RunStep:
                    retState = RunningTestsController.Status.RUNNING;
                    break;
                case UBA_PROTO_BPT.STATE.Pause:
                    retState = RunningTestsController.Status.PAUSED;
                    break;
                case UBA_PROTO_BPT.STATE.TestFailed:
                    retState = RunningTestsController.Status.ABORTED;
                    break;
                case UBA_PROTO_BPT.STATE.TestCompleate:
                    retState = RunningTestsController.Status.FINISHED;
                    break;
                case UBA_PROTO_BPT.STATE.StepCompleate:
                    retState = RunningTestsController.Status.RUNNING;
                    break;
                case UBA_PROTO_BPT.STATE.Init:
                    retState = RunningTestsController.Status.STANDBY;
                    break;
            }

            return (int)retState;
        }

        protected async Task AddUBA2List(CancellationToken stoppingToken) {
            ////_logger.LogInformation("AddUBA2List: Add UBA devices to list");
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogInformation("2 No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);
                } else {
                    ////_logger.LogInformation("Found {Count} UBA devices.", ubaDeviceDtos.Count);
                    foreach (var ubaDto in ubaDeviceDtos) {
                        //check if UBA with the same SN already exists
                        if (!UBAs.Any(uba => uba.SerialNumber.Equals(ubaDto.UbaSN))) {
                            //add UBA device to UBAs list
                            UBA_Interface? intrefaceCOM = UBA_Interfaces.FirstOrDefault(ui => ui.PortName == ubaDto.ComPort);
                            UBA6 newUba = new UBA6(_ubaLogger, intrefaceCOM, ubaDto.UbaSN);
                            newUba.Address = uint.TryParse(ubaDto.Address, out var addr) ? addr : 0;
_logger.LogInformation("==> 3-Add UBA {address} {sn} {comPort}", newUba.Address, ubaDto.UbaSN, ubaDto.ComPort);
//_logger.LogInformation("==> get message 4");
                            var result = await newUba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device);
                            if (result != null) {
                                UBAs.Add(newUba);
                                //_logger.LogInformation($"Added new UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");

                                //update UBA running test
                                //_logger.LogInformation($"Update Running Tests for UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");
//_logger.LogInformation("==> get message 5");
                                Message message = null;//await UBAs.First().GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                                for (int i = 0; i < UBAs.Count; i++)
                                {
                                    if (UBAs[i].Address.ToString() == ubaDto.Address)
                                    {
_logger.LogInformation("3.3 AddUBA2List GetMessage");
                                        message =  await UBAs[i].GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                                        break;
                                    }
                                }
                                //_logger.LogInformation($"Message: {message}");
                                await wcs.UpdateTestReadingData(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);
                            }
                        }                     
                    }
                }
            } catch (Exception ex) {
                _logger.LogError(ex.Message);
            }
        }

        protected async Task updateUBA2List(List<GETPendingTestResponseDTO>? pt) {
            ////_logger.LogInformation("updateUBA2List: Update UBA devices to list");
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogInformation("3 No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);

                    for (int i = UBAs.Count - 1; i >= 0; i--)
                    {
_logger.LogInformation($"==> 1 Remove UBA: {UBAs[i]}");
//                        UBAs[i].Dispose();    // if needed
                        UBAs.RemoveAt(i);
                       
                        foreach (var pendingTest in pt) {
                            if (pt.Find(pendingTest => pendingTest.UbaSN == UBAs[i].SerialNumber) == null) {
_logger.LogInformation($"==> 1 Pending Test: {pendingTest.UbaSN}");
                                pt.Remove(pendingTest);
                                break;
                            }
                        }                    
                    }

                } else {
                    ////_logger.LogInformation("Found {Count} UBA devices.", ubaDeviceDtos.Count);
                    foreach (var ubaDto in ubaDeviceDtos) {
                        //check if UBA with the same SN already exists
                        if (!UBAs.Any(uba => uba.SerialNumber.Equals(ubaDto.UbaSN))) {
                            UBA_Interface? intrefaceCOM = UBA_Interfaces.FirstOrDefault(ui => ui.PortName == ubaDto.ComPort);
                            UBA6 newUba = new UBA6(_ubaLogger, intrefaceCOM, ubaDto.UbaSN);
                            newUba.Address = uint.TryParse(ubaDto.Address, out var addr) ? addr : 0;

                            //check if to add UBA device to UBAs list
                            if (UBAs.Find(uba => uba.SerialNumber.Equals(newUba.SerialNumber)) == null) {
//_logger.LogInformation("==> get message 6");
                                var result = await newUba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device);
                                if (result != null) {
                                    UBAs.Add(newUba);
                                    _logger.LogInformation($"Added new UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");
                                } else {
                                    _logger.LogError($"7-No Response from UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");

                                    foreach (GETPendingTestResponseDTO pendingTest in pt) {
                                        ////_logger.LogInformation("==> STOPPED, ch {channel} ...", pendingTest.Channel);
                                        newUba.StopBPT(util.GetChannelFormDTO(pendingTest));
                                    }

_logger.LogInformation("==> Remove UBA: updateUBA2List");
                                    //remove UBA device - no response
                                    newUba.Dispose();
                                    newUba = null;
                                }
                            }
                        }                     
                    }

                    for (int i = UBAs.Count - 1; i >= 0; i--)
                    {
                        if (ubaDeviceDtos.Find(ubaDto => ubaDto.UbaSN == UBAs[i].SerialNumber) == null) {
_logger.LogInformation($"==> 2 Remove UBA: {UBAs[i]}");
//                            UBAs[i].Dispose();    // if needed
                            UBAs.RemoveAt(i);
                        }
                    }                    
                }


            } catch (Exception ex) {
                _logger.LogError(ex.Message);
            }
        }


        protected async Task updateRunningTestData() {
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogInformation("4 No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);
                } else {
                    foreach (var ubaDto in ubaDeviceDtos) {
                        int Status = (int) ubaDto.Status;
                        //update UBA running test
                        try {
                            Message message = null;//await UBAs.First().GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                            for (int i = 0; i < UBAs.Count; i++)
                            {
                                if (UBAs[i].Address.ToString() == ubaDto.Address)
                                {
//                                    message =  await UBAs[i].GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                                    break;
                                }
                            }
                            await wcs.UpdateTestReadingData(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);

                            if (ubaDto.Channel.Equals("A") && (channelStatus [0] != (int)message.QueryResponse.Bpt.State)) {
                                channelStatus [0] = (int)message.QueryResponse.Bpt.State;
                                if ((message.QueryResponse.Bpt.State == UBA_PROTO_BPT.STATE.RunStep) ||
                                    (message.QueryResponse.Bpt.State == UBA_PROTO_BPT.STATE.Pause) ||
                                    (message.QueryResponse.Bpt.State == UBA_PROTO_BPT.STATE.StepCompleate)) {
                                    GETPendingTestResponseDTO pendingTestResponseDTO = new GETPendingTestResponseDTO();
                                    pendingTestResponseDTO.Id = ubaDto.RunningTestID;
                                    pendingTestResponseDTO.Channel = ubaDto.Channel;
                                    pendingTestResponseDTO.UbaSN = ubaDto.UbaSN;
_logger.LogInformation($"==> 3.1.pendingTestResponseDTO: RUNNING {Status} {message.QueryResponse.Bpt.StartTime}", Status, message.QueryResponse.Bpt.StartTime);
                                    await wcs.ChangeRunningTestStatus(pendingTestResponseDTO, (int)ubaDto.Status);//RunningTestsController.Status.RUNNING);

                                } else if ((message.QueryResponse.Bpt.State == UBA_PROTO_BPT.STATE.Standby) ||
                                           (message.QueryResponse.Bpt.State == UBA_PROTO_BPT.STATE.TestCompleate) ||
                                           (message.QueryResponse.Bpt.State == UBA_PROTO_BPT.STATE.Init)) {
                                    GETPendingTestResponseDTO pendingTestResponseDTO = new GETPendingTestResponseDTO();
                                    pendingTestResponseDTO.Id = ubaDto.RunningTestID;
                                    pendingTestResponseDTO.Channel = ubaDto.Channel;
                                    pendingTestResponseDTO.UbaSN = ubaDto.UbaSN;
_logger.LogInformation($"==> 3.2.pendingTestResponseDTO: STANDBY {Status} channelStatus {channelStatus[0]}", Status, channelStatus[0]);
//                                    if ((channelStatus [0] == (int)UBA_PROTO_BPT.STATE.RunStep) ||
//                                        (channelStatus [0] == (int)UBA_PROTO_BPT.STATE.Pause) ||
//                                        (channelStatus [0] == (int)UBA_PROTO_BPT.STATE.StepCompleate)) {
                                        await wcs.ChangeRunningTestStatus(pendingTestResponseDTO, (int)(int)ubaDto.Status);//RunningTestsController.Status.STOPPED);
//                                    } else {
//                                        await wcs.ChangeRunningTestStatus(pendingTestResponseDTO, (int)RunningTestsController.Status.STANDBY);                                        
//                                    }
                                }
                                channelStatus [0] = (int)message.QueryResponse.Bpt.State;
                            }
                        } catch {
                            //_logger.LogInformation($"Trying to update Running Tests for UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac} CH: {ubaDto.Channel} testName: {ubaDto.TestName}");
                            //_logger.LogError($"8-No Response from UBA Device on Port");
                        }
                    }
                }
            } catch (Exception ex) {
                _logger.LogError(ex.Message);
            }
        }

        protected async Task updateRunningTestStatus(List<GETPendingTestResponseDTO>? pt) {
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogInformation("5 No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);
                } else {
                    foreach (var ubaDto in ubaDeviceDtos) {
                        //_logger.LogInformation($"updateRunningTestStatus: {ubaDto.Address}");
                        //if (!UBAs.Any(uba => uba.SerialNumber.Equals(ubaDto.UbaSN))) {
                            //update UBA running test
                            try {
                                if (ubaDto.Channel.Equals("A") && (testInProgress[0] == false)) {
                                    Message message = null;//await UBAs.First().GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                                    for (int i = 0; i < UBAs.Count; i++)
                                    {
                                        if (UBAs[i].Address.ToString() == ubaDto.Address)
                                        {
//                                            message =  await UBAs[i].GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                                            break;
                                        }
                                    }
                                    if (message != null)
                                    {
//                                    _logger.LogInformation($"QueryResponse: {message.QueryResponse.Bpt}");
                                        await wcs.UpdateTestStatus(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);
                                    }
                                }  
                                if (ubaDto.Channel.Equals("B") && (testInProgress[1] == false)) {
                                    Message message = null;// await UBAs.First().GetMessage(ubaDto.Channel.Equals("B") ? UBA_PROTO_QUERY.RECIPIENT.BptB : UBA_PROTO_QUERY.RECIPIENT.BptA);
                                    for (int i = 0; i < UBAs.Count; i++)
                                    {
                                        if (UBAs[i].Address.ToString() == ubaDto.Address)
                                        {
//                                            message =  await UBAs[i].GetMessage(ubaDto.Channel.Equals("B") ? UBA_PROTO_QUERY.RECIPIENT.BptB : UBA_PROTO_QUERY.RECIPIENT.BptA);
                                            break;
                                        }
                                    }
                                    if (message != null)
                                    {
//                                    _logger.LogInformation($"QueryResponse: {message.QueryResponse.Bpt}");
                                        await wcs.UpdateTestStatus(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);
                                    }
                                }

                            } catch {
                                _logger.LogInformation($"Trying to update Running Tests for UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac} CH: {ubaDto.Channel} testName: {ubaDto.TestName}");
                                //_logger.LogError($"9-No Response from UBA Device on Port");
                            }
                        //}                     
                    }
                }
            } catch (Exception ex) {
                _logger.LogError(ex.Message);
            }
        }

        protected async Task AddUBAsAsync(CancellationToken stoppingToken) {

            int timeout = 1000;
            var tcs = new TaskCompletionSource<Message?>();

            _logger.LogInformation("Add UBA's");
            while (true) {
                CancellationTokenSource timeoutCts = new(timeout);
                var stopwatch = System.Diagnostics.Stopwatch.StartNew();

                try { 
                    var delayTask = Task.Delay(timeout);
                    var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                    stopwatch.Stop();
                    //add UBA's:
//                    _logger.LogInformation("add UBA's");
                    await AddUBA2List(stoppingToken);
                    await Task.Delay(200/*msec delay*/, stoppingToken);
                } finally {
                }
            }
            _logger.LogInformation("Initialization exit.");
        }

        protected async Task InitAsync(CancellationToken stoppingToken) {
            try {
                if (!isInitialized) {
                    _logger.LogInformation("Initializing UBA Windows Service...");
                    await wcs.CreateStstion();
                    await addIntreface(stoppingToken);
                    _logger.LogInformation("Initialization complete.");
                    isInitialized = true;






                    //add UBA's:

                    await AddUBA2List(stoppingToken);
                }
            } catch (Exception ex) {
                //old version: _logger.LogError(ex, "Failed to initialize UBA Windows Service.");
                _logger.LogError("Failed to initialize UBA Windows Service.");
            }
        }

        private async Task addIntreface(CancellationToken stoppingToken) {
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogWarning("6 No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);

                } else {
                    _logger.LogDebug("Found {Count} UBA devices.", ubaDeviceDtos.Count);
                    // Check if UBA with the same SN already exists
                    foreach (var ubaDto in ubaDeviceDtos) {
                        if (!UBA_Interfaces.Any(ui => ui.PortName == ubaDto.ComPort)) {
_logger.LogInformation("==> 2-Add Interface {sn} {address} {comPort}", ubaDto.UbaSN, ubaDto.Address, ubaDto.ComPort);
                            UBA_Interfaces.Add(new UBA_Interface(_comLoger, ubaDto.ComPort));
                            _logger.LogInformation($"Added new UBA Interface: Port: {ubaDto.ComPort}");
                        } else {
                            _logger.LogInformation($"UBA Interface with Port: {ubaDto.ComPort} already exists. ubaSN {ubaDto.UbaSN}");
                        }
                    }
                }
            } catch (Exception ex) {
                _logger.LogCritical(ex.Message);
            }
        }
    }
}
