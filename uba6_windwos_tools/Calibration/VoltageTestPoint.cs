using UBA_MSG;
using UBA_PROTO_LINE;

namespace Calibration {
    public partial class Calibration {

        public class VoltageTestPoint : TestPoint {
            public UBA_CALIBRATION_VOLTAGE_TYPE Type;
            public VoltageTestPoint(ID line, int value2set, UBA_CALIBRATION_VOLTAGE_TYPE type) : base(line, value2set) {
                Type = type;
            }
         
           

            public override string ToString() {
                return $"Voltage Test Point: Line:{Line}, Type:{Type}, Value2Set:{Value2Set} mV, ActualValue:{ActualValue} mV, UBA_MeasuredValue:{UBA_MeasuredValue} mV";
            }
        }
    }
}

