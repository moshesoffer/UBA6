using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks;
using UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models;
using UBA6Library.WebServerApi.Services.web_console.Controllers.Reports;
using UBA6Library.WebServerApi.Services.web_console.Controllers.Reports.Models;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;
using UBA6Library.WebServerApi.Services.WebConsole.Model;
using System.Text.Json;
using System.Reflection.Metadata;
using Google.Protobuf;

namespace UBA6Library.WebServerApi.Services.WebConsole {
    public class WebConsoleService : WebService {

        protected ILogger<WebConsoleService> _logger { get; set; }
        public HttpClient Client { get; set; } = new HttpClient();
        public AuthController AuthController { get; private set; }
        public MachinesController MachinesController { get; private set; }
        public UBADevicesController UBADevices { get; private set; }
        public RunningTestsController RT_Controller { get; private set; }
        public PendingTasksController PendingTasksController { get; private set; }
        public ReportsController ReportsController { get; private set; }

        public WebConsoleService(ILogger<WebConsoleService> logger, string host, string port) : base(host, port, "web-console") {
            _logger = logger;
            AuthController = new AuthController(this);
            MachinesController = new MachinesController(this);
            UBADevices = new UBADevicesController(this);
            RT_Controller = new RunningTestsController(this);
            PendingTasksController = new PendingTasksController(this);
            ReportsController = new ReportsController(this);


            Controllers.Add(AuthController);
            Controllers.Add(MachinesController);
            Controllers.Add(UBADevices);
            Controllers.Add(RT_Controller);
            Controllers.Add(PendingTasksController);
            Controllers.Add(ReportsController);
            logger.LogInformation($"WebConsoleService initialized at {servicePath} with {Controllers.Count} controllers.");
        }


        public async Task CreateStstion() {
            // Implementation for creating a station
            _logger.LogInformation("Creating station...");
            MachineDTO nm = new MachineDTO();
            nm.Name = $"{Environment.MachineName} - {GetMacAddress()}";
            nm.Ip = GetLocalIPv4();
            nm.Mac = GetMacAddress();
            List<MachineDTO> ml = await MachinesController.WebRq.Get<List<MachineDTO>>(Client);
            if (ml.Any(m => m.Mac == nm.Mac)) {
                _logger.LogWarning($"Machine with MAC {nm.Mac} already exists. Skipping creation.");
                return;
            }

            await MachinesController.WebRq.Post<object, MachineDTO>(Client, nm);
        }


        public override string ToString() {
            return $"{serviceName} v{version}";
        }                                     
        public async Task<List<UbaDeviceDto>> GetStationUBAs() {
            ////_logger.LogInformation("Retrieving UBA devices for the station...");
            UBADevicesResponseDTO m = await UBADevices.WebRq.Get<UBADevicesResponseDTO>(Client);
            List<UbaDeviceDto> matchingUBAs = m.UbaDevices
                .Where(uba => uba.MachineMac == GetMacAddress())
                .ToList();

//_logger.LogInformation($"==> msg: {matchingUBAs.Count()}, {JsonSerializer.Serialize(matchingUBAs)}");
//Moshe: wrong delete. need to based on ubaChannel field!
//            //remove duplications    
//            foreach (var uba in matchingUBAs.ToList()) {
//                string UbaSN = uba.UbaSN;
//                var count = 0;
//                var index = 0;
//                foreach (var uba1 in matchingUBAs.ToList()) {
//                    if (matchingUBAs.Any(uba => uba1.UbaSN.Equals(UbaSN))) {
//                        count = count + 1;
//                        if (count > 1)
//                        {
//                            count = count - 1;
//                            matchingUBAs.RemoveRange(index,1);//RemoveAt(index); 
//                        }
//                    }
//                    index++;
//                }
//            }

            ////_logger.LogInformation($"=========> List Count: {matchingUBAs.Count()}");
            ////foreach (UbaDeviceDto uba in matchingUBAs) {
            ////    _logger.LogInformation($"UBA Device: {uba.Name}, SN: {uba.UbaSN}, MAC: {uba.MachineMac}");
            ////}
            return matchingUBAs;
        }
       
