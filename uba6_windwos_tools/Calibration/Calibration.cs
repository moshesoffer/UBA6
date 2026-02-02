using AmicellUtil;
using BK_PRECISION9104Libary;
using KeithleyDMM6500Library;
using Microsoft.AspNetCore.DataProtection.KeyManagement;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using System.Drawing;
using UBA_MSG;
using UBA_PROTO_BPT;
using UBA_PROTO_QUERY;
using UBA6Library;
using static Calibration.Calibration;
using static UBA6Library.UBA6.LineCalibrationData;

namespace Calibration {
    public partial class Calibration : IStatus{
        private readonly ILogger<Calibration> _logger;
        public UBA6 UBA;
        public BK_PRECISION9104 PowerSupply;
        public KeithleyDMM6500 MultiMeter;
        public KelDeviceController LoadCell;
        public TimeSpan PS_Delay { get; set; } = TimeSpan.Zero;
        public uint MaxCurrent { get; set; } = 3000; // in mA, default value for UBA6   
        public uint MaxDischargeCurrent { get; set; } = 3000; // in mA, default value for UBA6   
        public uint MaxChargeCurrent { get; set; } = 3000; // in mA, default value for UBA6   
        public uint MaxVoltage { get; set; } = 60000; // in mA, default value for UBA6   

        public bool IsInEmulationMode { get {
                return PowerSupply.IsInEmulationMode | MultiMeter.IsInEmulationMode | LoadCell.IsInEmulationMode;
            } set {
                PowerSupply.IsInEmulationMode = value;
                MultiMeter.IsInEmulationMode = value;
                LoadCell.IsInEmulationMode = value;
            } 
        } 
        public TimeSpan MesuremntDelay { get; set; } = TimeSpan.FromSeconds(2);
        public uint AvgCount = 5;

        public event EventHandler<StatusEventArg> StatusChanged;
        public event EventHandler<ExceptionEventArg> ExceptionOccurred;
        public Calibration() {
        }

        public Calibration(ILogger<Calibration> logger) {
            _logger = logger;
            _logger.LogInformation(_logger != null ? "Calibration initialized with logger." : "Calibration initialized without logger.");
        }
        public Calibration(ILogger<Calibration> log,UBA6 uba) :this(log){
            UBA = uba;
            _logger.LogInformation("Calibration initialized with UBA6.");
        }
        public Calibration(ILogger<Calibration> log,UBA6 uba, BK_PRECISION9104 ps) : this(log,uba) {
            PowerSupply = ps;
            _logger.LogInformation("Calibration initialized with Power Supply BK_PRECISION9104.");

        }
        public Calibration(ILogger<Calibration> log,UBA6 uba, BK_PRECISION9104 ps, KeithleyDMM6500 mm) : this(log, uba, ps) {
            MultiMeter = mm;
            _logger.LogInformation("Calibration initialized with MultiMeter KeithleyDMM6500.");

        }
        public Calibration(ILogger<Calibration> log,UBA6 uba, BK_PRECISION9104 ps, KeithleyDMM6500 mm, KelDeviceController lc) : this(log, uba, ps, mm) {
            LoadCell = lc;
            _logger.LogInformation("Calibration initialized with Load Cell KelDeviceController.");
        }        
        public Calibration(ILogger<Calibration> log, UBA6 uba, BK_PRECISION9104 ps, KeithleyDMM6500 mm, KelDeviceController lc, TimeSpan delay, uint repeat) : this(log,uba, ps, mm, lc) {
            MesuremntDelay = delay;
            AvgCount = repeat;
            _logger.LogInformation("Calibration initialized with delay: {Delay} [Sec] and repeat count: {RepeatCount}", delay.TotalSeconds, repeat);
        }
     
