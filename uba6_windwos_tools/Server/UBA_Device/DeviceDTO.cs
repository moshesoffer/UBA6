using System.Text.Json.Serialization;

namespace Server.UBA_Device {
    public partial class DeviceDTO :AddDeviceDTO
    {
        [JsonPropertyName("id")]
        public uint Id { get; set; }
        
        [JsonPropertyName("channel"), JsonConverter(typeof(JsonStringEnumConverter))]
        public CHANNEL Channel { get; set; } = CHANNEL.NONE;

        private static UInt32 runningId = 0;

        public string Port { get; set; }
        public string Address { get; set; }
        public string TestName { get; set; }
        public string Status { get; set; }
        public string Runtime { get; set; }
        public string Reading { get; set; }
        public string Report { get; set; }
        public string Actions { get; set; }

        public static DeviceDTO CreateNewDevice(AddDeviceDTO baseDevice) {
            DeviceDTO t  = new DeviceDTO(); 
            t.Machine = baseDevice.Machine;
            t.Port = baseDevice.Port;   
            t.Address = baseDevice.Address;
            t.Name = baseDevice.Name;
            t.Id = runningId++;
            return t;
        }
        public static DeviceDTO CreateNewDevice(DeviceBaseDTO baseDevice) {
            DeviceDTO t = new DeviceDTO();
            return t;
        }


            public void Update(DeviceBaseDTO other) {

            return;
        }

        public void Update(DeviceDTO other)
        {

            return;
        }

    }

    public class TotalDeviceDTO{

        [JsonPropertyName("configured")]
        public uint Configured { get; set; }
        [JsonPropertyName("connected")]
        public uint Connected { get; set; }
        [JsonPropertyName("running")]
        public uint Running { get; set; }

        public TotalDeviceDTO(uint configured, uint connected, uint running)
        {
            Configured = configured;
            Connected = connected;
            Running = running;
        }
    }
}