        public async Task DeleteStationUBA(UbaDeviceDto ubaDeviceDto)
        {
            _logger.LogInformation($"Updating UBA List");
            UBADevicesResponseDTO m = await UBADevices.WebRq.Get<UBADevicesResponseDTO>(Client);
            var devicesToDelete = m.UbaDevices
                .Where(uba => uba.MachineMac == ubaDeviceDto.Address)
                .ToList();

            foreach (var uba in devicesToDelete)
            {
                string json = JsonSerializer.Serialize(new { ubaSN = uba.MachineMac });
                await UBADevices.WebRq.Delete<object>(Client, json);
                break;
            }
        }

        public async Task ChangeRunningTestStatus(GETPendingTestResponseDTO pt, int status) {
            ////_logger.LogInformation($"Changing status of running test {pt.Id} to {status}...");
            PATCH_ChangeTR_StatusRequest pATCH_ChangeTR_StatusRequest = new PATCH_ChangeTR_StatusRequest();
            pATCH_ChangeTR_StatusRequest.RunningTestID = pt.Id;
            pATCH_ChangeTR_StatusRequest.TestRoutineChannels = pt.Channel;
            pATCH_ChangeTR_StatusRequest.UbaSN = pt.UbaSN;
            pATCH_ChangeTR_StatusRequest.NewTestStatus = status == 0 ? 1 : status; 
            await RT_Controller.ChangeRunningTestStatus.Patch<object, PATCH_ChangeTR_StatusRequest>(Client, pATCH_ChangeTR_StatusRequest);
            ////_logger.LogInformation($"Running test {pATCH_ChangeTR_StatusRequest.RunningTestID} status changed to {pATCH_ChangeTR_StatusRequest.NewTestStatus}.");
        }
        public async Task ChangeRunningTestStatus(UbaDeviceDto dto, int status) {
            ////_logger.LogInformation($"Changing status of running test {dto.RunningTestID} to {status}...");
            PATCH_ChangeTR_StatusRequest pATCH_ChangeTR_StatusRequest = new PATCH_ChangeTR_StatusRequest();
            pATCH_ChangeTR_StatusRequest.RunningTestID = dto.RunningTestID;
            pATCH_ChangeTR_StatusRequest.TestRoutineChannels = dto.Channel;
            pATCH_ChangeTR_StatusRequest.UbaSN = dto.UbaSN;
            pATCH_ChangeTR_StatusRequest.NewTestStatus = status == 0 ? 1 : status;
            await RT_Controller.ChangeRunningTestStatus.Patch<object, PATCH_ChangeTR_StatusRequest>(Client, pATCH_ChangeTR_StatusRequest);
            ////_logger.LogInformation($"Running test {pATCH_ChangeTR_StatusRequest.RunningTestID} status changed to {pATCH_ChangeTR_StatusRequest.NewTestStatus}.");
        }

        public async Task UpdateChannelReadingData(Guid runningTestID, UBA_PROTO_CHANNEL.status msg) {
            InstantTestResultsDTO instantTestResultsDTO = new InstantTestResultsDTO();
            instantTestResultsDTO.RunningTestID = runningTestID;
            
            instantTestResultsDTO.Timestamp = DateTime.UtcNow;
            instantTestResultsDTO.TestState = msg.State.ToString();
            instantTestResultsDTO.TestCurrentStep = 0;
            instantTestResultsDTO.Voltage = msg.Data.Voltage;
            instantTestResultsDTO.Current = msg.Data.Current;
            instantTestResultsDTO.Temp = msg.Data.Temperature;
            instantTestResultsDTO.Capacity = msg.Data.Capacity;
            instantTestResultsDTO.Error = (int) msg.Error ;
            instantTestResultsDTO.IsLogData = 0;
            List<InstantTestResultsDTO> sadas = new List<InstantTestResultsDTO>() { instantTestResultsDTO };
            await RT_Controller.InstantTestResults.Post<object, List<InstantTestResultsDTO>>(Client, sadas);
        }