        public static LinerEquation RandomLinerEquation() {
            Random random = new Random();
            float[] numbers = new float[2];
            return new LinerEquation((float)(random.NextDouble()), (float)(random.NextDouble()));
        }
        public static LinerEquation CalculateLinerEquation(float actualPoint1, float measuredPoint1, float actualPoint2, float measuredPoint2) {
            Console.WriteLine($"Calculate Liner Equation: MultiMeter:{actualPoint1},{actualPoint2} Measure :{measuredPoint1},{measuredPoint2}");
            if (actualPoint1 == actualPoint2) {
                throw new ArgumentException($"Cannot calculate slope: measured Point 1: {measuredPoint1} and measured Point 2: {measuredPoint2} cannot be equal (vertical line).");
            }
            float Slope = (actualPoint2 - actualPoint1) / (measuredPoint2 - measuredPoint1);
            float Intercept = actualPoint1 - Slope * measuredPoint1;
            return new LinerEquation(Slope, Intercept);
        }
        public static LinerEquation CalculateLinerEquation(float actualPoint1, float measuredPoint1) {
            return new LinerEquation(1, actualPoint1 - measuredPoint1);
        }
        public static float AddRandom10Percent(float value) {
            var rand = new Random();
            // Generate random multiplier between 0.9 and 1.1
            float factor = 0.9f + (float)rand.NextDouble() * 0.2f;
            return value * factor;
        }

        public async Task Init() {
            await PowerSupply.SetOverVoltage(65000);
            await PowerSupply.SetOverCurrent(10000); //10000mA 10A
            await PowerSupply.SetABC_Select(BK_PRECISION9104Libary.BK_PRECISION9104.ABC_PRESET.NORMAL);
            await PowerSupply.SetOutputVoltage(BK_PRECISION9104.ABC_PRESET.NORMAL, 12000);
            await PowerSupply.SetOutputCurrnt(BK_PRECISION9104.ABC_PRESET.NORMAL, 2000);
            await PowerSupply.SetPresetVoltageAndCuurent(BK_PRECISION9104.ABC_PRESET.NORMAL, 10000, 2000);
            
        }

        private async Task voltagePointMesure(VoltageTestPoint vp, float mm_value) {
            _logger.LogDebug("Before Mesure : Voltage Point Measure: {vp}", vp);
            UBA6.MeasurementType type = vp.Line == UBA_PROTO_LINE.ID.A ? UBA6.MeasurementType.LineA : UBA6.MeasurementType.LineB;
            if (vp.Type.HasFlag(UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V) ||
                           vp.Type.HasFlag(UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V) ||
                           vp.Type.HasFlag(UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE60V)) {
                type |= UBA6.MeasurementType.BAT_Voltage;
            } else if (vp.Type.HasFlag(UBA_CALIBRATION_VOLTAGE_TYPE.GEN_VOLTAGE)) {
                type |= UBA6.MeasurementType.GEN_Voltage;
            } else if (vp.Type.HasFlag(UBA_CALIBRATION_VOLTAGE_TYPE.VPS)) {
                type |= UBA6.MeasurementType.VPS;
            } else {
                throw new ArgumentException($"Unsupported UBA_CALIBRATION_VOLTAGE_TYPE: {vp.Type}");
            }
            vp.UBA_MeasuredValue = await UBA.Mesure(type, this.MesuremntDelay,this.AvgCount);
            vp.ActualValue = mm_value;
            _logger.LogInformation("Voltage Point Measure: {vp}", vp);
        }
        private async Task voltagePointMesure(VoltageTestPoint vp ,bool PS_shoudown = false) {
            await PowerSupply.SetOutputVoltage(BK_PRECISION9104Libary.BK_PRECISION9104.ABC_PRESET.NORMAL,(int)vp.Value2Set, true);
            _logger.LogInformation($"waiting... for {PS_Delay.TotalMilliseconds} ms");
            await Task.Delay(PS_Delay);
            await voltagePointMesure(vp, await MultiMeter.Mesure(KeithleyDMM6500.MeasurementType.Voltage, this.MesuremntDelay, this.AvgCount));
            if (PS_shoudown) { 
                await PowerSupply.SetOutput(false); // turn off the power supply
            }
        }

        private async Task currentPointMesure(CurrentTestPoint ctp, float mm_value) {
            _logger.LogDebug("Before Mesure : Current Point Measure: {ctp}", ctp);
            UBA6.MeasurementType type = ctp.Line == UBA_PROTO_LINE.ID.A ? UBA6.MeasurementType.LineA : UBA6.MeasurementType.LineB;
            if (ctp.Type.HasFlag(UBA_CALIBRATION_CURRENT_TYPE.CHARGE_CURRENT)) {
                type |= UBA6.MeasurementType.ChargeCurrent;
            } else if (ctp.Type.HasFlag(UBA_CALIBRATION_CURRENT_TYPE.DISCHARGE_CURRENT)) {
                type |= UBA6.MeasurementType.DischageCurrent;
            } else {
                throw new ArgumentException($"Unsupported UBA_CALIBRATION_CURRENT_TYPE: {ctp.Type}");
            }
            ctp.UBA_MeasuredValue = await UBA.Mesure(type, this.MesuremntDelay, this.AvgCount);
            ctp.ActualValue = mm_value;
            _logger.LogInformation("Current Point Measure: {ctp}", ctp);
        }

