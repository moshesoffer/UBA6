using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models {
    public class PendingConnectionUbaDevice {
        [JsonPropertyName("machineMac")]
        public string MachineMac { get; set; }

        [JsonPropertyName("address")]
        public string Address { get; set; }

        [JsonPropertyName("comPort")]
        public string ComPort { get; set; }

        [JsonPropertyName("action")]
        public string Action { get; set; }
    }
}
