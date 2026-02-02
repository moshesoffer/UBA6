using AmicellUtil;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using System.IO.Ports;
using System.Text.RegularExpressions;
using static BK_PRECISION9104Libary.BK_PRECISION9104.Command;

namespace BK_PRECISION9104Libary {
    public partial class BK_PRECISION9104 : AmicellDevice<BK_PRECISION9104>,IDisposable{
        public event EventHandler<ReadingEventArgs> ReadingReceived;

        static readonly int MILI_MAX_VALUE = 999999;
        static readonly int MILI_MIN_VALUE = 0;
        static readonly int MAX_CURRENT = 8000; // 8000 mA = 8 A
        public static double TolerancePercentage { get; set; } = 5; // 5%

        private static int Mili2Centi(int mV) {
            if ((mV > MILI_MAX_VALUE) || (mV < MILI_MIN_VALUE)) {
                throw new OutOfBoundException(mV, MILI_MIN_VALUE, MILI_MAX_VALUE);
            }
            return (mV / 10);
        }

        private static int Centi2Mili(int centi) {
            if ((centi > Mili2Centi(MILI_MAX_VALUE)) || (centi < Mili2Centi(MILI_MIN_VALUE))) {
                throw new OutOfBoundException(centi, Mili2Centi(MILI_MIN_VALUE), Mili2Centi(MILI_MAX_VALUE));
            }
            return (centi * 10);
        }

        public enum ABC_PRESET {
            A = 0,
            B = 1,
            C = 2,
            NORMAL = 3
        }

        public enum CV_CC_Mode {
            CV_MODE = 0,
            CC_MODE = 1,

        }
        public enum MeasurementType {
            Voltage = 0,
            Current = 1,
            Mode = 2
        }


        public SerialPort SerialPort;
        public List<PreSet> PreSetValues = new List<PreSet>() { new PreSet(), new PreSet(), new PreSet(), new PreSet() };
        public int Voltage { get; set; }
        public int UVL { get; set; }
        public int Current { get; set; }
        public int UCL { get; set; }
        public ABC_PRESET Preset { get; set; }
        public CV_CC_Mode CV_CC_mode { get; set; }
        public bool IsOutput { get; set; }

        public bool IsAutoRead { get; set; } = false;
        private CancellationToken ct { get; set; }
        private CancellationTokenSource _cts = new CancellationTokenSource();

        public BK_PRECISION9104(ILogger<BK_PRECISION9104> logger) :base(logger) {
            _logger.LogInformation("BK_PRECISION9104 instance created.");
        }

        public BK_PRECISION9104(ILogger<BK_PRECISION9104> logger,string comPort) : this(logger){
            SetPort(comPort);
        }
        public void SetPort(string comPort) {
            try {
                if (AmicellUtil.Util.IsComPortExists(comPort)) {
                    SerialPort = new SerialPort(comPort, 9600, Parity.None, 8, StopBits.One);
                    SerialPort.ReadTimeout = 500;
                    SerialPort.WriteTimeout = 500;
                    SerialPort.NewLine = "\r";
                    RaiseNewStatusEvent($"New Port was Set {SerialPort.ToString()}");
                }
            } catch (Exception ex) {
                RaiseException(new Exception("Failed To Set The Device Port", ex));
            }
        }

        public async Task StartAutoRead() {
            if (_cts.IsCancellationRequested) {
                _cts?.Cancel();
            }
            IsAutoRead = true;
            _cts = new CancellationTokenSource();
            while (!_cts.Token.IsCancellationRequested && IsAutoRead) {
                try {
                    await GetReadingVoltCurrAndMode();
                } catch (Exception ex) {
                    RaiseException(ex);
                    Console.WriteLine(ex);
                } finally {
                    await Task.Delay(1000, _cts.Token); // Wait 1 second
                }
            }
            _cts?.Cancel();
        }

        public void StopAutoRead() {
            _cts?.Cancel();
            IsAutoRead = false;
        }