        private async Task currentPointMesure(CurrentTestPoint ctp) {
            LoadCell.SetCurrent(ctp.Value2Set );
            await PowerSupply.SetOutputCurrnt(BK_PRECISION9104Libary.BK_PRECISION9104.ABC_PRESET.NORMAL, (int)(ctp.Value2Set + 500));
            await PowerSupply.SetOutputVoltage(BK_PRECISION9104Libary.BK_PRECISION9104.ABC_PRESET.NORMAL, 5000, true); // Set Power Supply to 12V for current calibration
            LoadCell.SetFunctionMode(KelDeviceController.FunctionMode.CC); // Set Load Cell to CC mode (Constant Voltage Mode) for current calibration
            LoadCell.SetInput(true); // turn on load cell power
            _logger.LogInformation($"waiting... for {PS_Delay.TotalMilliseconds} ms");
            await Task.Delay(PS_Delay);
            await currentPointMesure(ctp, await MultiMeter.Mesure(KeithleyDMM6500.MeasurementType.Current, this.MesuremntDelay, this.AvgCount));
        }

        private async Task TempPointMesure(TempTestPoint tp) {
            UBA_PROTO_QUERY.RECIPIENT recipient = tp.Line == UBA_PROTO_LINE.ID.A ? UBA_PROTO_QUERY.RECIPIENT.LineA : UBA_PROTO_QUERY.RECIPIENT.LineB;
            Message m = await UBA.GetMessage(recipient);
            tp.UpdatedFromUBA_Messsage(m.QueryResponse.Line);
            _logger.LogInformation("Temperature Point Measure: {tp}", tp);
        }

        private async Task<LinerEquation> CalibrateVoltage(VoltageCalibration vc) {          
            if (IsInEmulationMode) {
                _logger.LogWarning("Emulation Mode: Returning random LinerEquation."); 
                await voltagePointMesure(vc.P1, AddRandom10Percent(vc.P1.Value2Set));
                await voltagePointMesure(vc.P2, AddRandom10Percent(vc.P2.Value2Set));
            } else {
                await voltagePointMesure(vc.P1);
                await voltagePointMesure(vc.P2);
            }
            _logger.LogInformation($"Calibrate Voltage result: {vc.ToString()}");
            return vc.Equation;
        }
        private async Task<LinerEquation> CalibrateCurrent(CurrentCalibration ccc) {
            if (IsInEmulationMode) {
                _logger.LogWarning("Emulation Mode: Returning random LinerEquation.");
                await currentPointMesure(ccc.P1, AddRandom10Percent(ccc.P1.Value2Set));
                await currentPointMesure(ccc.P1, AddRandom10Percent(ccc.P1.Value2Set));
            } else {
                await PowerSupply.SetOverCurrent(10000); //10000mA 10A
                await currentPointMesure(ccc.P1);
                await currentPointMesure(ccc.P2);                
            }
            return ccc.Equation;
        }

        private async Task<LinerEquation> CalibrateVoltage(TempCalibration tc) {
            LinerEquation ret;
            UBA_PROTO_QUERY.RECIPIENT recipient = tc.LineID== UBA_PROTO_LINE.ID.A ? UBA_PROTO_QUERY.RECIPIENT.LineA : UBA_PROTO_QUERY.RECIPIENT.LineB;            
            Message m = await UBA.GetMessage(recipient);            
            if (tc.P1.Type == UBA_CALIBRATION_TEMP_TYPE.AMBIANT_TEMP) {
                tc.P1.UBA_MeasuredValue = m.QueryResponse.Line.Data.AmbTemperature; ;
            } else if (tc.P1.Type == UBA_CALIBRATION_TEMP_TYPE.BATTERY_TEMP) {                
                tc.P1.UBA_MeasuredValue = m.QueryResponse.Line.Data.BatTemperature;
            } else {
                throw new ArgumentException($"Unsupported UBA_CALIBRATION_TEMP_TYPE: {tc.P1.Type}");
            }
            
            return tc.Equation;
        }


