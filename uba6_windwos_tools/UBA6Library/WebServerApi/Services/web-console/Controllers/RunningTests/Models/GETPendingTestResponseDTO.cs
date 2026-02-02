using System.ComponentModel.DataAnnotations;
using System.ComponentModel.Design.Serialization;
using System.Globalization;
using System.Text.Json.Serialization;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models {
    public class GETPendingTestResponseDTO {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("ubaSN")]
        public string UbaSN { get; set; }

        [JsonPropertyName("channel")]
        public string Channel { get; set; }

        [JsonPropertyName("status")]
        public int Status { get; set; }

        [JsonPropertyName("testRoutineChannels")]
        public string TestRoutineChannels { get; set; }

        [JsonPropertyName("machineMac")]
        public string MachineMac { get; set; }

        [JsonPropertyName("noCellSerial")]
        public int NoCellSerial { get; set; }

        [JsonPropertyName("testName")]
        public string TestName { get; set; }

        [JsonPropertyName("plan")]
        public List<PlanStepDTO> Plan { get; set; }
        [JsonPropertyName("reportId")]
        public Guid? ReportId { get; set; }

    }

    public class PlanStepDTO {
        [JsonPropertyName("id")]
        public int Id { get; set; }

        [JsonPropertyName("type")]
        public string Type { get; set; }

        [JsonPropertyName("cRate")]
        public double? CRate { get; set; }

        [JsonPropertyName("source")]
        public string Source { get; set; }

        [JsonPropertyName("maxTemp")]
        [JsonConverter(typeof(StringToNullableFloatJsonConverter))]
        public float? MaxTemp { get; set; }

        [JsonPropertyName("maxTime")]
        [JsonConverter(typeof(HHmmssToUIntSecondsJsonConverter))]
        public uint? MaxTime { get; set; }

        [JsonPropertyName("minTemp")]
        [JsonConverter(typeof(StringToNullableFloatJsonConverter))]
        public float? MinTemp { get; set; }

        [JsonPropertyName("goToStep")]
        public int? GoToStep { get; set; }

        [JsonPropertyName("waitTemp")]
        [JsonConverter(typeof(StringToNullableFloatJsonConverter))]
        public float? WaitTemp { get; set; }
        [JsonConverter(typeof(HHmmssToUIntSecondsJsonConverter))]
        [JsonPropertyName("delayTime")]
        public uint? DelayTime { get; set; }

        [JsonPropertyName("isMaxTemp")]
        public bool IsMaxTemp { get; set; }

        [JsonPropertyName("isMaxTime")]
        public bool IsMaxTime { get; set; }

        [JsonPropertyName("isMinTemp")]
        public bool IsMinTemp { get; set; }

        [JsonPropertyName("repeatStep")]
        public int? RepeatStep { get; set; }

        [JsonPropertyName("chargeLimit")]
        [JsonConverter(typeof(ParseToMiliAmpsHJsonConverter))]
        public int? ChargeLimit { get; set; }

        [JsonPropertyName("isCollapsed")]
        public bool IsCollapsed { get; set; }

        [JsonPropertyName("chargeCurrent")]
        [JsonConverter(typeof(ParseToMiliAmpsJsonConverter))]
        public int? ChargeCurrent { get; set; }

        [JsonPropertyName("chargePerCell")]
        [JsonConverter(typeof(StringToNullableFloatJsonConverter))]
        public float? ChargePerCell { get; set; }

        [JsonPropertyName("cutOffCurrent")]
        [JsonConverter(typeof(ParseToMiliAmpsJsonConverter))]
        public int? CutOffCurrent { get; set; }

        [JsonPropertyName("cutOffVoltage")]
        [JsonConverter(typeof(StringToNullableFloatJsonConverter))]
        public float? CutOffVoltage { get; set; }

        [JsonPropertyName("isChargeLimit")]
        public bool IsChargeLimit { get; set; }

        [JsonPropertyName("dischargeLimit")]
        [JsonConverter(typeof(ParseToMiliAmpsHJsonConverter))]
        public int? DischargeLimit { get; set; }

        [JsonPropertyName("isCutOffCurrent")]
        public bool IsCutOffCurrent { get; set; }

        [JsonPropertyName("isCutOffVoltage")]
        public bool IsCutOffVoltage { get; set; }

        [JsonPropertyName("dischargeCurrent")]        
        public string? DischargeCurrent { get; set; }


        [JsonIgnore]
        public double? DischargeValue => ParseValue();

        [JsonIgnore]
        public UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE? DischargeType => ParseType();

        private double? ParseValue() {
            if (string.IsNullOrWhiteSpace(DischargeCurrent))
                return null;

            var parts = DischargeCurrent.Split(':', 2);

            double? ret =  double.TryParse(parts[0],
                NumberStyles.Any,
                CultureInfo.InvariantCulture,
                out var v) ? v : null;
            switch (DischargeType) { 
                case UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute:
                    return ret;
                case UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Power: // in Watts AH
                    return ret ; 
                case UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Resistance: // in Ohms
                    return ret ; 
                default:
                    return null;
            }                
        }

        private UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE? ParseType() {
            if (string.IsNullOrWhiteSpace(DischargeCurrent))
                return null;

            var parts = DischargeCurrent.Split(':', 2);
            if (parts.Length != 2)
                return null;

            return parts[1].ToLowerInvariant() switch {
                "absolutema" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute,
                "absolutea" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute,
                "power" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Power,
                "resistance" => UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Resistance,
                _ => null
            };
        }
        

        [JsonPropertyName("isDischargeLimit")]
        public bool IsDischargeLimit { get; set; }

        


    }

    public class util {


        public static UBA_PROTO_CHANNEL.ID GetChannelFormDTO(GETPendingTestResponseDTO dto) {
            if (dto.TestRoutineChannels.Equals("A-and-B")) {
                return UBA_PROTO_CHANNEL.ID.Ab;
            } else if (dto.Channel.Equals("A")) {
                return UBA_PROTO_CHANNEL.ID.A;
            } else {
                return UBA_PROTO_CHANNEL.ID.B;
            }
        }
        public static uint GetIndexFormDTO(GETPendingTestResponseDTO dto) {
            if (dto.TestRoutineChannels.Equals("A-and-B")) {
                return 9;
            } else if (dto.Channel.Equals("A")) {
                return 8;
            } else {
                return 9;
            }
        }

        public static int ParseToMiliAmps(string input) {
            if (string.IsNullOrWhiteSpace(input))
                throw new ArgumentException("Input is null or empty", nameof(input));

            var parts = input.Split(':');
            if (parts.Length != 2)
                throw new FormatException($"Invalid format: {input}");

            if (!double.TryParse(parts[0], out double value))
                throw new FormatException($"Invalid numeric value: {parts[0]}");

            string unit = parts[1].ToLowerInvariant();

            return unit switch {
                "absolutea" => (int)(value * 1_000), // A to mA
                "absolutema" => (int)(value),    // in mA 
                _ => throw new NotSupportedException($"Unsupported unit: {unit}")
            };
        }


        public static int ParseToMiliAmpsH(string input) {
            if (string.IsNullOrWhiteSpace(input))
                throw new ArgumentException("Input is null or empty", nameof(input));

            var parts = input.Split(':');
            if (parts.Length != 2)
                throw new FormatException($"Invalid format: {input}");

            if (!double.TryParse(parts[0], out double value))
                throw new FormatException($"Invalid numeric value: {parts[0]}");

            string unit = parts[1].ToLowerInvariant();

            return unit switch {
                "absoluteah" => (int)(value * 1_000), // Ah to mAh
                "absolutemah" => (int)(value),    // mAh
                _ => throw new NotSupportedException($"Unsupported unit: {unit}")
            };
        }
        static float ConvertTemp(string tempstr) {
            if (float.TryParse(tempstr, out float temp)) {
                return temp;
            } else {
                throw new ArgumentException("Invalid MaxTemp value");
            }
        }
        static float ConvertTemp(float? tempstr) {
            if (!tempstr.HasValue) {
                throw new ArgumentNullException(nameof(tempstr), "Input float? value is null");
            } else { 
                return tempstr.Value;
            }
        }
        public static uint ConvertTimeToSeconds(string time) {
            if (TimeSpan.TryParse(time, out TimeSpan result)) {
                return (uint)result.TotalSeconds;
            }
            throw new FormatException($"Invalid time format: {time}");
        }
        public static int Convert2mV(float? input, int numberofcell) {
            if (!input.HasValue)
                throw new ArgumentNullException(nameof(input), "Input float? value is null");
            return (int)(input.Value * 1000 * numberofcell);
        }


     
        protected static UBA_PROTO_BPT.charge_stop_condition plan2ChargeSC(PlanStepDTO ps) {
            if (ps.Type != "charge") {
                throw new Exception("Invalid step type for charge stop condition conversion");
            }            

            float maxTemp = ProtoHelper.DEFAULT_MAX_TEMP;
            uint maxTime = ProtoHelper.DEFAULT_MAX_TIME;
            int CutoffCurrent = ProtoHelper.DEFAULT_CHARGE_CUTOFF_CURRENT;
            int LimitCapacity = ProtoHelper.DEFAULT_LIMIT_CAPACITY;

            if (ps.IsMaxTemp) {
                maxTemp = ConvertTemp(ps.MaxTemp);
            }
            if(ps.IsMaxTime && ps.MaxTime != null) {
                maxTime = ps.MaxTime.Value;
            }
            if (ps.IsCutOffCurrent){
                CutoffCurrent = ps.CutOffCurrent ?? CutoffCurrent;
            }
            if (ps.IsChargeLimit) {
                LimitCapacity = ps.ChargeLimit?? LimitCapacity;
            }
            UBA_PROTO_BPT.charge_stop_condition msg = ProtoHelper.CreateChargeStopCondtion(
                maxTemp:maxTemp
                ,maxTime: maxTime,
                cutOffCurrent: CutoffCurrent,
                limitCapacity: LimitCapacity
                );

            return msg;
        }
           protected static UBA_PROTO_BPT.discharge_stop_condition plan2DischargeSC(PlanStepDTO ps, int numberOfCell) {
            if (ps.Type != "discharge") {
                throw new Exception("Invalid step type for discharge stop condition conversion");
            }
            float MaxTemp = ProtoHelper.DEFAULT_MAX_TIME;
            uint MaxTime = ProtoHelper.DEFAULT_MAX_TIME;
            int CutoffVoltage = ProtoHelper.DEFAULT_DISCHARGE_CUTOFF_VOLTAGE;
            int LimitCapacity = ProtoHelper.DEFAULT_LIMIT_CAPACITY;
            if (ps.IsMaxTemp) {
                MaxTemp = ps.MaxTemp ?? -273.15f;
            }
            if(ps.IsMaxTime && ps.MaxTime != null) {
                MaxTime = ps.MaxTime.Value;
            }
            if (ps.IsCutOffVoltage) {
                CutoffVoltage = (int)(((ps.CutOffVoltage ?? 0) * numberOfCell) *1000);
            }
            if (ps.IsDischargeLimit && ps.DischargeLimit != null) {
                LimitCapacity = ps.DischargeLimit.Value;
            }
            UBA_PROTO_BPT.discharge_stop_condition msg = ProtoHelper.CreateDischargeStopCondition(
                maxTemp:MaxTemp,maxTime:MaxTime,cutOfVoltage:CutoffVoltage,limitCapacity:LimitCapacity);
           
            return msg;
        }

        protected static UBA_PROTO_BPT.charge plan2ChargeStep(PlanStepDTO ps, int numberOfCell) {
            if (ps.Type != "charge") {
                throw new Exception("Invalid step type for charge conversion");
            }
            UBA_PROTO_BPT.charge msg = ProtoHelper.CreateChargeStep(
                UBA_PROTO_BPT.SOURCE.Internal,
                ps.ChargeCurrent ?? 0,
                (int)((ps.ChargePerCell ?? 0) * numberOfCell*1000.0f),
                plan2ChargeSC(ps),
                ps.MinTemp ?? - 273.15f );
            return msg;
        }
        protected static UBA_PROTO_BPT.discharge plan2DischargeStep(PlanStepDTO ps, int numberOfCell) {
            if (ps.Type != "discharge") {
                throw new Exception("Invalid step type for discharge conversion");
            }
            return ProtoHelper.CreateDischargeStep(UBA_PROTO_BPT.SOURCE.Internal,
                (int)(ps.DischargeValue ?? 1 ) ,
                ps.DischargeType?? UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE.Absolute,
                plan2DischargeSC(ps, numberOfCell),
                ps.MinTemp ?? -273.15f);
        }
        protected static UBA_PROTO_BPT.delay plan2DelayStep(PlanStepDTO ps) {
            UBA_PROTO_BPT.delay msg = new UBA_PROTO_BPT.delay();
            if (ps.Type != "delay") {
                throw new Exception("Invalid step type for delay");
            }
            msg.CoolDownTemperature =  ps.WaitTemp ?? float.MaxValue ;
            msg.DelayTime = ps.DelayTime ?? uint.MaxValue ;
            return msg;
        }

        protected static UBA_PROTO_TR.Loop plan2Loop(PlanStepDTO ps) {
            UBA_PROTO_TR.Loop loop = new UBA_PROTO_TR.Loop();
            if (ps.Type != "loop") {
                throw new Exception("Invalid step type for loop conversion");
            }
            if (!ps.GoToStep.HasValue)
                throw new ArgumentException("GoToStep must have a value for loop step");
            if (!ps.RepeatStep.HasValue)
                throw new ArgumentException("RepeatStep must have a value for loop step");
            loop.LoopToStep = (uint)ps.GoToStep.Value;
            loop.LoopCounter = (uint)ps.RepeatStep.Value;
            return loop;
        }
        public static UBA_PROTO_TR.config_step planStepDTO2Message(PlanStepDTO ps,int numberOfCell) {
            UBA_PROTO_TR.config_step msg = new UBA_PROTO_TR.config_step();
            msg.TypeId = ps.Type switch {
                "charge" => UBA_PROTO_TR.STEP_TYPE.Charge,
                "discharge" => UBA_PROTO_TR.STEP_TYPE.Discharge,
                "delay" => UBA_PROTO_TR.STEP_TYPE.Delay,
                "loop" => UBA_PROTO_TR.STEP_TYPE.Loop,
                _ => throw new ArgumentException("Invalid step type")
            };
            if (msg.TypeId == UBA_PROTO_TR.STEP_TYPE.Charge) {
                msg.Charge = plan2ChargeStep(ps, numberOfCell);
            } else if (msg.TypeId == UBA_PROTO_TR.STEP_TYPE.Discharge) {
                msg.Discharge = plan2DischargeStep(ps, numberOfCell);
            } else if (msg.TypeId == UBA_PROTO_TR.STEP_TYPE.Delay) {
                msg.Delay = plan2DelayStep(ps);
            } else if (msg.TypeId == UBA_PROTO_TR.STEP_TYPE.Loop) {
                msg.Loop = plan2Loop(ps);
            } else { 
                throw new ArgumentException("Invalid step type for planStepDTO2Message conversion");
            }
                return msg;
        }

        public static UBA_PROTO_TR.Test_Routine GETPendingTestResponseDTO2TR_Message(GETPendingTestResponseDTO pn) {
            UBA_PROTO_TR.Test_Routine msg = new UBA_PROTO_TR.Test_Routine();
            msg.Mode = pn.TestRoutineChannels switch {
                "A-or-B" => UBA_PROTO_BPT.MODE.SingleChannel,
                "A-and-B" => UBA_PROTO_BPT.MODE.DualChannel,
              _ => throw new ArgumentException("Invalid test routine channels")
            };
            msg.Name = pn.TestName.Substring(0, pn.TestName.Length > 10 ? 10: pn.TestName.Length-1);
            msg.Length = (uint)pn.Plan.Count;
            msg.LogInterval = ProtoHelper.DEFAULT_TR_LOG_INTRVAL_MS;
            UBA_PROTO_TR.config_step step = new UBA_PROTO_TR.config_step();
            for (int i = 0; i < pn.Plan.Count; i++) {
                PlanStepDTO ps = pn.Plan[i];
                step = planStepDTO2Message(ps,  pn.NoCellSerial);
                msg.Config.Add(step);
            }
            step = new UBA_PROTO_TR.config_step();
            for (int i = pn.Plan.Count; i <10; i++) {
                msg.Config.Add(step);
            }
             return msg;
        }

    }
}
