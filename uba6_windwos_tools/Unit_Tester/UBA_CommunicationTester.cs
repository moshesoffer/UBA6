using Microsoft.Extensions.Logging;
using UBA_PROTO_QUERY;
using UBA_PROTO_TR;
using UBA6Library;

namespace Unit_Tester {
    [TestClass]
    public class UBA_CommunicationTester {
        private static TestContext _testContext;
        public TestContext TestContext { get; set; }
        public static UBA6 UBA { get; set; }

        [ClassInitialize]
        public static void Init(TestContext context) {
            _testContext = context;
            try {
                // Create a logger factory
                using var loggerFactory = LoggerFactory.Create(builder => {
                    builder.AddConsole();
                    builder.SetMinimumLevel(LogLevel.Debug); // Set log level to Debug
                });
                ILogger<UBA6> ubaLogger = loggerFactory.CreateLogger<UBA6>();
                ILogger<UBA_Interface> intreface_logger = loggerFactory.CreateLogger<UBA_Interface>();


                string com = context.Properties["UBA_COM"]?.ToString() ?? "COM1"; // Default fallback
                UBA_Interface uBA_Interface = new UBA_Interface(intreface_logger, com);
                UBA = new UBA6(ubaLogger, uBA_Interface);
                UBA.Address = 0x00080001;
            } catch (Exception ex) {
                Console.WriteLine($"Initialization failed: {ex}");
                throw; // Let MSTest handle it as a test failure
            }
        }
        [ClassCleanup]
        public static void Cleanup() {
            UBA.UBA_Interface.WaitForQueueToBeEmptyAsync().Wait();
        }

        [TestMethod]
        public void Test_Ping_message() {
            UBA.SentMessage(UBA_Message_Factory.CreateQeuryMessage(UBA.Address, RECIPIENT.Device | RECIPIENT.ChannelB | RECIPIENT.ChannelA | RECIPIENT.ChannelAb | RECIPIENT.LineA | RECIPIENT.LineB));
        }
        [TestMethod]
        public void TestDeviceQueryMessage() {
            UBA.SentMessage(UBA_Message_Factory.CreateQeuryMessage(UBA.Address,RECIPIENT.Device));
        }
        [TestMethod]
        public async Task TestQueryMessage() {
            foreach (RECIPIENT r in Enum.GetValues(typeof(RECIPIENT))) {
                if (r > RECIPIENT.Device) {
                    await UBA.GetMessage(r);                    
                }
            }

        } 
        [TestMethod]
        public void TestUpdateTime() {
            DateTime localTime = DateTime.Now;
            TimeSpan offset = TimeZoneInfo.Local.GetUtcOffset(localTime);
            UBA.UpdatedTime();
            UBA.UBA_Interface.WaitForQueueToBeEmptyAsync().Wait();  
        }

        [TestMethod]
        public void Test_TR_message() {
            Test_Routine test_Routine = new Test_Routine();
            List<object> list = new List<object>();
            UBA_PROTO_BPT.charge step = ProtoHelper.CreateChargeStep(UBA_PROTO_BPT.SOURCE.Internal, 1000, 51000,
                ProtoHelper.CreateChargeStopCondtion(cutOffCurrent: 0,
                                                    limitCapacity: 50000,
                                                    maxTime: 20000,
                                                    maxTemp: 500),
                -20.0f);            
            list.Add(new UBA_PROTO_BPT.charge(step));
            UBA.SentMessage(UBA_Message_Factory.CreateMessage(UBA.Address, ProtoHelper.CreateTR_Message(0, ProtoHelper.CreateTestRoutine(UBA_PROTO_BPT.MODE.SingleChannel, list, "test_charge"))));            
        }

        [TestMethod]
        public void Test_UBA_CMD_Message() {
            UBA.SentMessage(UBA_Message_Factory.CreateMessage(UBA.Address,ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Test)));
            UBA.UBA_Interface.WaitForQueueToBeEmptyAsync().Wait();
        }
        [TestMethod]
        public void Test_ChannelCMD_Message() {
            UBA.SentMessage(UBA_Message_Factory.CreateMessage(UBA.Address, ProtoHelper.CreateChannelCommand(UBA_PROTO_CHANNEL.CMD_ID.Test)));
        }
        [TestMethod]
        public void Test_LineCMD_Message() {
            UBA.SentMessage(UBA_Message_Factory.CreateMessage(UBA.Address, ProtoHelper.CreateLineCommand(UBA_PROTO_LINE.CMD_ID.Test)));
            
        }
        [TestMethod]
        public void Test_BootCommend () {
            UBA.boot();
        }
        [TestMethod]
        public void Test_CalibrationMessage() {
            UBA6.LineCalibrationData calibrationDataA = new UBA6.LineCalibrationData();
            UBA6.LineCalibrationData calibrationDataB = new UBA6.LineCalibrationData();
            UBA.SentMessage(UBA_Message_Factory.CreateMessage(UBA.Address, UBA.CalDataLineA.CreateProtoMessage(), UBA.CalDataLineB.CreateProtoMessage()));            

        }

        [TestMethod]
        public async Task TestQuery1Message() {
            await UBA.GetMessage(RECIPIENT.BptA);
        }

        [TestMethod]
        public async Task TestMessageFactory() {            
            await UBA.GetMessage(RECIPIENT.BptAb);
        }

        [TestMethod]
        public async Task TestFeatchFile() {
            //string filename = "Channel B_20000101012158_PC_Delay.pb";
            string filename = "Channel A_20250903130401_PC_Delay.pb";
            //string filename = "Channel A_20250902071752_PC_Delay.pb";
            byte[]  file = await UBA.FeatchFileToByteArray(filename);
            // create a binary file that cointion the byte array name it the same as the file name
            List<UBA_PROTO_DATA_LOG.data_log>  a= ProtoHelper.DecodeDataLogMessages(file);
            Console.WriteLine(a);
            System.IO.File.WriteAllBytes(filename, file);

        }
        // Example usage in your test method
        [TestMethod]
        public async Task TestDecodeDataLogMessages() {
            string filePath = "C:\\Users\\ORA\\source\\repos\\UBA6\\Unit_Tester\\bin\\Debug\\net8.0\\Channel A_20250903130401_PC_Delay.pb";
            byte[] file = await File.ReadAllBytesAsync(filePath);
            var logs = ProtoHelper.DecodeDataLogMessages(file);
            Assert.IsTrue(logs.Count > 0);
        }

        [TestMethod]
        public async Task TestFetchFilesList() {
            var files = await UBA.FeatchFileList();
            Assert.IsTrue(files.Count > 0);
        }
        [TestMethod]
        public async Task TestStartBPT() {
            UBA.StartBPT(UBA_PROTO_CHANNEL.ID.B,0);
            UBA.UBA_Interface.WaitForQueueToBeEmptyAsync().Wait();
            UBA.StartBPT(UBA_PROTO_CHANNEL.ID.B, 0);
            Task.Delay(10000).Wait();

        }
        [TestMethod]
        public async Task TestGetRunningTestFileName() {
            string fileNAme =  await UBA.GetRunningTestFileName(UBA_PROTO_CHANNEL.ID.A);
            Assert.IsNotNull(fileNAme);
        }
    }

}