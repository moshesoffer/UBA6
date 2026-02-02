using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models {
    public class PendingReportDTO {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("ubaSN")]
        public string UbaSN { get; set; }

        [JsonPropertyName("channel")]
        public string Channel { get; set; }

        [JsonPropertyName("timestampStart")]
        public DateTime TimestampStart { get; set; }

        [JsonPropertyName("status")]
        public int Status { get; set; }

        [JsonPropertyName("testName")]
        public string TestName { get; set; }

        [JsonPropertyName("batteryPN")]
        public string BatteryPN { get; set; }

        [JsonPropertyName("batterySN")]
        public string BatterySN { get; set; }

        [JsonPropertyName("cellPN")]
        public string CellPN { get; set; }

        [JsonPropertyName("chemistry")]
        public string Chemistry { get; set; }

        [JsonPropertyName("noCellSerial")]
        public int NoCellSerial { get; set; }

        [JsonPropertyName("noCellParallel")]
        public int NoCellParallel { get; set; }

        [JsonPropertyName("maxPerBattery")]
        public double MaxPerBattery { get; set; }

        [JsonPropertyName("ratedBatteryCapacity")]
        public double RatedBatteryCapacity { get; set; }

        [JsonPropertyName("notes")]
        public string Notes { get; set; }

        [JsonPropertyName("customer")]
        public string Customer { get; set; }

        [JsonPropertyName("workOrderNumber")]
        public string WorkOrderNumber { get; set; }

        [JsonPropertyName("approvedBy")]
        public string ApprovedBy { get; set; }

        [JsonPropertyName("conductedBy")]
        public string ConductedBy { get; set; }

        [JsonPropertyName("cellSupplier")]
        public string CellSupplier { get; set; }

        [JsonPropertyName("cellBatch")]
        public string CellBatch { get; set; }

        [JsonPropertyName("plan")]
        public List<PlanStepDTO> Plan { get; set; }

        [JsonPropertyName("testRoutineChannels")]
        public string TestRoutineChannels { get; set; }

        [JsonPropertyName("machineMac")]
        public string MachineMac { get; set; }

        [JsonPropertyName("machineName")]
        public string MachineName { get; set; }

      /*  [JsonPropertyName("timeOfTest")]
        public DateTime? TimeOfTest { get; set; }*/

        [JsonPropertyName("createdTime")]
        public DateTime CreatedTime { get; set; }

        [JsonPropertyName("modifiedTime")]
        public DateTime ModifiedTime { get; set; }
    }
}
