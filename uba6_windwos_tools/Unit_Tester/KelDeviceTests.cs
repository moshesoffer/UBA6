using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unit_Tester {
    [TestClass]
    public class KelDeviceTests {
        private static KelDeviceController _device;

        [ClassInitialize]
        public static void Init(TestContext context) {
            // Adjust the COM port and baud rate to match your setup
            try {
                var loggerFactory = LoggerFactory.Create(builder =>
                {
                    builder.AddConsole();
                    builder.SetMinimumLevel(LogLevel.Debug);
                });
                ILogger<KelDeviceController> logger = loggerFactory.CreateLogger<KelDeviceController>();

                _device = new KelDeviceController(logger,context.Properties["LoadCell_COM"]?.ToString());
            } catch (Exception ex) { 
                Debug.WriteLine(ex);
            }
        }

        [ClassCleanup]
        public static void Cleanup() {
            _device?.Dispose();
        }

        [TestMethod]
        public void GetDeviceId_ShouldReturnValidId() {
            var id = _device.GetDeviceId();
            Assert.IsFalse(string.IsNullOrWhiteSpace(id), "Device ID should not be null or empty.");
        }

        [TestMethod]
        public void SetAndGetBeep_ShouldRespondCorrectly() {
            bool beep2set = true;
            _device.SetBeep(beep2set);
            bool beep = _device.QueryBeep();
            Assert.AreEqual(beep2set, beep, $"Beep should be {(beep2set ? "ON":"OFF")}.");
            beep2set = false;
            _device.SetBeep(beep2set);
            beep = _device.QueryBeep();
            Assert.AreEqual(beep2set, beep, $"Beep should be {(beep2set ? "ON" : "OFF")}.");
        }
        [TestMethod]
        public void SetAndGetInput() {
            bool input2set = true;
            _device.SetInput(input2set);
            bool input = _device.GetInput() ;
            Assert.AreEqual(input, input2set, $"input should be {(input2set? "ON" :"OFF")}");
        }



        [TestMethod]
        public void SetAndGetVoltage_ShouldMatchSetValue() {
            double voltage = 12.5;
            _device.SetVoltage(voltage);
            double response = _device.GetVoltage();
            Assert.AreEqual(response, voltage, "Voltage readback should contain set value.");
        }

        [TestMethod]
        public void SetAndGetCurrent_ShouldMatchSetValue() {
            double current = 1.5;
            _device.SetCurrent(current);
            double response = _device.GetCurrent();
            Assert.AreEqual(response, current, "Current readback should contain set value.");
        }

        [TestMethod]
        public void SetAndGetResistance_ShouldMatchSetValue() {
            double targetResistance = 20.0;
            _device.SetResistance(targetResistance);
            double actualResistance = _device.GetResistance();
            Assert.AreEqual(targetResistance, actualResistance, 0.01, "Resistance should match set value.");
        }

        [TestMethod]
        public void GetResistanceUpper_ShouldReturnPositiveValue() {
            double upper = _device.GetResistanceUpper();
            Assert.IsTrue(upper > 0, "Upper resistance should be greater than zero.");
        }

        [TestMethod]
        public void GetResistanceLower_ShouldReturnNonNegativeValue() {
            double lower = _device.GetResistanceLower();
            Assert.IsTrue(lower >= 0, "Lower resistance should be non-negative.");
        }
        [TestMethod]
        public void SetAndGetPower_ShouldMatchSetValue() {
            double targetPower = 25.0;
            _device.SetPower(targetPower);

            double actualPower = _device.GetPower();
            Assert.AreEqual(targetPower, actualPower, 0.01, "Power should match the value set.");
        }

        [TestMethod]
        public void GetPowerUpper_ShouldReturnPositiveValue() {
            double upper = _device.GetPowerUpper();
            Assert.IsTrue(upper > 0, "Upper power limit should be greater than zero.");
        }

        [TestMethod]
        public void GetPowerLower_ShouldReturnNonNegativeValue() {
            double lower = _device.GetPowerLower();
            Assert.IsTrue(lower >= 0, "Lower power limit should be non-negative.");
        }

        [TestMethod]
        public void MeasureVoltage_ShouldReturnNumericValue() {
            double result = _device.MeasureVoltage();
            Assert.IsTrue(result >= 0, "Voltage should be non-negative.");
        }

        [TestMethod]
        public void MeasureCurrent_ShouldReturnNumericValue() {
            double result = _device.MeasureCurrent();
            Assert.IsTrue(result >= 0, "Current should be non-negative.");
        }
        [TestMethod]
        public void MeasurePower_ShouldReturnNumericValue() {
            double result = _device.MeasurePower();
            Assert.IsTrue(result >= 0, "Power should be non-negative.");
        }
        [TestMethod]
        public void SetAndGetFunctionMode() {
            KelDeviceController.FunctionMode mode2set = KelDeviceController.FunctionMode.CC;
            _device.SetFunctionMode(mode2set);
            KelDeviceController.FunctionMode mode = _device.GetFunctionMode();
            Assert.AreEqual(mode2set, mode, $"Function mode should be {mode2set}.");
        }
    }
}