        public async Task<List<TempCalibration>> CalibrateAllTemp(UBA_CALIBRATION_TEMP_TYPE lineA_Temp, UBA_CALIBRATION_TEMP_TYPE lineB_Temp, bool emulate = false) {
            List<TempTestPoint> points = new List<TempTestPoint>();
            List<TempCalibration> vp = new List<TempCalibration>();
            foreach (UBA_CALIBRATION_TEMP_TYPE flag in Enum.GetValues(typeof(UBA_CALIBRATION_TEMP_TYPE))) {
                if (lineA_Temp.HasFlag(flag) && flag != UBA_CALIBRATION_TEMP_TYPE.NONE) {
                    Console.WriteLine($"Flag: {flag}");
                    vp.Add(new TempCalibration(UBA_PROTO_LINE.ID.A, flag));
                }
                if (lineB_Temp.HasFlag(flag) && flag != UBA_CALIBRATION_TEMP_TYPE.NONE) {
                    Console.WriteLine($"Flag: {flag}");
                    vp.Add(new TempCalibration(UBA_PROTO_LINE.ID.B, flag));
                }
            }
            foreach (TempCalibration cal in vp) {
                points.Add(cal.P1);             
            }
            foreach (TempTestPoint p in points) {
                await TempPointMesure(p);
            }
            return vp;
        }
        public async Task<List<VoltageCalibration>> CalibrateAllVoltages(UBA_CALIBRATION_VOLTAGE_TYPE lineA_Voltages, UBA_CALIBRATION_VOLTAGE_TYPE lineB_Voltages, bool emulate = false) {
            List<VoltageTestPoint> points = new List<VoltageTestPoint>();
            List<VoltageCalibration> vp = new List<VoltageCalibration>();
            foreach (UBA_CALIBRATION_VOLTAGE_TYPE flag in Enum.GetValues(typeof(UBA_CALIBRATION_VOLTAGE_TYPE))) {
                if (lineA_Voltages.HasFlag(flag) && flag != UBA_CALIBRATION_VOLTAGE_TYPE.NONE) {
                    Console.WriteLine($"Flag: {flag}");
                    vp.Add(new VoltageCalibration(UBA_PROTO_LINE.ID.A, flag));
                }
                if (lineB_Voltages.HasFlag(flag) && flag != UBA_CALIBRATION_VOLTAGE_TYPE.NONE) {
                    Console.WriteLine($"Flag: {flag}");
                    vp.Add(new VoltageCalibration(UBA_PROTO_LINE.ID.B, flag));
                }
            }
            foreach (VoltageCalibration cal in vp) {
                points.Add(cal.P1);
                points.Add(cal.P2);
            }
            points.Sort();
            if (emulate == false) {
                await PowerSupply.SetABC_Select(BK_PRECISION9104Libary.BK_PRECISION9104.ABC_PRESET.NORMAL);
                await PowerSupply.SetOutputCurrnt(BK_PRECISION9104.ABC_PRESET.NORMAL, 1000);
            }            
            int? lastValue2set = null;
            float mmValue = 0;
            foreach (VoltageTestPoint point in points) {
                if (lastValue2set == null || point.Value2Set != lastValue2set) {
                    Console.WriteLine($"Processing TestPoint: {point}");
                    lastValue2set = (int)point.Value2Set;
                    await PowerSupply.SetOutputVoltage(BK_PRECISION9104Libary.BK_PRECISION9104.ABC_PRESET.NORMAL, (int)point.Value2Set, true);
                    mmValue = await MultiMeter.Mesure(KeithleyDMM6500.MeasurementType.Voltage, this.MesuremntDelay, this.AvgCount);
                }
                await voltagePointMesure(point, mmValue);                
                Console.WriteLine(point);
            }
            await PowerSupply.SetOutput(false); // turn off the power supply
            return vp;
        }

        
        public async Task<List<LinerEquation>> BatteryCalibration(UBA_PROTO_LINE.ID line) {
            List<VoltageCalibration> vcList = new List<VoltageCalibration>() {
                new VoltageCalibration(line, UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V),
                new VoltageCalibration(line, UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V),
                new VoltageCalibration(line, UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE60V),
            };
            List<LinerEquation> calList = new List<LinerEquation>();
            foreach (VoltageCalibration vc in vcList) {
                calList.Add(await CalibrateVoltage(vc));
            }
            await PowerSupply.SetOutput(false); // turn off the power supply
            return calList;
        }
        public async Task<LinerEquation> VPS_Calibration(UBA_PROTO_LINE.ID line) {
            VoltageCalibration vpsCalibration = new VoltageCalibration(line, UBA_CALIBRATION_VOLTAGE_TYPE.VPS);
            return await CalibrateVoltage(vpsCalibration);
        }