        public async Task SetOutput(bool output, int delay = 10000) {
            try {
                if (IsInEmulationMode) {
                    _logger.LogWarning("Emulation Mode: Skipping SetOutput.");
                    return; // Skip in emulation mode
                }
                string ret = await sendCommand(new Command(PowerSupplyCommand.SOUT, output));
                await Task.Delay(delay);
                RaiseNewStatusEvent($"Output was set To {(output ? "ON" : "Off")}");
            } catch (Exception ex) {
                RaiseException(new Exception("failed To Set the Output Value", ex));
            }
        }

        public async Task GetOutput() {
            try {
                if (IsInEmulationMode) {
                    _logger.LogWarning("Emulation Mode: Skipping GetOutput.");                 
                    return; // Skip in emulation mode
                }
                string ret = await sendCommand(new Command(PowerSupplyCommand.GOUT));
                var match = Regex.Match(ret, @"(?<output>[01])");
                if (!match.Success) {
                    throw new Exception("Failed to read res");
                }
                IsOutput = match.Groups["output"].Value == "1" ? true : false;
            } catch (Exception ex) {
                RaiseException(new Exception("Failed to get the Output Value", ex));
            }
        }
        /// <summary>
        /// set the Voltage of the prest Type in mV
        /// </summary>
        /// <param name="presetType"></param>
        /// <param name="voltage"> the Voltage in mV</param> 
        /// <returns></returns>
        /// <exception cref="Exception"></exception>
        public async Task SetOutputVoltage(ABC_PRESET presetType, int voltage, bool setOutput = false ) {
            try {
                if(IsInEmulationMode) {
                    _logger.LogWarning("Emulation Mode: Skipping SetOutputVoltage.");
                    return; // Skip in emulation mode
                }
                CancellationTokenSource cts = new CancellationTokenSource();
                string ret = await sendCommand(new Command(PowerSupplyCommand.VOLT, presetType, voltage));
                PreSetValues[(int)presetType].Voltage = Centi2Mili(voltage);
                if (setOutput) {
                    cts.CancelAfter(TimeSpan.FromSeconds(100));
                    await SetOutput(true,10);
                    do {
                        if (cts.IsCancellationRequested) { 
                                throw  new Exception("Operation was cancelled due to timeout or user request.");    
                        }                    
                        await GetReadingVoltCurrAndMode(); // get the voltage and current 
                        await Task.Delay(100, cts.Token); // Wait 100 ms
                    } while ((AmicellUtil.Util.IsWithinPercentage(Voltage, voltage, TolerancePercentage) == false ));
                }
            } catch (Exception ex) {
                RaiseException(new Exception("Failed to set the Output Voltage", ex));
            }
        }      

