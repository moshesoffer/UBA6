using UBA_PROTO_BPT;
using static UBA6Library.UBA6.LineCalibrationData;

namespace Calibration {
    public partial class Calibration {

        public class VoltageCalibration {
            public static Dictionary<UBA_CALIBRATION_VOLTAGE_TYPE, int> p1Values = new Dictionary<UBA_CALIBRATION_VOLTAGE_TYPE, int>() {
            { UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V, 3000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V, 12000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE60V,  31000},
            { UBA_CALIBRATION_VOLTAGE_TYPE.VPS, 12000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.GEN_VOLTAGE, 12000 }
        };
            public static Dictionary<UBA_CALIBRATION_VOLTAGE_TYPE, int> p2Values = new Dictionary<UBA_CALIBRATION_VOLTAGE_TYPE, int>() {
            { UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V, 9000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V, 29000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE60V, 47000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.VPS, 24000 },
            { UBA_CALIBRATION_VOLTAGE_TYPE.GEN_VOLTAGE, 47000 }
        };
            public VoltageTestPoint P1 { get; set; }
            public VoltageTestPoint P2 { get; set; }
            public UBA_PROTO_LINE.ID LineID { get { return P1.Line; } }
            public UBA_CALIBRATION_VOLTAGE_TYPE TYPE { get { return P1.Type; } }

            public LinerEquation Equation { 
                get {                    
                    return CalculateLinerEquation(P1.ActualValue,P1.UBA_MeasuredValue, P2.ActualValue, P2.UBA_MeasuredValue);                 } 
            }

            public VoltageCalibration(VoltageTestPoint p1, VoltageTestPoint p2) {
                P1 = p1;
                P2 = p2;
            }
            public VoltageCalibration(UBA_PROTO_LINE.ID Line, UBA_CALIBRATION_VOLTAGE_TYPE type ) : this(new VoltageTestPoint(Line, p1Values[type], type), new VoltageTestPoint(Line, p2Values[type], type)) {
           

            }

            public override string ToString() {
                return $"Voltage Calibration:  {Environment.NewLine}Point 1 :{P1} {Environment.NewLine}Point 2: {P2} {Environment.NewLine} Equation: {Equation}";

            }
        }


    }
}

