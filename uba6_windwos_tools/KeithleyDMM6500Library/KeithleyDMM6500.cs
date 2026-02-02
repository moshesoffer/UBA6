
using System;
using System.Diagnostics;
using System.IO;
using System.Net.NetworkInformation;
using System.Text.RegularExpressions;
using AmicellUtil;
using Ivi.Visa.Interop;
using Microsoft.Extensions.Logging;


namespace KeithleyDMM6500Library {

    public class KeithleyDMM6500  : AmicellDevice<KeithleyDMM6500> {
        public string VisaAddress = "USB0::0x05E6::0x6500::04624183::0::INSTR";
        public string Manufacturer = string.Empty;
        public string Model =string.Empty;
        public string Serial =string.Empty;
        public string Firmware = string.Empty;
        private bool isSimulate = false;
        private float voltage;
        private float current;

        private ResourceManager rm = new ResourceManager();
        private FormattedIO488 io = new FormattedIO488();

        public event EventHandler<StatusEventArg> StatusChanged;
        public event EventHandler<ExceptionEventArg> ExceptionOccurred;

        public enum MeasurementType {
            Voltage,
            Current
        }

        public KeithleyDMM6500(ILogger<KeithleyDMM6500> log) :base(log){ 

        }
        public KeithleyDMM6500(ILogger<KeithleyDMM6500> log,string visaAddress): this(log) {
            VisaAddress = visaAddress;
        }
        public KeithleyDMM6500(ILogger<KeithleyDMM6500> log,string visaAddress, bool simulate) :this(log,visaAddress) {
            isSimulate = simulate;
        }

        public bool ChecKConnection() {
            try {
                string idn;
                if (isSimulate) {
                    idn = "KEITHLEY INSTRUMENTS,MODEL DMM6500,04624183,1.7.12b";
                } else {
                    // Open session
                    io.IO = (IMessage)rm.Open(VisaAddress, AccessMode.NO_LOCK, 2000, "");
                    io.WriteString("*IDN?\n");
                    idn = io.ReadString(); // KEITHLEY INSTRUMENTS,MODEL DMM6500,04624183,1.7.12b
                }                
                Match match = Regex.Match(idn, @"^(?<manufacturer>.*?),MODEL (?<model>[^,]+),(?<serial>[^,]+),(?<firmware>[\d\.]+[a-zA-Z]?)$");

                if (match.Success) {
                    Manufacturer = match.Groups["manufacturer"].Value;
                    Model = match.Groups["model"].Value;
                    Serial = match.Groups["serial"].Value;
                    Firmware = match.Groups["firmware"].Value;                   
                    return true;
                } else {
                    Console.WriteLine("No match found.");
                    throw new Exception($"Failed to Paras IDN:{idn}");
                }                
            } catch (Exception ex) { 
                return false;
            } 
        }


        private float GetVoltage(int rpet = 2) {
            try {
                string voltageStr;
                if (isSimulate) {
                    voltageStr = "-9.089944E-06";
                } else {
                    // Open session
                    io.IO = (IMessage)rm.Open(VisaAddress, AccessMode.NO_LOCK, 2000, "");
                    io.WriteString("MEAS:VOLT:DC?\n");
                    voltageStr = io.ReadString();                    
                }
                if (float.TryParse(voltageStr, out voltage)) {
                    Debug.WriteLine($"Read Voltage:{voltage}");
                    if (rpet > 0) {
                        return GetVoltage(rpet-1);
                    } else { 
                        return voltage;
                    }
                } else {
                    throw new Exception("failed to Paras voltage string");
                }
            } catch (Exception ex) {
                Console.WriteLine("Error: " + ex.Message);
                return 0;
            }
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="rpet"></param>
        /// <returns>the current in A</returns>
        private float GetCurrent(int rpet = 2) {
            try {
                string currentStr;
                if (isSimulate) {
                    currentStr = "4.071613E-08";
                } else {
                    // Open session
                    io.IO = (IMessage)rm.Open(VisaAddress, AccessMode.NO_LOCK, 2000, "");
                    io.WriteString("MEAS:CURR:DC?\n");
                    currentStr = io.ReadString();
                }
                if (float.TryParse(currentStr, out current)) {
                    Debug.WriteLine($"Read Current:{current}");
                    if (rpet > 0) {
                        return GetCurrent(rpet - 1);
                    } else {
                        return current;
                    }
                } else {
                    throw new Exception("failed to Paras voltage string");
                }
            } catch (Exception ex) {
                Console.WriteLine("Error: " + ex.Message);
                return 0;
            }
        }

        public override string ToString() {
            return $"{Manufacturer},{Model},{Serial},{Firmware}-{(isSimulate? "Simulate" : "" )}";
         
        }

        public void RaiseNewStatusEvent(string statusStr) {
            throw new NotImplementedException();
        }

        public void RaiseNewStatusEvent(string statusStr, int progress) {
            throw new NotImplementedException();
        }

        public Exception RaiseException(Exception ex) {
            throw new NotImplementedException();
        }

        public override Task<float> Mesure<TEnum>(TEnum Type) {
            if(IsInEmulationMode) {
                _logger.LogWarning($"Emulation Mode: Returning default value for {Type}.");                
                return Task.FromResult(AmicellUtil.Util.RandomFloat()); // Return a default value in emulation mode
            }
            if (Type is MeasurementType measurementType) {
                switch (measurementType) {
                    case MeasurementType.Voltage:
                        return Task.FromResult(GetVoltage()*1000);
                    case MeasurementType.Current:
                        return Task.FromResult(GetCurrent()*1000);
                    default:
                        throw new ArgumentOutOfRangeException(nameof(Type), $"Unsupported measurement type: {measurementType}");
                }
            } else {
                throw new ArgumentException("Invalid measurement type", nameof(Type));

            }
        }
    }
}
