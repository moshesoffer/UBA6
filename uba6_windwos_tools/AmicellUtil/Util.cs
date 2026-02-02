using System.IO.Ports;

namespace AmicellUtil {
    public class Util
    {
        public static bool IsComPortExists(string portName) {
            string[] availablePorts = SerialPort.GetPortNames(); // e.g., ["COM1", "COM3", "COM7"]
            return Array.Exists(availablePorts, p => string.Equals(p, portName, StringComparison.OrdinalIgnoreCase));
        }
         public static bool IsWithinPercentage(double value, double target, double percentage) {
            if (percentage < 0)
                throw new ArgumentOutOfRangeException(nameof(percentage), "Percentage must be non-negative.");

            double margin = target * (percentage / 100.0);
            double lowerBound = target - margin;
            double upperBound = target + margin;
            Console.WriteLine($"Value: {value}, Target: {target}, Percentage: {percentage}, Lower Bound: {lowerBound}, Upper Bound: {upperBound}");
            return value >= lowerBound && value <= upperBound;
        }
        public static TEnum BitwiseAnd<TEnum>(TEnum value1, TEnum value2) where TEnum : Enum {
            var underlyingType = Enum.GetUnderlyingType(typeof(TEnum));

            if (underlyingType == typeof(int)) {
                int result = Convert.ToInt32(value1) & Convert.ToInt32(value2);
                return (TEnum)Enum.ToObject(typeof(TEnum), result);
            } else if (underlyingType == typeof(byte)) {
                byte result = (byte)((byte)(object)value1 & (byte)(object)value2);
                return (TEnum)(object)result;
            } else if (underlyingType == typeof(short)) {
                short result = (short)((short)(object)value1 & (short)(object)value2);
                return (TEnum)(object)result;
            } else if (underlyingType == typeof(long)) {
                long result = Convert.ToInt64(value1) & Convert.ToInt64(value2);
                return (TEnum)Enum.ToObject(typeof(TEnum), result);
            } else {
                throw new NotSupportedException($"Unsupported enum underlying type: {underlyingType}");
            }
        }
        public static float RandomFloat(float min = 0, float max=1000) {
            Random random = new Random();
            return (float)(random.NextDouble() * (max - min) + min);
        }

        public static int RandomInt(int min, int max) {
            Random random = new Random();
            return (int)(random.NextDouble() * (max - min) + min);
            
        }
        public class MissingFlagException : Exception {
            public Enum FlagChecked { get; }
            public Enum CurrentValue { get; }

            public MissingFlagException(Enum flagChecked, Enum currentValue)
                : base($"Required flag '{flagChecked}' is not set in current value '{currentValue}'.") {
                FlagChecked = flagChecked;
                CurrentValue = currentValue;
            }
            public MissingFlagException(Enum flagChecked, Enum currentValue, Exception inner) : base($"Required flag '{flagChecked}' is not set in current value '{currentValue}'.", inner) {
                FlagChecked = flagChecked;
                CurrentValue = currentValue;
            }
        }
        public static void EnsureFlagIsSet<TEnum>(TEnum value, TEnum flag) where TEnum : Enum {
            ulong valueLong = Convert.ToUInt64(value);
            ulong flagLong = Convert.ToUInt64(flag);

            if ((valueLong & flagLong) != flagLong) {
                throw new MissingFlagException(flag, value);
            }
        }
    }
}
