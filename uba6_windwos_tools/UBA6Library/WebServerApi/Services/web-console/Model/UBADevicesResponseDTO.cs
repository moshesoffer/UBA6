using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Model {

    public class UBADevicesResponseDTO {
        [JsonPropertyName("ubaDevices")]
        public List<UbaDeviceDto> UbaDevices { get; set; }

        [JsonPropertyName("ubaTotal")]
        public UbaTotalDto UbaTotal { get; set; }
    }

    public class UbaDeviceDto {
        [JsonPropertyName("ubaSN")]
        public string UbaSN { get; set; }

        [JsonPropertyName("machineMac")]
        public string MachineMac { get; set; }

        [JsonPropertyName("name")]
        public string Name { get; set; }

        [JsonPropertyName("address")]
        public string Address { get; set; }

        [JsonPropertyName("comPort")]
        public string ComPort { get; set; }

        [JsonPropertyName("ubaChannel")]
        public string UbaChannel { get; set; }

        [JsonPropertyName("isConnected")]
        public int IsConnected { get; set; }

        [JsonPropertyName("lastConDis")]
        public DateTime? LastConDis { get; set; }

        [JsonPropertyName("createdTime")]
        public DateTime CreatedTime { get; set; }

        [JsonPropertyName("modifiedTime")]
        public DateTime ModifiedTime { get; set; }

        [JsonPropertyName("machineName")]
        public string MachineName { get; set; }

        [JsonPropertyName("runningTestID")]
        public Guid RunningTestID { get; set; }

        [JsonPropertyName("testName")]
        public string TestName { get; set; }

        [JsonPropertyName("channel")]
        public string Channel { get; set; }

        [JsonPropertyName("timestampStart")]
        public DateTime TimestampStart { get; set; }

        [JsonPropertyName("status")]
        public int Status { get; set; }

        [JsonPropertyName("batteryPN")]
        public string BatteryPN { get; set; }

        [JsonPropertyName("batterySN")]
        public string BatterySN { get; set; }

        [JsonPropertyName("testRoutineChannels")]
        public string TestRoutineChannels { get; set; }

        [JsonPropertyName("totalStagesAmount")]
        public int TotalStagesAmount { get; set; }

        [JsonPropertyName("testState")]
        public string TestState { get; set; }

        [JsonPropertyName("testCurrentStep")]
        public int? TestCurrentStep { get; set; }

        [JsonPropertyName("voltage")]
        public double? Voltage { get; set; }

        [JsonPropertyName("current")]
        public float? Current { get; set; }

        [JsonPropertyName("temp")]
        public float? Temp { get; set; }

        [JsonPropertyName("capacity")]
        public double? Capacity { get; set; }

        [JsonPropertyName("error")]
        public int? Error { get; set; }
    }

    public class UbaTotalDto {
        [JsonPropertyName("configured")]
        public int Configured { get; set; }

        [JsonPropertyName("connected")]
        public int Connected { get; set; }

        [JsonPropertyName("running")]
        public int Running { get; set; }
    }
}
