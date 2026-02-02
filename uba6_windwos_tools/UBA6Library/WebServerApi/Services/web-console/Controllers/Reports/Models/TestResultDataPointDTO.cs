using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json.Serialization;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.Reports.Models {
    public class TestResultDataPointDTO {
            [JsonPropertyName("current")]
            public double Current { get; set; }

            [JsonPropertyName("voltage")]
            public double Voltage { get; set; }

            [JsonPropertyName("timestamp")]
            public uint Timestamp { get; set; }

            [JsonPropertyName("temperature")]
            public double Temperature { get; set; }

            [JsonPropertyName("planIndex")]
            public uint PlanIndex { get; set; }

            [JsonPropertyName("stepIndex")]
            public uint StepIndex { get; set; }
        public TestResultDataPointDTO() {
        }

        public TestResultDataPointDTO(UBA_PROTO_DATA_LOG.data_log data_log) :this(){ 
            Current = data_log.Current;
            Voltage = data_log.Voltage;
            Timestamp = data_log.Time;
            Temperature = data_log.Temp /100.0f;
            PlanIndex = data_log.PlanIndex;
            StepIndex = data_log.StepIndex;
        }
    }
}
