namespace Calibration {
    public partial class Calibration {
        [Flags]
        public enum UBA_CALIBRATION_TEMP_TYPE {
            NONE = 0x00,
            AMBIANT_TEMP = 0x01,
            BATTERY_TEMP = 0x02,
        }

        [Flags]
        public enum UBA_CALIBRATION_VOLTAGE_TYPE {
            NONE = 0x00,
            BATTERY_RANGE10V = 0x01,
            BATTERY_RANGE30V = 0x02,
            BATTERY_RANGE60V = 0x04,
            // BATTERY = BATTERY_RANGE10V | BATTERY_RANGE30V | BATTERY_RANGE60V,
            VPS = 0x08,
            GEN_VOLTAGE = 0x10,
        }
        [Flags]
        public enum UBA_CALIBRATION_CURRENT_TYPE {
            NONE = 0x00,
            CHARGE_CURRENT = 0x01,
            DISCHARGE_CURRENT = 0x02,
        }

    }

}

