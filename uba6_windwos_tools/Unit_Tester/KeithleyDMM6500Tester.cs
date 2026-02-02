using BK_PRECISION9104Libary;
using KeithleyDMM6500Library;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UBA6Library;

namespace Unit_Tester {


    [TestClass]
    public class KeithleyDMM6500Tester {
        private static KeithleyDMM6500 multimeter;
        private static TestContext _testContext;
        public TestContext TestContext { get; set; }

        [ClassInitialize]
        public static void Init(TestContext context) {
            try {
                _testContext = context;
                using var loggerFactory = LoggerFactory.Create(builder => {
                    builder.AddConsole(); // Add console logging
                });
                // Create an ILogger<Calibration.Calibration> instance            
                ILogger<KeithleyDMM6500> MM_logger = loggerFactory.CreateLogger<KeithleyDMM6500>();
                string name = context.Properties["MultimeterName"]?.ToString() ?? "COM1"; // Default fallback
                multimeter = new KeithleyDMM6500(MM_logger, name);
            } catch (Exception ex) {
                Console.WriteLine($"Initialization failed: {ex}");
                throw; // Let MSTest handle it as a test failure
            }
        }

        [TestMethod]
        public void VoltageReadTest() {
            multimeter.Mesure(KeithleyDMM6500.MeasurementType.Voltage);
        }
        [TestMethod]
        public void CurrentReadTest() {
            multimeter.Mesure(KeithleyDMM6500.MeasurementType.Current);            
        }
        [TestMethod]
        public void CheckConnectionTest() {
            Assert.IsTrue(multimeter.ChecKConnection(), "Connction Test Failed");
            Console.WriteLine($"{multimeter}");
        }
    }
}
