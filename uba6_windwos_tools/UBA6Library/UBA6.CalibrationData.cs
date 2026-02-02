namespace UBA6Library {
    public partial class UBA6 {
        public class LineCalibrationData {
            public class LinerEquation {
                public float Slop { get; set; } = 1.0f;
                public float Y_Intercept { get; set; } = 0.0f;
                public LinerEquation() { 
                }
                public LinerEquation(float slop, float y_Intercept) {
                    Slop = slop;
                    Y_Intercept = y_Intercept;
                }

                public LinerEquation(UBA_PROTO_CALIBRATION.liniar_calibration_message msg) :this(msg.Slop, msg.YIntercept) { 
            
                }

                public UBA_PROTO_CALIBRATION.liniar_calibration_message CreateProtoMessage() {
                    UBA_PROTO_CALIBRATION.liniar_calibration_message ret = new UBA_PROTO_CALIBRATION.liniar_calibration_message();
                    ret.Slop = Slop;
                    ret.YIntercept = Y_Intercept;   
                    return ret; 
                }

                public override string ToString() {
                    return $"Slop:{Slop}, Y_Intercept:{Y_Intercept}";

                }


            }

            public LinerEquation[] Vbat = new LinerEquation[3] { new LinerEquation() , new LinerEquation() , new LinerEquation() };
            public LinerEquation Vgen = new LinerEquation();
            public LinerEquation Vps = new LinerEquation();
            public LinerEquation ChargeCurrent = new LinerEquation();
            public LinerEquation DischargeCurrent = new LinerEquation();
            public LinerEquation AmbTemp = new LinerEquation();
            public LinerEquation NtcTemp = new LinerEquation();
            public UInt32 MaxVoltage { get; set; } = 0;
            public UInt32 MaxDischargeCurrent { get; set; } = 0;
            public UInt32 MaxChargeCurrent { get; set; } = 0;            
            public bool IsCalibrated = false;

            public LineCalibrationData() { 
            }
            public LineCalibrationData(UBA_PROTO_CALIBRATION.line_calibration_message msg) {
                Vbat[0] = new LinerEquation(msg.Vbat[0]);
                Vbat[1] = new LinerEquation(msg.Vbat[1]);
                Vbat[2] = new LinerEquation(msg.Vbat[2]);
                Vgen = new LinerEquation(msg.Vgen);
                ChargeCurrent = new LinerEquation(msg.ChargeCurrent);
                DischargeCurrent = new LinerEquation(msg.DischargeCurrent);
                AmbTemp = new LinerEquation(msg.AmbTemp);
                NtcTemp = new LinerEquation(msg.NtcTemp);
                MaxChargeCurrent = msg.MaxChargeCurrent;
                MaxDischargeCurrent = msg.MaxDischargeCurrent;
                MaxVoltage = msg.MaxVoltage;
            }
            public UBA_PROTO_CALIBRATION.line_calibration_message CreateProtoMessage() {
                UBA_PROTO_CALIBRATION.line_calibration_message ret = new UBA_PROTO_CALIBRATION.line_calibration_message();
                ret.Vbat.Add(this.Vbat[0].CreateProtoMessage());
                ret.Vbat.Add(this.Vbat[1].CreateProtoMessage());
                ret.Vbat.Add(this.Vbat[2].CreateProtoMessage());
                ret.Vgen = this.Vgen.CreateProtoMessage();
                ret.Vps = this.Vps.CreateProtoMessage();
                ret.ChargeCurrent = this.ChargeCurrent.CreateProtoMessage();
                ret.DischargeCurrent = this.DischargeCurrent.CreateProtoMessage();
                ret.AmbTemp = this.AmbTemp.CreateProtoMessage();
                ret.NtcTemp = this.NtcTemp.CreateProtoMessage();
                ret.MaxChargeCurrent = this.MaxChargeCurrent;
                ret.MaxDischargeCurrent = this.MaxDischargeCurrent;
                ret.MaxVoltage = this.MaxVoltage;
                return ret;
            }

        }
    }

}
