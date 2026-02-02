namespace BK_PRECISION9104Libary {
    public partial class BK_PRECISION9104 {
        public class PreSet {

            public int Voltage = 0; // in mV
            public int Current = 0; // in mA
            public int SwTime = 0; // in sec
            
            public PreSet() {

            }
            public PreSet(int voltage) {
                this.Voltage = voltage;
            }
            public PreSet(int voltage, int current) : this(voltage) {
                this.Current = current;
            }
            public PreSet(int voltage, int current, int swTime) : this(voltage, current) {
                this.SwTime = swTime;
            }            
            /// <summary>
            /// return command string that represent the preset valeus 
            /// </summary>
            /// <param name="full"></param>
            /// <returns></returns>
            public string ToCommandStr(bool full = false) {
                if (full) { 
                    return Mili2Centi(Voltage).ToString("D4") + Mili2Centi(Current).ToString("D4") + SwTime.ToString("D3");
                }
                return Mili2Centi(Voltage).ToString("D4") + Mili2Centi(Current).ToString("D4");
            }
        }
    }
}