        public async Task<LinerEquation> GenVoltage_Calibration(UBA_PROTO_LINE.ID line, bool IsMM_Emulation = false) {
            try {
              VoltageCalibration vpsCalibration = new VoltageCalibration(line, UBA_CALIBRATION_VOLTAGE_TYPE.GEN_VOLTAGE);
                return await CalibrateVoltage(vpsCalibration);
            } catch (Exception e) {
                Debug.WriteLine(e);
                Console.WriteLine(e);
                throw new Exception("Failed to Calibrate Vgen", e);
            } finally {
            }
        }

        public async Task<LinerEquation> ChargeCuurentCalibration(UBA_PROTO_LINE.ID line) {
            try {
                CurrentCalibration ccc =  new CurrentCalibration(line, UBA_CALIBRATION_CURRENT_TYPE.CHARGE_CURRENT, MaxChargeCurrent);
                return await CalibrateCurrent(ccc);                 
            } catch (Exception ex) {
                throw RaiseException(ex);
            } finally {
                if (IsInEmulationMode == false) {
                    await PowerSupply.SetOutput(false, 0);
                    LoadCell.SetInput(false); // turn off load cell power
                }
            }        
        }
        public async Task<LinerEquation> DischargeCuurentCalibration(UBA_PROTO_LINE.ID line) {
            try {
                CurrentCalibration ccc = new CurrentCalibration(line, UBA_CALIBRATION_CURRENT_TYPE.DISCHARGE_CURRENT, MaxDischargeCurrent);
                return await CalibrateCurrent(ccc);
            } catch (Exception ex) {
                throw RaiseException(ex);
            } finally {
                if (IsInEmulationMode==false) {
                    await PowerSupply.SetOutput(false, 0);
                    LoadCell.SetInput(false); // turn off load cell power
                }
            }
        }


     


        public async Task<LinerEquation> AmbiantTempCalibration(UBA_PROTO_LINE.ID line) {
            if (IsInEmulationMode) {
                return RandomLinerEquation();
            }
            UBA_PROTO_QUERY.RECIPIENT recipient = line == UBA_PROTO_LINE.ID.A ? UBA_PROTO_QUERY.RECIPIENT.LineA : UBA_PROTO_QUERY.RECIPIENT.LineB;
            float expectedTemp = 25.0f, mesureTemp;
            Message m = await UBA.GetMessage(recipient);
            mesureTemp = m.QueryResponse.Line.Data.AmbTemperature;
            return CalculateLinerEquation(expectedTemp, mesureTemp);
        }

        public async Task<LinerEquation> BatteryTempCalibration(UBA_PROTO_LINE.ID line) {
            if (IsInEmulationMode) {
                return RandomLinerEquation();
            }
            UBA_PROTO_QUERY.RECIPIENT recipient = line == UBA_PROTO_LINE.ID.A ? UBA_PROTO_QUERY.RECIPIENT.LineA : UBA_PROTO_QUERY.RECIPIENT.LineB;
            float expectedTemp = 25.0f, mesureTemp;
            Message m = await UBA.GetMessage(recipient);
            mesureTemp = m.QueryResponse.Line.Data.BatTemperature;
            return CalculateLinerEquation(expectedTemp, mesureTemp);
        } 
        public void RaiseNewStatusEvent(string statusStr) {
            StatusEventArg newEvent = new StatusEventArg(statusStr);
            _logger.LogInformation("Status changed: {Status}", newEvent);
            StatusChanged?.Invoke(this, new StatusEventArg(statusStr));
        }

        public void RaiseNewStatusEvent(string statusStr, int progress) {
            StatusEventArg newEvent = new StatusEventArg(statusStr, progress);
            _logger.LogInformation("Status changed: {Status}", newEvent);
            StatusChanged?.Invoke(this, newEvent);            
        }

        public Exception RaiseException(Exception ex) {
            _logger.LogError(ex, "An exception occurred in: {Message}", ex.Message);
            ExceptionOccurred?.Invoke(this, new ExceptionEventArg(ex));
            return ex;
        }
    }
}

