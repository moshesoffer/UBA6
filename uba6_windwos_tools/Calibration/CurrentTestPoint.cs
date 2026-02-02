using UBA_MSG;

namespace Calibration {
    public partial class Calibration {
        public class CurrentTestPoint : TestPoint { 
            public UBA_CALIBRATION_CURRENT_TYPE Type;

            public CurrentTestPoint(UBA_PROTO_LINE.ID line, int value2set, UBA_CALIBRATION_CURRENT_TYPE type) : base(line, value2set) {
                Type = type;
            }


        }
    }
}