        public async Task SetOutputCurrnt(ABC_PRESET presetType, int currnt) {
            try {
                if (currnt > MAX_CURRENT) {
                    throw new OutOfBoundException(currnt, 0, MAX_CURRENT);  
                }
                if (IsInEmulationMode) {
                    _logger.LogWarning("Emulation Mode: Skipping SetOutputCurrnt.");                    
                }else {
                    _logger.LogDebug($"Set Output Current to {currnt} mA for preset {presetType}");
                    string ret = await sendCommand(new Command(PowerSupplyCommand.CURR, presetType, currnt));
                }
                PreSetValues[(int)presetType].Current = currnt;
            } catch (Exception ex) {                
                RaiseException( new Exception("Failed to set the Output current", ex));
            }
        }
        public async Task SetOverVoltage(int voltage) {
            try {
                if (IsInEmulationMode) {
                    _logger.LogWarning("Emulation Mode: Skipping SetOverVoltage.");
                } else { 
                    string ret = await sendCommand(new Command(PowerSupplyCommand.SOVP, voltage));
                }
                this.UVL = voltage;
            } catch (Exception ex) {
                RaiseException(new Exception("Failed to set Over Voltage", ex));
            }
        }
        public async Task GetReadingVoltCurrAndMode() {
            try {
                if (IsInEmulationMode) {
                    _logger.LogWarning("Emulation Mode: Skipping GetReadingVoltCurrAndMode.");
                    Voltage = AmicellUtil.Util.RandomInt(0, MILI_MAX_VALUE);
                    Current = AmicellUtil.Util.RandomInt(0, MILI_MAX_VALUE);
                    CV_CC_mode = (CV_CC_Mode)AmicellUtil.Util.RandomInt(0, 1);
                    ReadingReceived?.Invoke(this, new ReadingEventArgs(Voltage, Current, CV_CC_mode));
                    RaiseNewStatusEvent($"Reading received successfully {Voltage}mv {Current}mA ");
                    return; // Skip in emulation mode

                }
                string ret = await sendCommand(new Command(PowerSupplyCommand.GETD));
                var match = Regex.Match(ret, @"(?<volt>\d{4})(?<curr>\d{4})(?<mode>[01])");
                if (!match.Success) {
                    throw new Exception($"Failed Parse Commad Response: {ret}");
                }
                Voltage = Centi2Mili(int.Parse(match.Groups["volt"].Value));
                Current = Centi2Mili(int.Parse(match.Groups["curr"].Value));
                CV_CC_mode = match.Groups["mode"].Equals("0") ? CV_CC_Mode.CV_MODE : CV_CC_Mode.CC_MODE;
                ReadingReceived?.Invoke(this, new ReadingEventArgs(Voltage, Current, CV_CC_mode));
                RaiseNewStatusEvent($"Reading received successfully {Voltage}mv {Current}mA ");
            } catch (Exception ex) {
                RaiseException(new Exception("Failed to get Voltage Curretn & Mode", ex));
            }
        }
        public async Task SetOverCurrent(int overCurrent) {
            try {
                _logger.LogDebug($"Set Over Current to {overCurrent} mA");
                await sendCommand(new Command(PowerSupplyCommand.SOCP, overCurrent));
            } catch (Exception ex) {
                RaiseException(new Exception("Failed to set the over Current Value", ex));
            }
        }

        public async Task GetUpperLimitVoltage() {
            try {
                string ret = await sendCommand(new Command(PowerSupplyCommand.GOVP));
                var match = Regex.Match(ret, @"(?<voltage>\d{4})");
                int OverVoltage = Centi2Mili(int.Parse(match.Groups["voltage"].Value));
                if (!match.Success) {
                    throw new Exception("Failed to read res");
                }
            } catch (Exception ex) {
                RaiseException (new Exception("Failed to get the over Voltage Value", ex));
            }
        }

        public async Task GetUpperLimitCurrent() {
            try {
                string ret = await sendCommand(new Command(PowerSupplyCommand.GOCP));
                var match = Regex.Match(ret, @"(?<current>\d{4})");
                int OverCurrent = Centi2Mili(int.Parse(match.Groups["current"].Value));
                if (!match.Success) {
                    throw new Exception("Failed to read response Message");
                }

            } catch (Exception ex) {
                RaiseException(new Exception("Failed to get the over Voltage Value", ex));
            }
        }

        /// <summary>
        ///  set the values of the preset
        /// </summary>
        /// <param name="preset"></param> the preset to set
        /// <param name="voltage"></param> voltage in mV
        /// <param name="current"></param> Currant in MA
        /// <returns></returns>
        /// <exception cref="Exception"></exception>
        public async Task SetPresetVoltageAndCuurent(ABC_PRESET preset, int voltage, int current) {
            try {
                await sendCommand(new Command(PowerSupplyCommand.SETD, preset, voltage, current));
                if (preset < ABC_PRESET.NORMAL) {
                    this.PreSetValues.ElementAt((int)preset).Voltage = voltage;
                    this.PreSetValues.ElementAt((int)preset).Current = current;
                }
                RaiseNewStatusEvent($"Preset {preset} configured to {voltage} mV and {current} mA");
            } catch (Exception ex) {
                RaiseException( new Exception($"Failed To set Preset: {preset} Voltage: {voltage} & Current: {current}", ex));
            }
        }

