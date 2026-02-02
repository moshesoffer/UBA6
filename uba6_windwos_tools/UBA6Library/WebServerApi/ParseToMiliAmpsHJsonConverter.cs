using System;
using System.Globalization;
using System.Reflection.Metadata.Ecma335;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models
{
    public class ParseToMiliAmpsHJsonConverter : JsonConverter<int?>
    {
        public override int? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {

            var input = reader.GetString();

            if (string.IsNullOrWhiteSpace(input)) {
                Exception e = new JsonException("Input is null or empty");
            }

            var parts = input.Split(':');
            if (parts.Length != 2)
                throw new JsonException($"Invalid format: {input}");

            if (!double.TryParse(parts[0], out double value))
                return null;

            string unit = parts[1].ToLowerInvariant();

            switch (unit) {
                case "absoluteah":
                    return (int)(value * 1_000); // Ah to mAh
                case "absolutemah":
                    return (int)(value);    // mAh
                default:
                    Console.WriteLine($"Unsupported unit: {unit}");
                    return 0;
                    // Or: throw new JsonException($"Unsupported unit: {unit}");
            }
        }

        public override void Write(Utf8JsonWriter writer, int? value, JsonSerializerOptions options)
        {
            // Write as mAh string (e.g., "1000:absolutemAh")
            writer.WriteStringValue($"{value}:absolutemAh");
        }
    }

    public class ParseToMiliAmpsJsonConverter : JsonConverter<int?> {
        public override int? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options) {
            var input = reader.GetString();

            if (string.IsNullOrWhiteSpace(input)) {
                Exception e = new JsonException("Input is null or empty");
            }

            var parts = input.Split(':');
            if (parts.Length != 2)
                throw new JsonException($"Invalid format: {input}");

            if (!double.TryParse(parts[0], out double value))
                return null;

            string unit = parts[1].ToLowerInvariant();

            switch (unit) {
                case "absolutea":
                    return (int)(value * 1_000); // Ah to mAh
                case "absolutema":
                    return (int)(value);    // mAh
                case "power":

                default:
                    Console.WriteLine($"Unsupported unit: {unit}");
                    return 0;
                    // Or: throw new JsonException($"Unsupported unit: {unit}");
            }          
        }

        public override void Write(Utf8JsonWriter writer, int? value, JsonSerializerOptions options) {
            // Write as mAh string (e.g., "1000:absolutemA")
            writer.WriteStringValue($"{value}:absolutemA");
        }
    }


    public sealed class ParseToDischargeCurrentType
    : JsonConverter<UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE?> {
        public override UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE? Read(
            ref Utf8JsonReader reader,
            Type typeToConvert,
            JsonSerializerOptions options) {
            // Handle JSON null
            if (reader.TokenType == JsonTokenType.Null)
                return null;

            if (reader.TokenType != JsonTokenType.String)
                throw new JsonException($"Expected string, got {reader.TokenType}");

            var input = reader.GetString();

            if (string.IsNullOrWhiteSpace(input))
                throw new JsonException("Input is null or empty");

            var parts = input.Split(':', 2);
            if (parts.Length != 2)
                throw new JsonException($"Invalid format: '{input}'");

            // Value part is currently unused, but validated
            if (!double.TryParse(parts[0], NumberStyles.Any, CultureInfo.InvariantCulture, out _))
                throw new JsonException($"Invalid numeric value: '{parts[0]}'");

            var unit = parts[1].Trim().ToLowerInvariant();

            return unit switch {
                "absolutea" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute,
                "absolutema" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute,
                "power" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Power,
                "resistance" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Resistance,
                _ => throw new JsonException($"Unsupported unit: '{unit}'")
            };
        }

        public override void Write(
            Utf8JsonWriter writer,
            UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE? value,
            JsonSerializerOptions options) {
            if (value is null) {
                writer.WriteNullValue();
                return;
            }

            // Write canonical string format
            var unit = value.Value switch {
                UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute => "absolutemA",
                UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Power => "power",
                UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Resistance => "resistance",
                _ => throw new JsonException($"Unsupported value: {value}")
            };

            writer.WriteStringValue($"0:{unit}");
        }
    }/*


    public class ParseToDischargeCurrentType : JsonConverter<UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE?> {
        public override UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options) {
            var input = reader.GetString();

            if (string.IsNullOrWhiteSpace(input)) {
                Exception e = new JsonException("Input is null or empty");
            }

            var parts = input.Split(':');
            if (parts.Length != 2)
                throw new JsonException($"Invalid format: {input}");

            if (!double.TryParse(parts[0], out double value))
                return null;

            string unit = parts[1].ToLowerInvariant();

            switch (unit) {
                case "absolutea":
                    return UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute;
                case "absolutema":
                    return UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute;                    
                case "power":
                    return UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Power;
                case "resistance":
                    return UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Resistance;
                default:
                    Console.WriteLine($"Unsupported unit: {unit}");
                    return 0;
                    // Or: throw new JsonException($"Unsupported unit: {unit}");
            }
        }

        public override void Write(Utf8JsonWriter writer, UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE? value, JsonSerializerOptions options) {
            // Write as mAh string (e.g., "1000:absolutemA")
            writer.WriteStringValue($"{value}:absolutemA");
        }
    }
    */
}