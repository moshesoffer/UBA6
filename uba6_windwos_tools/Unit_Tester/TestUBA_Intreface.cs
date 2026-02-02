using Google.Protobuf;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using UBA_MSG;
using UBA_PROTO_QUERY;
using UBA_PROTO_TR;
using UBA6Library;

namespace Unit_Tester {
    [TestClass]
    public class TestUBA_Intreface {
        private static TestContext _testContext;
        private static readonly UInt32 UBA_Address = UInt32.MaxValue;
        public static UBA_Interface UBA_Interface { get; private set; }

        [ClassInitialize]
        public static void Init(TestContext context) {
            _testContext = context;
            try {
                // Create a logger factory
                using var loggerFactory = LoggerFactory.Create(builder => {
                    builder.AddConsole();
                    builder.SetMinimumLevel(LogLevel.Debug); // Set log level to Debug
                });
                ILogger<UBA_Interface> logger = loggerFactory.CreateLogger<UBA_Interface>();
                string com = context.Properties["UBA_COM"]?.ToString() ?? "COM1"; // Default fallback
                UBA_Interface = new UBA_Interface(logger, com);
                UBA_Interface.StartProcessing();                
            } catch (Exception ex) {
                Console.WriteLine($"Initialization failed: {ex}");

                throw; // Let MSTest handle it as a test failure
            }        
        }
        [TestMethod]
        public async Task TestLongMessage() {
            Test_Routine test_Routine = new Test_Routine();
            List<object> list = new List<object>();
            UBA_PROTO_BPT.charge step = ProtoHelper.CreateChargeStep(UBA_PROTO_BPT.SOURCE.Internal, 1000, 51000,
                ProtoHelper.CreateChargeStopCondtion(cutOffCurrent: 0,
                                                    limitCapacity: 50000,
                                                    maxTime: 20000,
                                                    maxTemp: 500),
                -20.0f);
            list.Add(new UBA_PROTO_BPT.charge(step));
            list.Add(new UBA_PROTO_BPT.charge(step));
            list.Add(new UBA_PROTO_BPT.charge(step));
            list.Add(new UBA_PROTO_BPT.charge(step));
            list.Add(new UBA_PROTO_BPT.charge(step));
            list.Add(new UBA_PROTO_BPT.charge(step));
            UBA_Interface.EnqueueMessage(UBA_Message_Factory.CreateMessage(UBA_Address, ProtoHelper.CreateTR_Message(0, ProtoHelper.CreateTestRoutine(UBA_PROTO_BPT.MODE.SingleChannel, list, "Long_test"))));
        }

        [TestMethod]
        public async Task TestAllQueryMessage() {
            foreach (RECIPIENT r in Enum.GetValues(typeof(RECIPIENT))) {
                if (r > RECIPIENT.Device) {
                    await UBA_Interface.GetMessage(r);
                    Task.Delay(4000).Wait();
                }
            }           
        }
        [TestMethod]
        public async Task TestQueryMessage() {
            await UBA_Interface.GetMessage(RECIPIENT.LineA);
            
        }
        public static byte[] HyphenHexToByteArray(string hex) {
            string[] hexValues = hex.Split('-', StringSplitOptions.RemoveEmptyEntries);
            byte[] result = new byte[hexValues.Length];

            for (int i = 0; i < hexValues.Length; i++) {
                result[i] = Convert.ToByte(hexValues[i], 16);
            }

            return result;
        }

        [TestMethod]
        public async Task TestHexMessage() {
            string hex = "51-0A-02-08-20-2A-4B-08-80-01-10-07-32-44-10-01-18-80-A7-8C-C2-03-30-02-3A-38-08-01-10-02-18-01-22-06-08-87-A1-01-18-04-2A-26-08-02-10-01-22-10-08-87-A1-01-10-EA-20-25-00-00-C8-41-30-04-40-01-2A-0E-08-EA-14-10-89-02-18-F6-0F-20-B1-1D-30-02-2A-00-";
            byte[] bytes = HyphenHexToByteArray(hex);

            using var stream = new MemoryStream(bytes);

            int messageSize = (int) ProtoHelper.DecodeVarint(stream);
            byte[] buffer = new byte[messageSize];// create buffer with size of message
            int bytesRead = stream.Read(buffer, 0, messageSize); // read the message from the stream
            if (bytesRead == messageSize) { // check if we read the full message
                var parser = new MessageParser<Message>(() => new Message());
                Message message = parser.ParseFrom(buffer, 0, messageSize);
                Console.WriteLine($"Parsed Message: {message}");
            } else { 
                Assert.Fail("Failed to read the full message from the stream.");    
            }
        }


        [TestMethod]
        public async Task TestGetChank() {
            string filename = "Channel B_20000101012158_PC_Delay.pb";
            await UBA_Interface.GetMessage(UBA_Message_Factory.CreateMessage(UBA_Address, ProtoHelper.CreateFileCommand(UBA_PROTO_FM.CMD_ID.ChunkRequest, filename, 0)));
            UBA_PROTO_FM.file_list fl = new UBA_PROTO_FM.file_list();
            
        }
        [TestMethod]
        public async Task TestGetFileList() {
            Message message = await UBA_Interface.GetMessage(UBA_Message_Factory.CreateMessage(UBA_Address, ProtoHelper.CreateFileCommand(UBA_PROTO_FM.CMD_ID.FileListRequest, string.Empty, 0)));
            foreach (var file in message.FmList.Filenames) {
                Console.WriteLine($"File: {file}");
            }


        }

    }
}
