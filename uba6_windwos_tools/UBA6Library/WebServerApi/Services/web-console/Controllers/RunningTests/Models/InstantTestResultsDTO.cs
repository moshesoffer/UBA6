using System;
using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models {
    public class CustomDateTimeConverter : JsonConverter<DateTime> {
        private const string Format = "yyyy-MM-dd HH:mm:ss.fff";

        public override DateTime Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options) {
            var str = reader.GetString();
            return DateTime.ParseExact(str, Format, null);
        }

        public override void Write(Utf8JsonWriter writer, DateTime value, JsonSerializerOptions options) {
            writer.WriteStringValue(value.ToString(Format));
        }
    }
    public class InstantTestResultsDTO :UBA_JSON_DTO{
        [Required]
        [StringLength(36, MinimumLength = 36)]
        [JsonPropertyName("runningTestID")]
        public Guid RunningTestID { get; set; }

        [Required]
        [JsonPropertyName("timestamp")]
        [JsonConverter(typeof(CustomDateTimeConverter))]
        public DateTime Timestamp { get; set; }

        [Required]
        [StringLength(64)]
        [JsonPropertyName("testState")]
        public string TestState { get; set; }

        [Required]
        [Range(0, int.MaxValue)]
        [JsonPropertyName("testCurrentStep")]
        public int TestCurrentStep { get; set; }

        [Required]
        [JsonPropertyName("voltage")]
        public double Voltage { get; set; }

        [Required]
        [JsonPropertyName("current")]
        public double Current { get; set; }

        [Required]
        [JsonPropertyName("temp")]
        public double Temp { get; set; }

        [Required]
        [JsonPropertyName("capacity")]
        public double Capacity { get; set; }

        [Required]
        [JsonPropertyName("error")]
        public int Error { get; set; } // TODO: Change to uint32
        [Required]
        [JsonPropertyName("isLogData")]
        public int IsLogData { get; set; }

    }
}
