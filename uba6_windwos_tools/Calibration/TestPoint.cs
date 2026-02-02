namespace Calibration {
    public partial class Calibration {
        public class TestPoint : IComparable<TestPoint> {
            public UBA_PROTO_LINE.ID Line { get; set; } // the line that should be tested, e.g. A or B
            public float Value2Set { get; set; } // the value that a device should set, e.g. PowerSupply or Load cell 
            public float UBA_MeasuredValue { get; set; } // the value that UBA should measure
            public float ActualValue { get; set; } // the value that a precied device should measure, e.g. MultiMeter

            public TestPoint(UBA_PROTO_LINE.ID line, float value2set) {
                Line = line;
                Value2Set = value2set;
            }

            public TestPoint(UBA_PROTO_LINE.ID line, int value2set) : this(line,(float) value2set) {
                
            }
            public bool IsAtTestPoint(int pointValue) {
                return Value2Set == pointValue;
            }
            public void SetPoint(float ubaMeasuredValue, float actualValue) {
                UBA_MeasuredValue = ubaMeasuredValue;
                ActualValue = actualValue;
            }
            // Implement IComparable<TestPoint>
            public int CompareTo(TestPoint other) {
                if (other == null) return 1;
                return Value2Set.CompareTo(other.Value2Set);
            }


            public override string ToString() {
                return $"Line: {Line}, Value2set: {Value2Set}, UBA_MeasuredValue: {UBA_MeasuredValue}, ActualValue: {ActualValue}";
            }
        }


    }
}

