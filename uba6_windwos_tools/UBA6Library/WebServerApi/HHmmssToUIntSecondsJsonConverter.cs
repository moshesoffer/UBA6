using System;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models
{
    public class HHmmssToUIntSecondsJsonConverter : JsonConverter <uint?>
    {
        public override uint? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            var timeString = reader.GetString();
            if (string.IsNullOrWhiteSpace(timeString))
                return null;           

            if (!TimeSpan.TryParseExact(timeString, @"hh\:mm\:ss", null, out var timeSpan))
                throw new JsonException($"Invalid time format: {timeString}. Expected format is HH:mm:ss.");

            return (uint)timeSpan.TotalSeconds;
        }

        public override void Write(Utf8JsonWriter writer, uint? value, JsonSerializerOptions options)
        {
            var timeSpan = TimeSpan.FromSeconds(value??0);
            var formatted = timeSpan.ToString(@"hh\:mm\:ss");
            writer.WriteStringValue(formatted);
        }
    }
}