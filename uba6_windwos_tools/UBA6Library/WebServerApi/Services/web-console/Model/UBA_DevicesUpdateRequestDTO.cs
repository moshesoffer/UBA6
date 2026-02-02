using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Model {
    public class UBA_DevicesUpdateRequestDTO : UBA_JSON_DTO {
    [JsonPropertyName("machineMac")]
        public string MachineMac { get; set; }

        [JsonPropertyName("name")]
        public string Name { get; set; }

        [JsonPropertyName("comPort")]
        public string ComPort { get; set; }

        [JsonPropertyName("address")]
        public string Address { get; set; }
    }
}

