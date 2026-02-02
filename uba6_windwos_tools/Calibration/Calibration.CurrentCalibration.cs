using static UBA6Library.UBA6.LineCalibrationData;

namespace Calibration {
    public partial class Calibration {
        public class CurrentCalibration {
            public static int MAX_DischargeCurrent { get; set; }


            public static Dictionary<UBA_CALIBRATION_CURRENT_TYPE, int> p1Values = new Dictionary<UBA_CALIBRATION_CURRENT_TYPE, int>() {
            { UBA_CALIBRATION_CURRENT_TYPE.CHARGE_CURRENT, 1000 },
            { UBA_CALIBRATION_CURRENT_TYPE.DISCHARGE_CURRENT, MAX_DischargeCurrent/2 }
            };
            public static Dictionary<UBA_CALIBRATION_CURRENT_TYPE, int> p2Values = new Dictionary<UBA_CALIBRATION_CURRENT_TYPE, int>() {
            { UBA_CALIBRATION_CURRENT_TYPE.CHARGE_CURRENT, 3000 },
            { UBA_CALIBRATION_CURRENT_TYPE.DISCHARGE_CURRENT, MAX_DischargeCurrent/2 }
            };
            public CurrentTestPoint P1 { get; set; }
            public CurrentTestPoint P2 { get; set; }
            public UBA_PROTO_LINE.ID LineID { get { return P1.Line; } }
            public UBA_CALIBRATION_CURRENT_TYPE TYPE { get { return P1.Type; } }
            public LinerEquation Equation {
                get {
                    return CalculateLinerEquation(P1.ActualValue, P1.UBA_MeasuredValue, P2.ActualValue, P2.UBA_MeasuredValue);
                }
            }
            public CurrentCalibration(CurrentTestPoint p1, CurrentTestPoint p2) {
                P1 = p1;
                P2 = p2;
            }

            public CurrentCalibration(UBA_PROTO_LINE.ID Line, UBA_CALIBRATION_CURRENT_TYPE type, uint maxCurrent) : this(new CurrentTestPoint(Line, (int)(maxCurrent / 2), type), new CurrentTestPoint(Line, (int)(maxCurrent), type)) {
            }

            public override string ToString() {
                return $"Current Calibration:  {Environment.NewLine}" +$"Point 1 :{P1} {Environment.NewLine}Point 2: {P2} {Environment.NewLine} Equation: {Equation}";
            }
        }


    }
}