        public async Task UpdateTestReadingData(Guid runningTestID, UBA_PROTO_BPT.status_message msg, bool isLog = false) {
            InstantTestResultsDTO instantTestResultsDTO = new InstantTestResultsDTO();
            instantTestResultsDTO.RunningTestID = runningTestID;
            
            //_logger.LogInformation($"==> voltage: {msg.ChannelStatus.Data.Voltage}");
            uint timestamp = msg.StartTime;
//_logger.LogInformation($"==> timestamp: {timestamp} msg.State: {msg.State}");

            DateTime dateTime = timestamp == 0
                ? DateTime.MinValue
                : DateTimeOffset.FromUnixTimeSeconds(timestamp).DateTime;
            //DateTime dateTime = DateTimeOffset.FromUnixTimeSeconds(msg.StartTime).DateTime;     
            instantTestResultsDTO.Timestamp = DateTime.Now; //UtcNow;
            //_logger.LogInformation($"==> DateTime: {msg.StartTime} {instantTestResultsDTO.Timestamp}");

            if (msg.State == UBA_PROTO_BPT.STATE.RunStep) {
                instantTestResultsDTO.TestState = ((UBA_PROTO_CHANNEL.STATE)msg.ChannelStatus.State).ToString();                
            } else { 
                instantTestResultsDTO.TestState = msg.State.ToString();
            }
            instantTestResultsDTO.TestCurrentStep =(int) msg.CurrentStep;
            
            if (msg.ChannelStatus.Data != null) {
                //_logger.LogInformation($"==> date: {instantTestResultsDTO.Timestamp} state: {instantTestResultsDTO.TestState}");
                //_logger.LogInformation($"==> volt: {msg.ChannelStatus.Data.Voltage}, crnt: {msg.ChannelStatus.Data.Current}, temp: {msg.ChannelStatus.Data.Temperature}, cap: {msg.ChannelStatus.Data.Capacity}");
                instantTestResultsDTO.Voltage = msg.ChannelStatus.Data.Voltage;
                instantTestResultsDTO.Current = msg.ChannelStatus.Data.Current/1000.0f;
                instantTestResultsDTO.Temp = msg.ChannelStatus.Data.Temperature;
                instantTestResultsDTO.Capacity = msg.ChannelStatus.Data.Capacity;
                instantTestResultsDTO.Error = ((int)msg.Error ) | ((int)msg.ChannelStatus.Error) | ((int)msg.ChannelStatus.LineStatus[0].Error);
                instantTestResultsDTO.IsLogData = isLog ? 1:0;
                List<InstantTestResultsDTO> sadas = new List<InstantTestResultsDTO>() { instantTestResultsDTO };
                await RT_Controller.InstantTestResults.Post<object, List<InstantTestResultsDTO>>(Client, sadas); 
            }          
        }

        public async Task UpdateTestStatus(Guid runningTestID, UBA_PROTO_BPT.status_message msg, bool isLog = false) {
            int retState = (int)RunningTestsController.Status.STANDBY;
            switch (msg.State) {
                case UBA_PROTO_BPT.STATE.RunStep:
                    retState = (int)RunningTestsController.Status.RUNNING;
                    break;
                case UBA_PROTO_BPT.STATE.Pause:
                    retState = (int)RunningTestsController.Status.PAUSED;
                    break;
                case UBA_PROTO_BPT.STATE.TestFailed:
                    retState = (int)RunningTestsController.Status.ABORTED;
                    break;
                case UBA_PROTO_BPT.STATE.TestCompleate:
                    retState = (int)RunningTestsController.Status.FINISHED;
                    break;
                case UBA_PROTO_BPT.STATE.StepCompleate:
                    retState = (int)RunningTestsController.Status.RUNNING;
                    break;
                case UBA_PROTO_BPT.STATE.Init:
                    retState = (int)RunningTestsController.Status.STANDBY;
                    break;
            }

            PATCH_ChangeTR_StatusRequest pATCH_ChangeTR_StatusRequest = new PATCH_ChangeTR_StatusRequest();
            pATCH_ChangeTR_StatusRequest.RunningTestID = runningTestID;
            pATCH_ChangeTR_StatusRequest.TestRoutineChannels = msg.ChannelStatus.Id == 'A' ? "A" : "B";
            pATCH_ChangeTR_StatusRequest.UbaSN = "0";
            pATCH_ChangeTR_StatusRequest.NewTestStatus = retState; 

            await RT_Controller.ChangeRunningTestStatus.Patch<object, PATCH_ChangeTR_StatusRequest>(Client, pATCH_ChangeTR_StatusRequest);
        }

        public async Task<GETPendingTasksDTO> GetPendingTasks() {
            List<KeyValuePair<string, string>> querys = new List<KeyValuePair<string, string>>();
            querys.Add(new KeyValuePair<string, string>("machineMac", GetMacAddress()));
            GETPendingTasksDTO rts = await PendingTasksController.PendingTasks.Get<GETPendingTasksDTO>(Client, querys);
            return rts;
        }

