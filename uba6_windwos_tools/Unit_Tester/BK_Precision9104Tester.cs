using BK_PRECISION9104Libary;
using Microsoft.Extensions.Logging;
using static BK_PRECISION9104Libary.BK_PRECISION9104;

namespace Unit_Tester {
    [TestClass]
    public class SanityTest {

        private static TestContext _testContext;
        public TestContext TestContext { get; set; }

        [ClassInitialize]
        public static void Init(TestContext context) {
            _testContext = context;
        }

        [TestMethod]
        public async Task MyTest() {
            await Task.Delay(10);
            Assert.AreEqual(1, 1);
        }
    }
    [TestClass]
    public class BK_Precision9104Tester {

        public static BK_PRECISION9104 bk9104 ;
        private static TestContext _testContext;
        public TestContext TestContext { get; set; }
        

        [ClassInitialize]
        public static void Init(TestContext context) {
            _testContext = context;
            try {
                var com = context.Properties["PS_COM"]?.ToString() ?? "COM1"; // Default fallback
                var loggerFactory = LoggerFactory.Create(builder =>
                {
                    builder.AddConsole();
                    builder.SetMinimumLevel(LogLevel.Debug);
                });

                ILogger<BK_PRECISION9104> logger = loggerFactory.CreateLogger<BK_PRECISION9104>();
                bk9104 = new BK_PRECISION9104(logger, com);
            } catch (Exception ex) {
                Console.WriteLine($"Initialization failed: {ex}");
                throw; // Let MSTest handle it as a test failure
            }
        }

        [ClassCleanup]
        public static void Cleanup() {
            bk9104.SetOutput(false);
        }

        [TestMethod]
        public async Task BK_Precision9104Test() {
            await bk9104.Getinformation();
            Console.WriteLine(bk9104.ToString());

        }

        [TestMethod]
        public async Task BK_PrecisionOutputTest() {
            await bk9104.Getinformation();
            Console.WriteLine(bk9104.ToString());
            await bk9104.GetOutput();
            await bk9104.SetOutput(!bk9104.IsOutput);
            await bk9104.GetOutput();
        }



        [TestMethod]
        public async Task BK_PrecisionVoltageSetTest() {
            await bk9104.Getinformation();
            Console.WriteLine(bk9104.ToString());
            await bk9104.SetOutputVoltage(ABC_PRESET.B, 2000);
            await bk9104.Getinformation();
        }


        [TestMethod]
        public async Task BK_PrecisionSetPreset() {
            await bk9104.Getinformation();
            Console.WriteLine(bk9104.ToString());
            await bk9104.SetABC_Select(ABC_PRESET.C);
            await bk9104.GetPresetVoltageAndCuurent(ABC_PRESET.C);
        }
        [TestMethod]
        public async Task BK_configPreset() {
            try {
                int voltage2set = 1000; // in mV
                int current2set = 1000; // in mV                
                await bk9104.SetPresetVoltageAndCuurent(ABC_PRESET.A, voltage2set, current2set);
                //await Task.Delay(10000);
                await bk9104.GetPresetVoltageAndCuurent(ABC_PRESET.A);

            } catch (Exception ex) {
                Console.WriteLine(ex);
                Assert.Fail();
            }
        }

        [TestMethod]
        public async Task BK_GetInfoTest() {
            await bk9104.Getinformation();
        }
    }
}
