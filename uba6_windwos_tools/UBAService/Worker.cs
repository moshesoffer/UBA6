using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System.Collections.Generic;
using System.Runtime;
using UBA_MSG;
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
        private static TimeSpan delay = TimeSpan.FromSeconds(3);

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
                        //await Task.Delay(delay*10, stoppingToken);
                    } else {
                        GETPendingTasksDTO pt = await wcs.GetPendingTasks();
                        if (pt != null) {
                            if (pt?.PendingRunningTests?.Count == 0 && pt?.PendingConnectionUbaDevices?.Count == 0) {
                                refreshChannleReading();
                            } else if (pt?.PendingRunningTests?.Count > 0) {
                                resolvePendingRunningTest(pt?.PendingRunningTests);
                            } else if (pt?.PendingConnectionUbaDevices?.Count > 0) {
                                resolvePendingUBA(pt?.PendingConnectionUbaDevices);
                                await addIntreface(stoppingToken);
                                await AddUBA2List(stoppingToken);
                            }
                        }
                    }
                    await Task.Delay(delay, stoppingToken);
                }
            } catch (Exception ex) {
                _logger.LogError(ex, "An error occurred in the UBA Service: {Message}", ex.Message);
            } finally {
                _logger.LogInformation("Service stopping.");
            }
        }
        private UBA6 getUbaFromList(string SN) {
            UBA6? existingUba = UBAs.FirstOrDefault(uba => uba.SerialNumber.Equals(SN));
            return existingUba;            
        }
        private async Task refreshChannleReading() {
            List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
            if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                _logger.LogWarning("No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);
            }
            foreach (var ubaDto in ubaDeviceDtos) {
                UBA6? existingUba = UBAs.FirstOrDefault(uba => uba.SerialNumber.Equals(ubaDto.UbaSN));
                if (existingUba == null) {
                    _logger.LogWarning($"UBA Device with SN: {ubaDto.UbaSN} not found in the current list. need to be added.");
                } else {
                    _logger.LogInformation($"UBA Device with SN: {ubaDto.UbaSN} found in the current list.");
                    Message message = await UBAs.First().GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                    if (message != null) {
                        await wcs.UpdateTestReadingData(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);
                        if ((((RunningTestsController.Status)ubaDto.Status) & RunningTestsController.Status.PENDING) == 0) {
                            await wcs.ChangeRunningTestStatus(ubaDto, Bptstate2DTOstate(message.QueryResponse.Bpt.State, message.QueryResponse.Bpt.StepType));
                        }

                    }

                }
            }
        }

        private async Task resolvePendingRunningTest(List<GETPendingTestResponseDTO>? pt) {
            try {
                _logger.LogInformation("Resolving pending UBA Test...");
                if (pt == null || pt.Count == 0) {
                    _logger.LogInformation("No pending tests found.");
                    return;
                }
                foreach (GETPendingTestResponseDTO pendingTest in pt) {
                    UBA6 uba = getUbaFromList(pendingTest.UbaSN);
                    if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.RUNNING) > 0) {
                        await uba.UpdatedTime();
                        Test_Routine_Message msg = new Test_Routine_Message();
                        msg.Index = util.GetIndexFormDTO(pendingTest);
                        msg.Tr = util.GETPendingTestResponseDTO2TR_Message(pendingTest);
                        await uba.SentMessageAsync(UBA_Message_Factory.CreateMessage(uba.Address, msg), UBA_Interface.MessagePriority.TEST_ROUTINE);
                        await Task.Delay(500);
                        uba.StartBPT(util.GetChannelFormDTO(pendingTest), util.GetIndexFormDTO(pendingTest));
                    } else if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.STOPPED) > 0) {
                        uba.StopBPT(util.GetChannelFormDTO(pendingTest));
                    } else if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.PAUSED) > 0) {
                        uba.PasueBPT(util.GetChannelFormDTO(pendingTest));
                    } else if ((((RunningTestsController.Status)pendingTest.Status) & RunningTestsController.Status.STANDBY) > 0) {
                        string filename = await uba.GetRunningTestFileName(util.GetChannelFormDTO(pendingTest));
                        _logger.LogInformation("Stopping BPT on channel {Channel} with file {FileName}", util.GetChannelFormDTO(pendingTest), filename);
                        uba.ClearBPT(util.GetChannelFormDTO(pendingTest));
                        // TODO: input filename
                        await wcs.TestResultUpdateStatus(pendingTest.ReportId,RunningTestsController.Status.PENDING | RunningTestsController.Status.SAVED);
                            _ = uba.FeatchFileToByteArray(filename).ContinueWith(task => {
                            byte[] file = task.Result;
                            _ = wcs.TestResultUpload(pendingTest.ReportId, file);
                        });
                    }
                    await wcs.ChangeRunningTestStatus(pendingTest, (int)((uint)pendingTest.Status & ~(uint)RunningTestsController.Status.PENDING));
                }
            } catch (Exception ex) {
                _logger.LogError(ex, "An error occurred while resolving pending tests: {Message}", ex.Message);
            }
        }

        private async Task resolvePendingReports(List<PendingReportDTO>? PR_List) {
            try {
                if (PR_List == null || PR_List.Count == 0) {
                    _logger.LogInformation("No pending tests found.");
                    return;
                }
                foreach (PendingReportDTO pr in PR_List) {
                    if (UBAs.Count > 0) {
                        List<string> filelist = await UBAs.First().FeatchFileList();
                        byte[]? file = await UBAs.First().FeatchFileToByteArray(filelist.Last());
                        await wcs.TestResultUpload(pr, file);
                    }
                }
            } catch (Exception e) {
                _logger.LogError("Error in resolvePendingReports: {Message}", e.Message);
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
                    _logger.LogInformation("Interface with PortName {PortName} {Exists}", pendingDevice.ComPort, exists ? "exists" : "does not exist");
                    if (exists == false) {
                        UBA_Interfaces.Add(new UBA_Interface(_comLoger, pendingDevice.ComPort));
                    }
                    UBA_Interface? UbaComInterface = UBA_Interfaces.FirstOrDefault(ui => ui.PortName == pendingDevice.ComPort);
                    if (UbaComInterface != null) {
                        Message? t = await UbaComInterface.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device, Convert.ToUInt32(pendingDevice.Address));
                        if (t != null) {
                            _logger.LogInformation($"Received message from UBA Device  {t?.QueryResponse}");
                            await wcs.DeviceFound(t.QueryResponse, pendingDevice.ComPort);
                        } else {
                            _logger.LogWarning($"No response from UBA Device on Port {pendingDevice.ComPort} at Address {pendingDevice.Address}");
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
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogWarning("No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);
                } else {
                    _logger.LogDebug("Found {Count} UBA devices.", ubaDeviceDtos.Count);
                    // Check if UBA with the same SN already exists
                    foreach (var ubaDto in ubaDeviceDtos) {
                        if (!UBAs.Any(uba => uba.SerialNumber.Equals(ubaDto.UbaSN))) {
                            UBA_Interface? intrefaceCOM = UBA_Interfaces.FirstOrDefault(ui => ui.PortName == ubaDto.ComPort);
                            UBA6 newUba = new UBA6(_ubaLogger, intrefaceCOM, ubaDto.UbaSN);
                            newUba.Address = uint.TryParse(ubaDto.Address, out var addr) ? addr : 0;
                            await newUba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device);
                            UBAs.Add(newUba);
                            _logger.LogInformation($"Added new UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");
                        } else {
                            _logger.LogInformation($"UBA Device with SN: {ubaDto.UbaSN} already exists.");
                        }
                        _logger.LogInformation($"UBA Device: {ubaDto.Name}, SN: {ubaDto.UbaSN}, MAC: {ubaDto.MachineMac}");
                        Message message = await UBAs.First().GetMessage(ubaDto.Channel.Equals("A") ? UBA_PROTO_QUERY.RECIPIENT.BptA : UBA_PROTO_QUERY.RECIPIENT.BptB);
                        await wcs.UpdateTestReadingData(ubaDto.RunningTestID, message.QueryResponse.Bpt, true);
                    }
                }
            } catch (Exception ex) {
                _logger.LogCritical(ex.Message);
            }
        }
        protected async Task InitAsync(CancellationToken stoppingToken) {
            try {
                if (!isInitialized) {
                    _logger.LogInformation("Initializing UBA Windows Service...");
                    await wcs.CreateStstion();
                    await addIntreface(stoppingToken);
                    await AddUBA2List(stoppingToken);
                    _logger.LogInformation("Initialization complete.");
                    isInitialized = true;
                }
            } catch (Exception ex) {
                _logger.LogError(ex, "Failed to initialize UBA Windows Service.");
            }
        }

        private async Task addIntreface(CancellationToken stoppingToken) {
            try {
                List<UbaDeviceDto> ubaDeviceDtos = await wcs.GetStationUBAs();
                if (ubaDeviceDtos == null || ubaDeviceDtos.Count == 0) {
                    _logger.LogWarning("No UBA devices found. Retrying in {Retry} seconds.", _settings.RetryCount);

                } else {
                    _logger.LogDebug("Found {Count} UBA devices.", ubaDeviceDtos.Count);
                    // Check if UBA with the same SN already exists
                    foreach (var ubaDto in ubaDeviceDtos) {
                        if (!UBA_Interfaces.Any(ui => ui.PortName == ubaDto.ComPort)) {
                            UBA_Interfaces.Add(new UBA_Interface(_comLoger, ubaDto.ComPort));
                            _logger.LogInformation($"Added new UBA Interface: Port: {ubaDto.ComPort}");
                        } else {
                            _logger.LogInformation($"UBA Interface with Port: {ubaDto.ComPort} already exists.");
                        }
                    }
                }
            } catch (Exception ex) {
                _logger.LogCritical(ex.Message);
            }
        }

    }
}
