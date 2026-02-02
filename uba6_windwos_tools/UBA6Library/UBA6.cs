using AmicellUtil;
using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using UBA_MSG;
using UBA_PROTO_BPT;
using UBA_PROTO_CALIBRATION;
using UBA_PROTO_CMD;
using UBA_PROTO_FM;
using UBA_PROTO_QUERY;
using UBA_PROTO_TR;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;
using static System.Runtime.InteropServices.JavaScript.JSType;
using static UBA6Library.UBA_Interface;

namespace UBA6Library {
    public partial class UBA6 : AmicellDevice<UBA6> {        
        private readonly MemoryStream _buffer = new MemoryStream();
        private int messageSize = 0;
        public event EventHandler<ProtoMessageEventArg>? MessageReceived;
        public Channel A { get; set; } = new Channel(UBA_PROTO_CHANNEL.ID.A);
        public Channel B = new Channel(UBA_PROTO_CHANNEL.ID.B);
        public Channel AB = new Channel(UBA_PROTO_CHANNEL.ID.Ab);
        public int VPS = 0;
        
        public LineCalibrationData CalDataLineA = new LineCalibrationData();
        public LineCalibrationData CalDataLineB = new LineCalibrationData();
        public UBA_Interface UBA_Interface { get; private set; } = null!; // Initialized in the constructor

        public string SerialNumber { get; set; } = "0000000000"; // Default serial number, can be changed later
        public UInt32 Address { get; set; } = 0x80000000; // Default address, can be changed later

        public UBA6(ILogger<UBA6> logger, UBA_Interface intrefece) : base(logger) {
            UBA_Interface = intrefece;
            UBA_Interface.MessageReceived += UBA_Interface_MessageReceived;

        }        

        public UBA6(ILogger<UBA6> logger, UBA_Interface com, string sn) : this(logger, com) {
            this.SerialNumber = sn;
        }
        public void SetIntreface(UBA_Interface uBA_Interface) {
            this.UBA_Interface = uBA_Interface;
        }
        private void UBA_Interface_MessageReceived(object? sender, ProtoMessageEventArg e) {

            if (e.Msg.Head?.SenderAddress == Address) { 
                _logger.LogDebug($"UBA6 received message for address {Address}: {e.Msg.PyloadCase}");
                MessageReceived?.Invoke(this, e);
            } else {
                _logger.LogDebug($"UBA6 received message for different address: {e.Msg?.Head?.TargetAddress}, expected: {Address}");
            }
        }       
      

        public async Task UpdatedTime() {
            DateTime localTime = DateTime.Now;
            TimeSpan offset = TimeZoneInfo.Local.GetUtcOffset(localTime);
            UInt32 newtime = (uint)new DateTimeOffset(localTime + offset).ToUnixTimeSeconds();
            UBA_PROTO_UBA6.command cmd = ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Time, newtime);
            SentMessage(UBA_Message_Factory.CreateMessage(Address,cmd));
        }
        
