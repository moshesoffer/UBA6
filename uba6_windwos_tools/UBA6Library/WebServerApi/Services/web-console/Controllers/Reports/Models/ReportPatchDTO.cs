using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.Reports.Models {
    public class ReportPatchDTO {
        [JsonPropertyName("status")]
        public int Status { get; set; }

        [JsonPropertyName("testResults")]
        public List<TestResultDataPointDTO> TestResults { get; set; }
    }


    public class ReportCreateDTO
    {
        [JsonPropertyName("status")]
        public int Status { get; set; }

        [JsonPropertyName("testResults")]
        public List<TestResultDataPointDTO> TestResults { get; set; } = new();

        // Add the fields required when creating a report:
        [JsonPropertyName("testName")]
        public string? TestName { get; set; }

        [JsonPropertyName("machineId")]
        public Guid? MachineId { get; set; }

        [JsonPropertyName("runningTestId")]
        public Guid? RunningTestId { get; set; }
    }
}
