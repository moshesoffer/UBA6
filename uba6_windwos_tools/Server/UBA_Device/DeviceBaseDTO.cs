using System.Text.Json.Serialization;

namespace Server.UBA_Device {
    public class DeviceBaseDTO {
        [JsonPropertyName("machine")]
        public string Machine { get; set; }
        [JsonPropertyName("port")]
        public string Port { get; set; }
        [JsonPropertyName("address")]
        public string Address { get; set; }

        public override bool Equals(object? obj) {
            if (obj == null || GetType() != obj.GetType())
                return false;
            DeviceBaseDTO other = obj as DeviceBaseDTO;
            return Machine.Equals(other.Machine) && Port.Equals(other.Port);
        }

        public override int GetHashCode() {
            return HashCode.Combine(Machine, Port, Address);
        }

        public static bool operator == (DeviceBaseDTO left, DeviceBaseDTO right) {
            if (ReferenceEquals(left, right))
                return true;

            if (left is null || right is null)
                return false;

            return left.Equals(right);
        }

        // Overload the != operator
        public static bool operator !=(DeviceBaseDTO left, DeviceBaseDTO right) {
            return !(left == right);
        }
    }
}
