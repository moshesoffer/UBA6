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

        public async Task<List<UbaDeviceDto>> GetStaionUBAs() {
            _logger.LogInformation("Retrieving UBA devices for the station...");
            UBADevicesResponseDTO m = await UBADevices.WebRq.Get<UBADevicesResponseDTO>(Client);
            foreach (UbaDeviceDto uba in m.UbaDevices) {
                _logger.LogInformation($"UBA Device: {uba.Name}, SN: {uba.UbaSN}, MAC: {uba.MachineMac}");

            }



            return new List<UbaDeviceDto>();

        }

        public override string ToString() {
            return $"{serviceName} v{version}";
        }
        public async Task<List<UbaDeviceDto>> GetStationUBAs() {
            _logger.LogInformation("Retrieving UBA devices for the station...");
            UBADevicesResponseDTO m = await UBADevices.WebRq.Get<UBADevicesResponseDTO>(Client);
            List<UbaDeviceDto> matchingUBAs = m.UbaDevices
                .Where(uba => uba.MachineMac == GetMacAddress())
                .ToList();

            foreach (UbaDeviceDto uba in matchingUBAs) {
                _logger.LogInformation($"UBA Device: {uba.Name}, SN: {uba.UbaSN}, MAC: {uba.MachineMac}");
            }
            return matchingUBAs;
        }
       

        public async Task ChangeRunningTestStatus(GETPendingTestResponseDTO pt, int status) {
            _logger.LogInformation($"Changing status of running test {pt.Id} to {status}...");
            PATCH_ChangeTR_StatusRequest pATCH_ChangeTR_StatusRequest = new PATCH_ChangeTR_StatusRequest();
            pATCH_ChangeTR_StatusRequest.RunningTestID = pt.Id;
            pATCH_ChangeTR_StatusRequest.TestRoutineChannels = pt.Channel;
            pATCH_ChangeTR_StatusRequest.UbaSN = pt.UbaSN;
            pATCH_ChangeTR_StatusRequest.NewTestStatus = status == 0 ? 1 : status; 
            await RT_Controller.ChangeRunningTestStatus.Patch<object, PATCH_ChangeTR_StatusRequest>(Client, pATCH_ChangeTR_StatusRequest);
            _logger.LogInformation($"Running test {pATCH_ChangeTR_StatusRequest.RunningTestID} status changed to {pATCH_ChangeTR_StatusRequest.NewTestStatus}.");
        }
        public async Task ChangeRunningTestStatus(UbaDeviceDto dto, int status) {
            _logger.LogInformation($"Changing status of running test {dto.RunningTestID} to {status}...");
            PATCH_ChangeTR_StatusRequest pATCH_ChangeTR_StatusRequest = new PATCH_ChangeTR_StatusRequest();
            pATCH_ChangeTR_StatusRequest.RunningTestID = dto.RunningTestID;
            pATCH_ChangeTR_StatusRequest.TestRoutineChannels = dto.Channel;
            pATCH_ChangeTR_StatusRequest.UbaSN = dto.UbaSN;
            pATCH_ChangeTR_StatusRequest.NewTestStatus = status == 0 ? 1 : status;
            await RT_Controller.ChangeRunningTestStatus.Patch<object, PATCH_ChangeTR_StatusRequest>(Client, pATCH_ChangeTR_StatusRequest);
            _logger.LogInformation($"Running test {pATCH_ChangeTR_StatusRequest.RunningTestID} status changed to {pATCH_ChangeTR_StatusRequest.NewTestStatus}.");
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

        public async Task UpdateTestReadingData(Guid runningTestID, UBA_PROTO_BPT.status_message msg,bool isLog = false) {
            InstantTestResultsDTO instantTestResultsDTO = new InstantTestResultsDTO();
            instantTestResultsDTO.RunningTestID = runningTestID;
            instantTestResultsDTO.Timestamp = DateTime.UtcNow;
            if (msg.State == UBA_PROTO_BPT.STATE.RunStep) {
                instantTestResultsDTO.TestState = ((UBA_PROTO_CHANNEL.STATE)msg.ChannelStatus.State).ToString();                
            } else { 
                instantTestResultsDTO.TestState = msg.State.ToString();
            }
            instantTestResultsDTO.TestCurrentStep =(int) msg.CurrentStep;
            
            instantTestResultsDTO.Voltage = msg.ChannelStatus.Data.Voltage;
            instantTestResultsDTO.Current = msg.ChannelStatus.Data.Current/1000.0f;
            instantTestResultsDTO.Temp = msg.ChannelStatus.Data.Temperature;
            instantTestResultsDTO.Capacity = msg.ChannelStatus.Data.Capacity;
            instantTestResultsDTO.Error = ((int)msg.Error ) | ((int)msg.ChannelStatus.Error) | ((int)msg.ChannelStatus.LineStatus[0].Error);
            instantTestResultsDTO.IsLogData = isLog ? 1:0;
            List<InstantTestResultsDTO> sadas = new List<InstantTestResultsDTO>() { instantTestResultsDTO };
            await RT_Controller.InstantTestResults.Post<object, List<InstantTestResultsDTO>>(Client, sadas);            
        }

        public async Task<GETPendingTasksDTO> GetPendingTasks() {
            List<KeyValuePair<string, string>> querys = new List<KeyValuePair<string, string>>();
            querys.Add(new KeyValuePair<string, string>("machineMac", GetMacAddress()));
            GETPendingTasksDTO rts = await PendingTasksController.PendingTasks.Get<GETPendingTasksDTO>(Client, querys);
            return rts;
        }

        public async Task DeviceFound(UBA_PROTO_QUERY.query_response_message meg, string comPort) { 
            _logger.LogInformation($"Notifying server about found device {meg}...");
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
                    reportPatchDTO.TestResults.Add(new TestResultDataPointDTO(log));
                    _logger.LogDebug($"Log Entry - Time: {log.Time}, Voltage: {log.Voltage}, Current: {log.Current}, Temp: {log.Temp}, PlanIndex: {log.PlanIndex}, StepIndex: {log.StepIndex}");
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
