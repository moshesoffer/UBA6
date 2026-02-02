using UBA_MSG;
using UBA_PROTO_LINE;

namespace Calibration {
    public partial class Calibration {
        public class TempTestPoint : TestPoint {
            public UBA_CALIBRATION_TEMP_TYPE Type;
            public TempTestPoint(ID line, float value2set, UBA_CALIBRATION_TEMP_TYPE type) : base(line, value2set) {
                Type = type;
                this.ActualValue = value2set; // Assuming ActualValue is the same as Value2Set for temperature
            }
            public float UpdatedFromUBA_Messsage(UBA_PROTO_LINE.status? m) {
                if (m == null) {
                    throw new ArgumentException("Message LineStatus is null");
                }
                if (Type.HasFlag(UBA_CALIBRATION_TEMP_TYPE.AMBIANT_TEMP)) {                    
                    if (m.Data == null) {
                        throw new ArgumentException("Message LineStatus Data is null");
                    }
                    this.UBA_MeasuredValue = m.Data.AmbTemperature;
                } else if (Type.HasFlag(UBA_CALIBRATION_TEMP_TYPE.BATTERY_TEMP)) {                    
                    this.UBA_MeasuredValue = m.Data.BatTemperature;
                } else {
                    throw new ArgumentException($"Unsupported UBA_CALIBRATION_TEMP_TYPE: {Type}");
                }
                return this.UBA_MeasuredValue;
            }
            public override string ToString() {
                return $"Temperature Test Point: Line:{Line}, Type:{Type}, Value2Set:{Value2Set} °C, ActualValue:{ActualValue} °C, UBA_MeasuredValue:{UBA_MeasuredValue} °C";
            }
        }
    }
}

