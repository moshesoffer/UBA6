using static UBA6Library.UBA6.LineCalibrationData;

namespace Calibration {
    public partial class Calibration {
        public class TempCalibration {
            public static Dictionary<UBA_CALIBRATION_TEMP_TYPE, float> p1Values = new Dictionary<UBA_CALIBRATION_TEMP_TYPE, float>() {
            { UBA_CALIBRATION_TEMP_TYPE.AMBIANT_TEMP, 25.0f },
            { UBA_CALIBRATION_TEMP_TYPE.BATTERY_TEMP, 25.0f },
            
        };
            public TempTestPoint P1 { get; set; }
            public UBA_PROTO_LINE.ID LineID { get { return P1.Line; } }
            public UBA_CALIBRATION_TEMP_TYPE Type { get { return P1.Type; } }   
            public TempCalibration(TempTestPoint p1) { 
                P1 = p1;
            }
            public TempCalibration(UBA_PROTO_LINE.ID Line, UBA_CALIBRATION_TEMP_TYPE type) : this(new TempTestPoint(Line, (int)p1Values[type], type)) {
            }
            public LinerEquation Equation {
                get {
                    return CalculateLinerEquation(P1.ActualValue, P1.UBA_MeasuredValue);
                }
            }
        }


    }
}

