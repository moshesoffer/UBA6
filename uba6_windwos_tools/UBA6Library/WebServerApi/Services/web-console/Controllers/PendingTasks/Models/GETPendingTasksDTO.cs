using System.Text.Json.Serialization;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models {
    public class GETPendingTasksDTO {
        [JsonPropertyName("pendingReports")]
        public List<PendingReportDTO>? PendingReports { get; set; }
        [JsonPropertyName("pendingConnectionUbaDevices")]
        public List<PendingConnectionUbaDevice>? PendingConnectionUbaDevices { get; set; }
        [JsonPropertyName("pendingRunningTests")]
        public List<GETPendingTestResponseDTO>? PendingRunningTests { get; set; }
    }
}
