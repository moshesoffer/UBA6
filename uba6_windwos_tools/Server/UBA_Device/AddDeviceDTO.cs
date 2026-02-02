using System.Text.Json.Serialization;

namespace Server.UBA_Device {
    public class AddDeviceDTO : DeviceBaseDTO{
        [JsonPropertyName("name")]
        public string Name { get; set; }
    }
}