        public void SentMessage(Message msg, MessagePriority priority = MessagePriority.DEFUALT) {
            msg.Head.TargetAddress = this.Address;
            UBA_Interface.EnqueueMessage(msg, priority);
        }
        public async Task SentMessageAsync(Message msg, MessagePriority priority = MessagePriority.DEFUALT) {
            msg.Head.TargetAddress = this.Address;
            await UBA_Interface.EnqueueMessageAndWaitForResponseAsync(msg, priority);
        }
        public void DisableCalibration(UBA_PROTO_LINE.ID line_id){ 
            UBA_PROTO_LINE.command cmd = ProtoHelper.CreateLineCommand(UBA_PROTO_LINE.CMD_ID.Calibration, line_id, state:0);
            SentMessage(UBA_Message_Factory.CreateMessage(this.Address, cmd));
        }
        public void boot() {
            SentMessage(UBA_Message_Factory.CreateMessage(this.Address,
                ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Boot)));
        }

        public void StopBPT(UBA_PROTO_CHANNEL.ID ch) {
            SentMessage(UBA_Message_Factory.CreateMessage(this.Address,
                ProtoHelper.CreateBPTCommand(UBA_PROTO_BPT.CMD_ID.Stop, ch)), MessagePriority.BPT_STOP);
        }
        public void StartBPT(UBA_PROTO_CHANNEL.ID ch,UInt32 index) {
            SentMessage(UBA_Message_Factory.CreateMessage(this.Address,
                ProtoHelper.CreateBPTCommand(UBA_PROTO_BPT.CMD_ID.Select, ch,index)), MessagePriority.BPT_START);
        }
        public void PasueBPT(UBA_PROTO_CHANNEL.ID ch) {
            SentMessage(UBA_Message_Factory.CreateMessage(this.Address,
                ProtoHelper.CreateBPTCommand(UBA_PROTO_BPT.CMD_ID.Paused, ch)), MessagePriority.BPT_PAUSE);
        }
        public void ClearBPT(UBA_PROTO_CHANNEL.ID ch) {
            SentMessage(UBA_Message_Factory.CreateMessage(this.Address,
                ProtoHelper.CreateBPTCommand(UBA_PROTO_BPT.CMD_ID.Clear, ch)));
        }



        public async Task<Message> GetMessage(RECIPIENT r = RECIPIENT.Device) {
           return await UBA_Interface.GetMessage(r,this.Address);
        }

        public async Task<byte[]> FeatchFileToByteArray(string filename = "Channel A_20250903130401_PC_Delay.pb") {
            if (System.IO.File.Exists(filename)) {
                _logger.LogDebug($"File '{filename}' exists locally. Returning as byte array.");
                return await System.IO.File.ReadAllBytesAsync(filename);
            }
            List<byte> fileData = new List<byte>();
            Message? m;
            UInt32 index = 0;
            UInt32 retry = 5;
            bool done = false;
            uint? totlaSize = uint.MaxValue;

            do {
                m = await UBA_Interface.GetMessage(UBA_Message_Factory.CreateMessage(Address, ProtoHelper.CreateFileCommand(UBA_PROTO_FM.CMD_ID.ChunkRequest, filename, index)),1000);
                if (m?.File?.Data.Length > 0) {
                    _logger.LogDebug($"Received file chunk {index} with size {m.File.Data.Length} bytes.");
                    fileData.AddRange(m.File.Data.ToByteArray());
                    index++;
                    done = false;
                    retry = 5;
                    totlaSize = m?.File.TotalSize;
                } else if (m == null) {
                    _logger.LogWarning($"didnt Received a chunk {index} message.");
                    if (retry-- <= 0) {
                        done = true;
                    } else {
                        done = false;
                    }                        
                } else {
                    _logger.LogWarning($"Received empty file chunk {index}.");
                    done = true;
                }
                if (totlaSize.HasValue && fileData.Count >= totlaSize) {
                    done = true;
                }                
            } while (done ==false);
            byte[] file = fileData.ToArray();
            System.IO.File.WriteAllBytes(filename, file);
            return file;
        }

        public async Task<List<string>> FeatchFileList(uint skip = 0) {
            List<string> files = new List<string>();
            Message? message;
            do {
                 message = await UBA_Interface.GetMessage(UBA_Message_Factory.CreateMessage(Address, ProtoHelper.CreateFileCommand(UBA_PROTO_FM.CMD_ID.FileListRequest, string.Empty, (uint)files.Count)));
                if (message?.FmList == null) {
                    throw RaiseException(new Exception("Failed to fetch file list."));
                }
                foreach (string file in message.FmList.Filenames) {
                    _logger.LogDebug($"Received file {file}.");
                    files.Add(file);
                }
            } while (files.Count< message?.FmList.TotalFiles );
            return files;
        }

        public async Task<string> GetRunningTestFileName(UBA_PROTO_CHANNEL.ID testOnChannel) { 
            string filename = string.Empty;
            Message? message = await UBA_Interface.GetMessage(UBA_Message_Factory.CreateMessage(Address, ProtoHelper.CreateFileCommand(UBA_PROTO_FM.CMD_ID.BptFile, string.Empty, (uint)testOnChannel)));
            if(message?.FmList != null) {
                if (message.FmList.Filenames.Count == 1) {
                    filename = message.FmList.Filenames.First();
                    _logger.LogDebug($"Received running test file name: {filename}.");
                }else {
                    throw RaiseException(new Exception("Unexpected number of running test files received."));
                }
            } else {
                throw RaiseException(new Exception("Failed to fetch running test file name."));
            }
            return filename;
        }

        public override async Task<float> Mesure<TEnum>(TEnum Type) {
            _logger.LogDebug($"Requesting measurement for type: {Type}");
            if (IsInEmulationMode) {
                _logger.LogWarning($"Emulation Mode: Returning default value for {Type}.");
                return AmicellUtil.Util.RandomFloat(); // Return a default value in emulation mode
            }
            RECIPIENT Recipient = (RECIPIENT)(Convert.ToUInt32(Type) & Convert.ToUInt32(MeasurementType.RECIPIENT));
            MeasurementType MType = (MeasurementType)(Convert.ToUInt32(Type) & Convert.ToUInt32(MeasurementType.Type));
            Message m = await GetMessage(Recipient);
            NotSupportedException ex = null;
            switch (MType) {
                case MeasurementType.VPS:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) { 
                        return m.QueryResponse.Line.Data.Vps;
                    } else if ((Recipient & (RECIPIENT.ChannelA | RECIPIENT.ChannelB | RECIPIENT.ChannelAb)) > 0) {
                        return m.QueryResponse.Channel.Data.Voltage;
                    }
                    break;
                case MeasurementType.BAT_Voltage:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.Voltage;
                    } else if ((Recipient & (RECIPIENT.ChannelA | RECIPIENT.ChannelB | RECIPIENT.ChannelAb)) > 0) {
                        return m.QueryResponse.Channel.Data.Voltage;
                    }
                    break;
                case MeasurementType.GEN_Voltage:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.GenVoltage;
                    }
                    break;
                case MeasurementType.ChargeCurrent:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.ChargeCurrent;
                    } 
                    break;
                case MeasurementType.DischageCurrent:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.DischargeCurrent;
                    } 

                    break;
                case MeasurementType.AMB_Temp:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.AmbTemperature;
                    } else if ((Recipient & (RECIPIENT.ChannelA | RECIPIENT.ChannelB | RECIPIENT.ChannelAb)) > 0) {
                        return m.QueryResponse.Channel.Data.Temperature;
                    }
                    break;
                case MeasurementType.NTC_Temp:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.BatTemperature;
                    } else if ((Recipient & (RECIPIENT.ChannelA | RECIPIENT.ChannelB | RECIPIENT.ChannelAb)) > 0) {
                        return m.QueryResponse.Channel.Data.Temperature;
                    }
                    break;
                case MeasurementType.Capacity:
                    if ((Recipient & (RECIPIENT.LineA | RECIPIENT.LineB)) > 0) {
                        return m.QueryResponse.Line.Data.Capacity;
                    } else if ((Recipient & (RECIPIENT.ChannelA | RECIPIENT.ChannelB | RECIPIENT.ChannelAb)) > 0) {
                        return m.QueryResponse.Channel.Data.Capacity;
                    }
                    break;
                default:
                    ex = new NotSupportedException($"Measurement type {Type} is not supported.");
                    break;

            }
            if (ex == null) {
                ex = new NotSupportedException($"Measurement type {Type} is not supported for recipient {Recipient}.");
            }
            throw RaiseException(ex);
        }
    }
}