        public async Task GetPresetVoltageAndCuurent(ABC_PRESET preset) {
            try {
                string ret = await sendCommand(new Command(PowerSupplyCommand.GETS, preset));
                var match = Regex.Match(ret, @"(?<Voltage>\d{4})(?<Current>\d{4})");
                if (!match.Success) {
                    throw new Exception("Failed to read res");
                }
                if (preset < ABC_PRESET.NORMAL) {
                    this.PreSetValues.ElementAt((int)preset).Voltage = Centi2Mili(int.Parse(match.Groups["Voltage"].Value));
                    this.PreSetValues.ElementAt((int)preset).Current = Centi2Mili(int.Parse(match.Groups["Current"].Value));
                }
                RaiseNewStatusEvent($"Preset {preset} read as Voltage={PreSetValues[(int)preset].Voltage} mV, Current={PreSetValues[(int)preset].Current} mA");
            } catch (Exception ex) {
                RaiseException(new Exception($"Failed To get Preset: {preset} ", ex));
            }
        }

        public async Task<ABC_PRESET> GetPreSetSelection() {
            try {
                string ret = await sendCommand(new Command(PowerSupplyCommand.GABC));
                var match = Regex.Match(ret, @"(?<preset>[0123])");
                if (!match.Success) {
                    throw new Exception("Failed to read res");
                }
                Preset = (ABC_PRESET)int.Parse(match.Groups["preset"].Value);
                RaiseNewStatusEvent($"Current preset selection is {Preset}");
                return Preset;
            } catch (Exception ex) {
                throw RaiseException( new Exception($"Failed To get Preset Selection"));                
            }
        }
        public async Task SetABC_Select(ABC_PRESET preset) {
            try {
                await sendCommand(new Command(PowerSupplyCommand.SABC, preset));
                Preset = preset;
                RaiseNewStatusEvent($"Preset selection was set to {preset}");
            } catch (Exception ex) {
                RaiseException( new Exception("Failed to set Preset Values"));
            }
        }


        public async Task DisableKeyboard() {
            try {
                await sendCommand(new Command(PowerSupplyCommand.SESS));
                RaiseNewStatusEvent("Keyboard disabled successfully");
            } catch (Exception ex) {
                RaiseException( new Exception("Failed to Disable Keyboard"));
            }
        }
        public async Task EnableKeyboard() {
            try {
                await sendCommand(new Command(PowerSupplyCommand.ENDS));
                RaiseNewStatusEvent("Keyboard enabled successfully");
            } catch (Exception ex) {
                RaiseException( new Exception("Failed to Enable Keyboard"));
            }
        }

