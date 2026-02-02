using AmicellUtil;
using System.ComponentModel;
using System.Reflection.Metadata;
using System.Text;
using System.Xml;
using static BK_PRECISION9104Libary.BK_PRECISION9104.Command;

namespace BK_PRECISION9104Libary {
    public partial class BK_PRECISION9104 {
        public class Command {     

            public enum PowerSupplyCommand {
                [Description("Set Output on/off")]
                SOUT,

                [Description("Get Output Status")]
                GOUT,

                [Description("Set output Voltage")]
                VOLT,

                [Description("Set output Current")]
                CURR,

                [Description("Set Over Voltage Protection")]
                SOVP,

                [Description("Get Reading Volt & Curr mode")]
                GETD,

                [Description("Set Over Current Protection")]
                SOCP,

                [Description("Get Over Voltage Protection")]
                GOVP,

                [Description("Get Over Current Protection")]
                GOCP,

                [Description("Set preset Voltage and Current")]
                SETD,

                [Description("Get preset Voltage and Current")]
                GETS,

                [Description("Get current preset selection")]
                GABC,

                [Description("Set current preset selection")]
                SABC,

                [Description("Get delta time setting")]
                GDLT,

                [Description("Set delta time setting")]
                SDLT,

                [Description("Get SW time")]
                GSWT,

                [Description("Set SW time")]
                SSWT,

                [Description("Run SW sequence from one preset to another")]
                RUNP,

                [Description("Stop SW running")]
                STOP,

                [Description("Disable Keyboard Input")]
                SESS,

                [Description("Enable Keyboard Input")]
                ENDS,

                [Description("Get all configuration and state values")]
                GALL,

                [Description("Configure Preset1/2/3")]
                SETM
            }
            public PowerSupplyCommand Id;
            public List<string> ParameterList = new List<string>(); 


            public Command(PowerSupplyCommand id) {
                this.Id = id;
            }
            public Command(PowerSupplyCommand id, string param1):this(id) {
                ParameterList.Add(param1);
            }
            public Command(PowerSupplyCommand id, string param1, string param2) : this(id,param1) {
                ParameterList.Add(param2);
            }
            public Command(PowerSupplyCommand id, string param1, string param2, string param3) : this(id, param1, param2) {
                ParameterList.Add(param3);
            }
            public Command(PowerSupplyCommand id, ABC_PRESET mode) : this(id,((int)mode).ToString()) {
                if (mode > ABC_PRESET.NORMAL) {
                    throw new Exception("Preset Mode is OOB");
                }
                ParameterList.Add(((int)mode).ToString("D1"));
            }
            public Command(PowerSupplyCommand id, ABC_PRESET mode, int p1) : this(id, ((int)mode).ToString()) {                
                ParameterList.Add(Mili2Centi(p1).ToString("D4"));
            }
            public Command(PowerSupplyCommand id, ABC_PRESET mode, int p1, int p2) : this(id, mode,p1) {                
                ParameterList.Add(Mili2Centi(p2).ToString("D4"));
            }

            public Command(PowerSupplyCommand id, bool p1) : this(id ) {                
                ParameterList.Add(p1 ? "1" : "0");
            }
            public Command(PowerSupplyCommand id, int p1) : this(id) {                
                ParameterList.Add(Mili2Centi(p1).ToString("D4"));                
            }
            public Command(PowerSupplyCommand id, int p1, int p2) : this(id, p1) {                
                ParameterList.Add(Mili2Centi(p2).ToString("D4"));
            }
            public Command(PowerSupplyCommand id, int p1, int p2, int p3) : this(id, p1, p2) {
                ParameterList.Add(Mili2Centi(p3).ToString("D4"));
            }
            public Command(PowerSupplyCommand id, List<PreSet> presetList):this(id) {
                foreach (PreSet preset in presetList) {
                    ParameterList.Add(preset.ToCommandStr());
                }
            }
            public string Format() {
                StringBuilder sb = new StringBuilder();
                sb.Append(this.Id.ToString().ToUpper());
                foreach (string str in ParameterList) {
                    if (string.IsNullOrWhiteSpace(str) == false) {
                        sb.Append(str);
                    }
                }
                return sb.ToString();
            }

            public override string ToString() {
                return $"Id:{Id} with {ParameterList.Count} Parmaters: {Format()}";
            }

        }

    }
}
