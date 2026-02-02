using System.Text.Json;
using System.Text.Json.Serialization;

namespace UBA6Library.WebServerApi {
    public class StringToNullableFloatJsonConverter : JsonConverter<float?> {
        public override float? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options) {
            // Accept both string and number
            if (reader.TokenType == JsonTokenType.String) {
                var str = reader.GetString();
                if (float.TryParse(str, out var value))
                    return value;
                return null;
            }
            if (reader.TokenType == JsonTokenType.Number) {
                return reader.GetSingle();
            }
            if (reader.TokenType == JsonTokenType.Null) {
                return null;
            }
            throw new JsonException("Unexpected token type for float?");
        }

        public override void Write(Utf8JsonWriter writer, float? value, JsonSerializerOptions options) {
            if (value.HasValue)
                writer.WriteNumberValue(value.Value);
            else
                writer.WriteNullValue();
        }
    }
}
