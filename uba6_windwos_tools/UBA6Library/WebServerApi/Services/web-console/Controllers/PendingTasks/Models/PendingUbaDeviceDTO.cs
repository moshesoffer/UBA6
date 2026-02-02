using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models {

    public class PendingUbaDeviceDTO {
        [JsonPropertyName("pendingUbaDevice")]
        public PendingUbaDeviceDTO2 t { get; set; } = new PendingUbaDeviceDTO2();
       
    }
    public class PendingUbaDeviceDTO2 {
        [JsonPropertyName("machineMac")]
        public string MachineMac { get; set; }

        [JsonPropertyName("address")]
        public string Address { get; set; }

        [JsonPropertyName("comPort")]
        public string ComPort { get; set; }

        [JsonPropertyName("ubaSN")]
        public string UbaSN { get; set; }

        [JsonPropertyName("ubaChannel")]
        public string UbaChannel { get; set; }

        [JsonPropertyName("name")]
        public string Name { get; set; }

        [JsonPropertyName("actionResult")]
        public string ActionResult { get; set; }

        [JsonPropertyName("action")]
        public string Action { get; set; }

        [JsonPropertyName("fwVersion")]
        public string FwVersion { get; set; }

        [JsonPropertyName("hwVersion")]
        public string HwVersion { get; set; }
    }
}