        public async Task DeviceFound(UBA_PROTO_QUERY.query_response_message meg, string comPort) { 
            //_logger.LogInformation($"Notifying server about found device {meg}...");
            PendingUbaDeviceDTO p = new PendingUbaDeviceDTO();
            p.t.MachineMac = GetMacAddress();
            p.t.Address = meg.Device.Settings.Address.ToString();
            p.t.UbaSN = meg.Device.Settings.SN.ToString();
            p.t.ComPort = comPort;
            p.t.UbaChannel = "AB";
            p.t.Name = string.IsNullOrEmpty(meg.Device.Settings.Name) ?  "N/A" : meg.Device.Settings.Name;
            p.t.Action = "query";
            p.t.ActionResult = "success";
            p.t.FwVersion = "Testing"; //TODO: update with real fw version
            p.t.HwVersion = "12.12"; // TODO: update with real hw version



            await PendingTasksController.PendingTasks.Post<object ,PendingUbaDeviceDTO>(Client, p); 
        }

        public async Task TestResultUpload(PendingReportDTO pr, byte[] file)   {
            await TestResultUpload(pr.Id, file);

        }
        public async Task TestResultUpdateStatus(Guid? report_id, RunningTestsController.Status newStatus) {            
            ReportPatchDTO reportPatchDTO = new ReportPatchDTO();
            reportPatchDTO.Status = (int)newStatus;
            reportPatchDTO.TestResults = new List<TestResultDataPointDTO>();
            UBA_PROTO_DATA_LOG.data_log dummy = new UBA_PROTO_DATA_LOG.data_log() {
                Time = 0,
                Voltage = 0,
                Current = 0,
                Temp = 0,
                PlanIndex = 0,
                StepIndex = 0
            };
            reportPatchDTO.TestResults.Add(new TestResultDataPointDTO(dummy));
            await ReportsController.Reports.Patch<object, ReportPatchDTO>(Client, report_id.ToString(), reportPatchDTO);
        }

        public async Task TestResultUpload(Guid? report_id, byte[] file) {
            try {

                _logger.LogInformation($"Uploading test results for report ID {report_id}...");
                ReportPatchDTO reportPatchDTO = new ReportPatchDTO();
                reportPatchDTO.Status = (int)RunningTestsController.Status.FINISHED;
                reportPatchDTO.TestResults = new List<TestResultDataPointDTO>();
                List<UBA_PROTO_DATA_LOG.data_log> logs = ProtoHelper.DecodeDataLogMessages(file);
                foreach (UBA_PROTO_DATA_LOG.data_log log in logs) {
                    //_logger.LogDebug($"Log Entry {logs.Count} - Time: {log.Time}, Voltage: {log.Voltage}, Current: {log.Current}, Temp: {log.Temp}, PlanIndex: {log.PlanIndex}, StepIndex: {log.StepIndex}");
                    reportPatchDTO.TestResults.Add(new TestResultDataPointDTO(log));
                }

                if(report_id == null)
                {
                    _logger.LogInformation($"ReportPatchDTO undefined report_id, test start by UBA");
                    return;
                }
                try {
                    await ReportsController.Reports.Patch<object, ReportPatchDTO>(Client, report_id.ToString(), reportPatchDTO);
                } catch (Exception ex) {
                    var options = new System.Text.Json.JsonSerializerOptions {
                        WriteIndented = true,
                        DefaultIgnoreCondition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingNull
                    };
                    string json = System.Text.Json.JsonSerializer.Serialize(reportPatchDTO, options);
                    await System.IO.File.WriteAllTextAsync("temp.josn", json);
                    _logger.LogInformation($"ReportPatchDTO saved to temp.json");
                    _logger.LogCritical($"Failed to upload test results: {ex}, marking report as ABORTED.");
                    reportPatchDTO = new ReportPatchDTO();
                    reportPatchDTO.Status = (int)RunningTestsController.Status.ABORTED;
                    await ReportsController.Reports.Patch<object, ReportPatchDTO>(Client, report_id.ToString(), reportPatchDTO);
                }
            } catch (Exception ex) { 
                _logger.LogError(ex, $"Error uploading test results for report ID {report_id}: {ex.Message}");
            }

        }



    }
}