        public async Task Getinformation() {
            try {
                string ret = await sendCommand(new Command(PowerSupplyCommand.GALL));
                Console.WriteLine($"the Length Of the line|{ret}| is: {ret.Length} ");

                var match = Regex.Match(ret, @"(?<AbcSele>\d{1})
                                                (?<Channel>\d{1})
                                                (?<UVL>\d{4})
                                                (?<UCL>\d{4})
                                                (?<output>[01])                                                
                                                (?<Swtime1>\d{3})
                                                (?<Swtime2>\d{3})
                                                (?<Swtime3>\d{3})                                                
                                                (?<Deltatimes>\d{12})
                                                (?<Mode>\d{4})
                                                (?<Setv1>\d{4})
                                                (?<Seti1>\d{4})
                                                (?<Setv2>\d{4})
                                                (?<Seti2>\d{4})
                                                (?<Setv3>\d{4})
                                                (?<Seti3>\d{4})
                                                (?<Setv4>\d{4})
                                                (?<Seti4>\d{4})", RegexOptions.IgnorePatternWhitespace);
                if (!match.Success) {
                    throw new Exception("Failed to Pars the Command");
                }
                Preset = (ABC_PRESET)int.Parse(match.Groups["AbcSele"].Value);
                UVL = Centi2Mili(int.Parse(match.Groups["UVL"].Value));
                UCL = Centi2Mili(int.Parse(match.Groups["UCL"].Value));
                IsOutput = match.Groups["output"].Value == "1" ? true : false;
                PreSetValues[(int)ABC_PRESET.A].Voltage = Centi2Mili(int.Parse(match.Groups["Setv1"].Value));
                PreSetValues[(int)ABC_PRESET.A].Current = Centi2Mili(int.Parse(match.Groups["Seti1"].Value));
                PreSetValues[(int)ABC_PRESET.B].Voltage = Centi2Mili(int.Parse(match.Groups["Setv2"].Value));
                PreSetValues[(int)ABC_PRESET.B].Current = Centi2Mili(int.Parse(match.Groups["Seti2"].Value));
                PreSetValues[(int)ABC_PRESET.C].Voltage = Centi2Mili(int.Parse(match.Groups["Setv3"].Value));
                PreSetValues[(int)ABC_PRESET.C].Current = Centi2Mili(int.Parse(match.Groups["Seti3"].Value));
                PreSetValues[(int)ABC_PRESET.NORMAL].Voltage = Centi2Mili(int.Parse(match.Groups["Setv4"].Value));
                PreSetValues[(int)ABC_PRESET.NORMAL].Current = Centi2Mili(int.Parse(match.Groups["Seti4"].Value));
                await GetReadingVoltCurrAndMode();
                RaiseNewStatusEvent("Full device information retrieved");
            } catch (Exception ex) {
                RaiseException( new Exception("Failed to Get All Info", ex));
            }
        }

        public async Task ConfigPreset(List<PreSet> presetList) {
            try {
                if (presetList.Count != 3) {
                    throw RaiseException(new Exception("List is not in the current length"));
                }
                await this.sendCommand(new Command(PowerSupplyCommand.SETM, presetList));
                RaiseNewStatusEvent("All presets configured successfully");
            } catch (Exception ex) {
               throw RaiseException( new Exception("Failed to config All Presets", ex));
            }
        }


        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e) {
            try {
                string msg = string.Empty;
                while ((SerialPort.BytesToRead > 0) && (ct.IsCancellationRequested == false)) {
                    msg = SerialPort.ReadLine();
                    if (msg.Equals("OK\r")) {
                        Console.WriteLine("ACK");
                    }
                }
            } catch (Exception ex) {
            }
        }

        private async Task<string> sendCommand(Command cmd) {
            if (SerialPort.IsOpen) {
            } else {
                SerialPort.Open();
            }
            await Task.Delay(10);
            SerialPort.DiscardInBuffer();
            string line = cmd.Format();            
            _logger.LogDebug($"Write Command Line:|{line}|");
            SerialPort.WriteLine(line);
            RaiseNewStatusEvent($"Send Command: {cmd.ToString()}");
            await Task.Delay(100);
            if (SerialPort.BytesToRead > 0) {
                line = SerialPort.ReadTo("OK\r");
                while (SerialPort.BytesToRead > 0) {
                    string restOfLine = SerialPort.ReadExisting();
                    Debug.WriteLine($"Rest Of line :{restOfLine}");
                }
                Console.WriteLine($"Command Response:|{line}|");
                return line.Trim();
            } else {
                throw RaiseException( new Exception("Device cannot be reatched"));
            }
        }

        public static string FormatCommand(PowerSupplyCommand cmd, int param) {
            return $"{cmd.ToString().ToUpper()}{param}\r";

        }

        public override string ToString() {
            return $"Voltage {Voltage} Current:{Current} Mode:{CV_CC_mode}";
        }
        public string DeviceName() {

            return $"BK PRECISION 9104 - {SerialPort.PortName}";
        }

       public override async Task<float> Mesure<TEnum>(TEnum Type)  {
            if (IsInEmulationMode) {
                _logger.LogWarning($"Emulation Mode: Returning default value for {Type}.");
                return AmicellUtil.Util.RandomFloat(); // Return a default value in emulation mode
            }
            await this.GetReadingVoltCurrAndMode();
            switch (Type) {
                case MeasurementType.Voltage:
                    return (float)Voltage;
                case MeasurementType.Current:
                    return (float)Current;
                case MeasurementType.Mode:
                    return (float)((int)CV_CC_mode);
                default:
                    throw new ArgumentException("Invalid measurement type");


            }
        }
        public void Dispose() {
            if (SerialPort.IsOpen) {
                SerialPort.Close();
            }
            SerialPort.Dispose();
        }
    }
}
