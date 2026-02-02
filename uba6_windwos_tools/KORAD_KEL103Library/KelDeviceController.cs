using AmicellUtil;
using Microsoft.Extensions.Logging;
using System;
using System.IO.Ports;
using System.Runtime.CompilerServices;
using System.Text.RegularExpressions;

public class KelDeviceController : AmicellDevice<KelDeviceController>, IDisposable {
    public SerialPort SerialPort;
    public bool IsConnected {
        get {
            return SerialPort?.IsOpen ?? false;
        }
    }
    public enum FunctionMode {
        CV,
        CC,
        CR,
        CW
    }

    public enum MeasurementType {
        Voltage,
        Current,
        Resistance,
        Power
    }


    public KelDeviceController(ILogger<KelDeviceController> logger, string portName) :base(logger) {
        if (AmicellUtil.Util.IsComPortExists(portName)) {
            SetPort(portName);            
        }
        RaiseNewStatusEvent("Initializing KEL Device Controller...");
    }

    private static double ExtractVoltage(string voltageStr) {
        var match = Regex.Match(voltageStr, @"^([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)V$");
        if (match.Success && double.TryParse(match.Groups[1].Value, out double value))
            return value;       
        throw new FormatException($"Invalid voltage format: '{voltageStr}'");
    }
    private static double ExtractCurrent(string CurrentStr) {
        var match = Regex.Match(CurrentStr, @"^([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)A$");
        if (match.Success && double.TryParse(match.Groups[1].Value, out double value))
            return value;
        throw new FormatException($"Invalid Current format: '{CurrentStr}'");
    }
    private static double ExtractResistance(string ResistanceStr) {
        var match = Regex.Match(ResistanceStr, @"^([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)OHM$");
        if (match.Success && double.TryParse(match.Groups[1].Value, out double value))
            return value;
        throw new FormatException($"Invalid Resistance format: '{ResistanceStr}'");
    }
    private static double ExtractPower(string PowerStr) {
        var match = Regex.Match(PowerStr, @"^([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)W$");
        if (match.Success && double.TryParse(match.Groups[1].Value, out double value))
            return value;
        throw new FormatException($"Invalid Power format: '{PowerStr}'");
    }

    private string SendCommand(string command, bool expectResponse = true) {
        _logger.LogInformation($"Sending command: |{command}|");
        SerialPort.WriteLine(command);
        if (expectResponse) {
            string Response = SerialPort.ReadLine();
            _logger.LogInformation($"Received response: {Response}");
            return Response;
        }
        return null;
    }

    public string GetDeviceId() => SendCommand("*IDN?");

    public void SaveState(int slot) => SendCommand($"*SAV {slot}", false);

    public void RecallState(int slot) => SendCommand($"*RCL {slot}", false);

    public void Trigger() => SendCommand("*TRG", false);

    public void SetBeep(bool on) => SendCommand($":SYSTem:BEEP {(on ? "ON" : "OFF")}", false);

    public bool QueryBeep() => SendCommand(":SYSTem:BEEP?").Trim().Equals("ON");

    public string QueryStatus() => SendCommand(":STATus?");

    public void SetInput(bool on) => SendCommand($":INPut {(on ? "ON" : "OFF")}", false);
    public bool GetInput() => SendCommand($":INPut?").Trim().Equals("ON");

    public void SetVoltage(double value) => SendCommand($":VOLTage {value}V", false);

    public double GetVoltage() => ExtractVoltage(SendCommand(":VOLTage?"));

    public double GetVoltageUpper() => ExtractVoltage(SendCommand(":VOLTage:UPPer?"));

    public double GetVoltageLower() => ExtractVoltage(SendCommand(":VOLTage:LOWer?"));

    public void SetCurrent(double value) => SendCommand($":CURRent {value/1000.0f}A", false);

    public double GetCurrent() => ExtractCurrent(SendCommand(":CURRent?"));

    public double GetCurrentUpper() => ExtractCurrent(SendCommand(":CURRent:UPPer?"));

    public double GetCurrentLower() => ExtractCurrent(SendCommand(":CURRent:LOWer?"));

    public void SetResistance(double ohms) => SendCommand($":RESistance {ohms}OHM", false);

    public double GetResistance() => ExtractResistance(SendCommand(":RESistance?"));

    public double GetResistanceUpper() => ExtractResistance(SendCommand(":RESistance:UPPer?"));

    public double GetResistanceLower() => ExtractResistance(SendCommand(":RESistance:LOWer?"));

    public void SetPower(double watts) => SendCommand($":POWer {watts}W", false);

    public double GetPower() => ExtractPower(SendCommand(":POWer?"));

    public double GetPowerUpper() => ExtractPower(SendCommand(":POWer:UPPer?"));

    public double GetPowerLower() => ExtractPower(SendCommand(":POWer:LOWer?"));

    public void SetFunctionMode(FunctionMode mode) => SendCommand($":FUNCtion {mode.ToString().ToUpperInvariant()}", false);

    public FunctionMode GetFunctionMode() {
        string mode = SendCommand(":FUNCtion?");
        if (Enum.TryParse(mode.Trim(), ignoreCase: true, out FunctionMode result)) {
            return result;
        } else {
            throw new Exception("Faction Mode is Unknown");
        }

    }

    public double MeasureCurrent() => ExtractCurrent(SendCommand(":MEASure:CURRent?"));

    public double MeasureVoltage() => ExtractVoltage(SendCommand(":MEASure:VOLTage?"));

    public double MeasurePower() => ExtractPower(SendCommand(":MEASure:POWer?"));

    public void Dispose() {
        if (SerialPort != null) {
            if (SerialPort.IsOpen)
                SerialPort.Close();
            SerialPort.Dispose();
            SerialPort = null;
        }
    }

    public void SetPort(string newValue, int baudRate = 115200) {
        _logger.LogInformation($"Setting port to {newValue} with baud rate {baudRate}");
        if (SerialPort != null) {
            SerialPort.Close();
            SerialPort.Dispose();
        }
        SerialPort = new SerialPort(newValue, baudRate) {
            Parity = Parity.None,
            StopBits = StopBits.One,
            DataBits = 8,
            Handshake = Handshake.None,
            NewLine = "\n",
            ReadTimeout = 1000,
            WriteTimeout = 1000
        };
        _logger.LogInformation($"Opening port {newValue}");
        SerialPort.Open();
    }

    public override Task<float> Mesure<TEnum>(TEnum Type)  {
        if (IsInEmulationMode) {
            _logger.LogWarning($"Emulation Mode: Returning default value for {Type}.");
            return Task.FromResult(AmicellUtil.Util.RandomFloat()); // Return a default value in emulation mode
        }
        if (Type is MeasurementType measurementType) {
            switch (measurementType) {
                case MeasurementType.Voltage:
                    return Task.FromResult((float)MeasureVoltage());
                case MeasurementType.Current:
                    return Task.FromResult((float)MeasureCurrent());
                case MeasurementType.Resistance:
                    return Task.FromResult((float)GetResistance());
                case MeasurementType.Power:
                    return Task.FromResult((float)MeasurePower());
                default:
                    throw new ArgumentOutOfRangeException(nameof(Type), $"Unsupported measurement type: {measurementType}");
            }
        } else {
          throw RaiseException(new ArgumentException($"Invalid measurement type: {Type}", nameof(Type)));
        }

    }
}
