using BK_PRECISION9104Libary;
using KeithleyDMM6500Library;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using UBA6Library;

namespace Unit_Tester {
    [TestClass]
    public class CalibrationTester {
        public static Calibration.Calibration Cal { get; set; }


        private static TestContext _testContext;
        public TestContext TestContext { get; set; }

        [ClassInitialize]
        public static void Init(TestContext context) {
            _testContext = context;
            try {
                // Create a logger factory
                using var loggerFactory = LoggerFactory.Create(builder =>
                {
                    builder.AddConsole(); // Add console logging
                });

                // Create an ILogger<Calibration.Calibration> instance
                ILogger<Calibration.Calibration> logger = loggerFactory.CreateLogger<Calibration.Calibration>();
                ILogger<UBA6> ubaLogger = loggerFactory.CreateLogger<UBA6>();
                ILogger<BK_PRECISION9104> BK_logger = loggerFactory.CreateLogger<BK_PRECISION9104>();
                ILogger<KelDeviceController> Kel_logger = loggerFactory.CreateLogger<KelDeviceController>();
                ILogger<KeithleyDMM6500> MM_logger = loggerFactory.CreateLogger<KeithleyDMM6500>();
                ILogger<UBA_Interface> intreface_logger = loggerFactory.CreateLogger<UBA_Interface>();

                string com = context.Properties["UBA_COM"]?.ToString() ?? "COM1"; // Default fallback
                UBA_Interface uBA_Interface = new UBA_Interface(intreface_logger, com);
                UBA6 UBA = new UBA6(ubaLogger, uBA_Interface);
                com = context.Properties["PS_COM"]?.ToString() ?? "COM1"; // Default fallback
                BK_PRECISION9104 ps = new BK_PRECISION9104(BK_logger,com);
                com  = context.Properties["MultimeterName"]?.ToString() ?? "COM1"; // Default fallback
                KeithleyDMM6500 mm= new KeithleyDMM6500(MM_logger,com);
                com = context.Properties["LoadCell_COM"]?.ToString() ?? "COM1"; // Default fallback
                KelDeviceController lc = new KelDeviceController(Kel_logger, com);
                Cal = new Calibration.Calibration(logger,UBA, ps,mm,lc);
                com = context.Properties["Peripheral_Emulation"]?.ToString() ?? "FALSE"; // Default fallback
                
                if (com.Equals("TRUE")){
                    Cal.IsInEmulationMode = true; // Enable emulation mode
                } else { 
                    Cal.Init().Wait();
                    lc.SetFunctionMode(KelDeviceController.FunctionMode.CC);
                }
            } catch (Exception ex) {
                Console.WriteLine($"Initialization failed: {ex}");
                throw; // Let MSTest handle it as a test failure
            }
        }
        [ClassCleanup]
        public static  void Cleanup() {
            Cal.PowerSupply.SetOutput(false);
        }
        [TestMethod]
        public async Task Test_init() {
            try {
                await Cal.Init();
            } catch (Exception ex) {                 
                Assert.Fail(ex.Message);
            }
        }
        [TestMethod]
        public async Task TestVPS_Cal() {
            try {
                await Cal.VPS_Calibration(UBA_PROTO_LINE.ID.A);
                await Cal.VPS_Calibration(UBA_PROTO_LINE.ID.B);
            }catch (Exception ex) { 
                Assert.Fail(ex.Message);
            }
        }
       
        [TestMethod]
        public async Task TestBatCal() {
            try {
                await Cal.BatteryCalibration(UBA_PROTO_LINE.ID.A);
            } catch (Exception ex) {
                Assert.Fail(ex.Message);
        }
            }
        [TestMethod]
        public async Task TestVgenCal() {
            try {
                await Cal.GenVoltage_Calibration(UBA_PROTO_LINE.ID.A);
                await Cal.GenVoltage_Calibration(UBA_PROTO_LINE.ID.B);
            } catch (Exception ex) { 
                Assert.Fail(ex.Message);
            }
        }
        [TestMethod]
        public async Task TsetBAtteryTempCal() {
            await Cal.BatteryTempCalibration(UBA_PROTO_LINE.ID.A);
            await Cal.BatteryTempCalibration(UBA_PROTO_LINE.ID.B);
        }
        [TestMethod]
        public async Task TsetambientTempCal() {
            await Cal.AmbiantTempCalibration(UBA_PROTO_LINE.ID.A);
            await Cal.AmbiantTempCalibration(UBA_PROTO_LINE.ID.B);
        }
        [TestMethod]
        public async Task TestChargeCalibration() {
            await Cal.ChargeCuurentCalibration(UBA_PROTO_LINE.ID.A);
        }
        [TestMethod]
        public async Task TsetCalAllVoltages() { 
            await Cal.CalibrateAllVoltages(Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V , Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V);
        }
    }   
}
