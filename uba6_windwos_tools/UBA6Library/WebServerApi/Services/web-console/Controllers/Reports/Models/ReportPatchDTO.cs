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
}
