using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Model {
    public class MachineDTO : UBA_JSON_DTO {
        [JsonPropertyName("mac")]
        public string Mac { get; set; }

        [JsonPropertyName("name")]
        public string Name { get; set; }

        [JsonPropertyName("ip")]
        public string Ip { get; set; }

        [JsonPropertyName("status")]
        public string Status { get; set; }

        [JsonPropertyName("createdTime")]
        public DateTime CreatedTime { get; set; }

        [JsonPropertyName("modifiedTime")]
        public DateTime ModifiedTime { get; set; }
    }
}
