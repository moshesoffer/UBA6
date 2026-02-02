using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using UBA_MSG;
using UBA_PROTO_QUERY;



namespace UBA6Library {
    public class UBA_Interface {
        protected readonly ILogger<UBA_Interface> _logger;
        private readonly int MAX_PORT_READ_RETRIES = 10;
        private SerialPort? sp { get; set; }
        private PriorityQueue<Message, int> messageQueue = new();
        private CancellationTokenSource? _cts;
        private Task? _processingTask;
        public event EventHandler<ProtoMessageEventArg>? MessageReceived;
        private int messageSize = 0;
        protected static UInt32 messageId = 0;
        private int failes { get; set; } = 0; // the number of failed to open the port 
        public string PortName => sp?.PortName ?? "Not Connected";

        public enum MessagePriority : int {
            BPT_STOP = 1,
            TEST_ROUTINE = 2,
            BPT_START = 3,
            BPT_PAUSE = 3,
            DEVICE_QUERY = 4,
            BPT_QUERY = 5,
            QUERY_MESSAGE = 6,
            FILE_NAME_REQUEST = 7,
            FILE_DATA_REQUEST = 8,
            DEFUALT = 10,

        }
        public UBA_Interface(ILogger<UBA_Interface> logger) {
            _logger = logger;
        }

        public UBA_Interface(ILogger<UBA_Interface> logger, string portName, int baudRate = 115200) : this(logger) {
            sp = new SerialPort(portName, baudRate);
            sp.ReadBufferSize = 4096;
            sp.Parity = Parity.None;
            sp.ReadTimeout = 5000;
            sp.WriteTimeout = 5000;
            sp.DataReceived += SerialPort_DataReceived;
            _logger.LogDebug($"Initializing UBA_Interface with COM port: {portName}");
            try {
                sp.Open();
            } catch (Exception ex) {
                _logger.LogError($"Failed to open COM port {portName}: {ex.Message}");
            }
            StartProcessing();
        }
        public void SwitchCom(string newComPort, bool overwite = false) {
            if (string.IsNullOrEmpty(newComPort)) {
                _logger.LogError("Cannot switch to an empty COM port.");
                return;
            }
            if (overwite == false) {
                if (sp != null && sp.PortName.Equals(newComPort)) {
                    _logger.LogDebug($"Already connected to {newComPort}, no need to switch.");
                    return;
                }
            }
            if (sp != null ) {
                _logger.LogDebug($"Switching COM port from {sp.PortName} to {newComPort}");
                    sp.DataReceived -= SerialPort_DataReceived;
                if (sp.IsOpen) { 
                    sp.Close();
                }
                sp.Dispose();
            }
            sp = new SerialPort(newComPort, 115200);
            sp.Parity = Parity.None;
            sp.ReadTimeout = 5000;
            sp.WriteTimeout = 5000;
            sp.DataReceived += SerialPort_DataReceived;
            try {
                sp.Open();
                _logger.LogDebug($"COM port switched to {sp.PortName}");
            } catch (Exception ex) {
                _logger.LogError($"Failed to open COM port {sp.PortName}: {ex.Message}");
            } finally {
                failes = 0; 
            }
        }

        public void EnqueueMessage(Message message, MessagePriority priority = MessagePriority.DEFUALT) {
            if (message == null) {
                throw new ArgumentNullException(nameof(message));
            }
            _logger.LogDebug($"Enqueuing message: {message} with priority {priority}");
            messageQueue.Enqueue(message, (int)priority);
        }

        public void StartProcessing() {
            if (_processingTask != null && !_processingTask.IsCompleted) {
                _logger.LogDebug("Processing task is already running, not starting a new one.");
                return;
            }
            _cts = new CancellationTokenSource();
            _processingTask = Task.Run(() => ProcessQueueAsync(_cts.Token));
        }

        public void StopProcessing() {
            _cts?.Cancel();
            _processingTask?.Wait();
        }

        public static byte[] EncodeVarint(ulong value) {
            var buffer = new List<byte>();
            while (value >= 0x80) {
                buffer.Add((byte)((value & 0x7F) | 0x80)); // Set MSB to 1
                value >>= 7;
            }
            buffer.Add((byte)value); // Last byte, MSB = 0
            return buffer.ToArray();
        }

        private ulong DecodeVarint(Stream stream) {
            ulong result = 0;
            int shift = 0;
            while (true) {
                int b = stream.ReadByte();
                if (b == -1) {
                    _logger.LogError("End of stream reached while decoding Varint");
                    throw new EndOfStreamException("Unexpected EOF during Varint decoding");
                }
                result |= ((ulong)(b & 0x7F)) << shift;
                if ((b & 0x80) == 0)
                    break;

                shift += 7;
                if (shift > 64) throw new InvalidDataException("Varint too long");
            }
            _logger.LogDebug($"Decoded Varint: {result}");
            return result;
        }

        private readonly object _serialReadLock = new object();

        private async void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e) {

            byte[] buffer = new byte[0];
            try {
                SerialPort sp = sender as SerialPort;
                if (sp.BytesToRead == 0) {
                    return;
                }
                if (messageSize == 0) {
                    ulong messageLength = DecodeVarint(sp.BaseStream);
                    _logger.LogDebug($"Message Length: {messageLength}");
                    messageSize = (int)messageLength;
                }
                if (sp.BytesToRead < messageSize) {
                    int delay = 50 + ((messageSize - sp.BytesToRead) / 10);
                    _logger.LogWarning(delay > 100 ? $"Large delay {delay} ms for message size {messageSize} bytes" : $"Waiting {delay} ms for full message of size {messageSize} bytes");
                    await Task.Delay(delay); // Wait a bit to ensure the message is fully received
                }
                lock (_serialReadLock) {
                    buffer = new byte[messageSize];// create buffer with size of message
                    int bytesRead = sp.BaseStream.Read(buffer, 0, messageSize); // read the message from the stream
                    if (bytesRead == messageSize) { // check if we read the full message
                        var parser = new MessageParser<Message>(() => new Message());
                        Message message = parser.ParseFrom(buffer, 0, messageSize);
                        _logger.LogDebug($"new message recevied: {message}");
                        MessageReceived?.Invoke(this, new ProtoMessageEventArg(message));
                    } else {
                        _logger.LogError($"Buffer length({bytesRead}) != Message Size({messageSize})");
                        throw new Exception($"Buffer length({bytesRead})!= Message Size({messageSize})");
                    }
                }
            } catch (Exception ex) {
                _logger.LogDebug($"Buffer (hex): {BitConverter.ToString(buffer)} - Exception {ex}");
                _logger.LogInformation($"Error reading from serial: {ex.Message} , Resetting the serial port");
                sp.DiscardInBuffer();
            } finally {
                messageSize = 0;
            }
        }


        private List<byte> message2byteArry(Message? msg = null) {
            if (msg == null) {
                msg = new Message();
            }
            List<byte> retByteList = [.. EncodeVarint((ulong)msg.CalculateSize()), .. msg.ToByteArray()];
            _logger.LogDebug($"Pacekt Size: {retByteList.Count} Message Size:{msg.CalculateSize()} bytes");
            return retByteList;

        }
        private async Task ProcessQueueAsync(CancellationToken cancellationToken) {
            while (!cancellationToken.IsCancellationRequested) {
                Message? msg = null;
                lock (messageQueue) {
                    if (messageQueue.Count > 0)
                        _logger.LogDebug($"Processing message queue, count: {messageQueue.Count}");
                    messageQueue.TryDequeue(out msg, out _);
                }
                if (msg != null) {
                    if (Monitor.TryEnter(_serialReadLock)) {
                        try {
                            if (sp?.IsOpen == false) {
                                sp.Open();
                            }
                            msg.Head.SenderAddress = 0;
                            byte[] byteMessage = message2byteArry(msg).ToArray();
                            sp?.Write(byteMessage, 0, byteMessage.Length);
                            _logger.LogDebug($"Sent message: {msg}\nSize:{byteMessage[0]} {BitConverter.ToString(byteMessage)}");

                        } catch (Exception) {
                            _logger.LogError($"Failed to send message: {msg}");
                            if ((sp?.IsOpen == false) && (failes++ > MAX_PORT_READ_RETRIES)) { 
                                _logger.LogInformation("Serial port is closed, attempting to reopen.");
                                this.SwitchCom(sp.PortName, true);
                            }
                        } finally {
                            Monitor.Exit(_serialReadLock);
                        }
                    } else {
                        messageQueue.Enqueue(msg, 1);
                        _logger.LogWarning("Serial port is busy reading, skipping write for this cycle.");
                    }
                } else {
                }
                await Task.Delay(50, cancellationToken); // Avoid busy-waiting
            }
        }
        /// <summary>
        /// Waits asynchronously until the message queue is empty or the cancellation token is triggered.
        /// </summary>
        /// <param name="cancellationToken">A cancellation token to observe while waiting.</param>
        /// <returns>A Task that completes when the queue is empty or cancellation is requested.</returns>
        public async Task WaitForQueueToBeEmptyAsync(CancellationToken cancellationToken = default) {
            while (true) {
                lock (messageQueue) {
                    if (messageQueue.Count == 0)
                        break;
                }
                await Task.Delay(50, cancellationToken);
                if (cancellationToken.IsCancellationRequested)
                    break;
            }
        }

        private bool checkQueryMessage(Message queryMessage, Message responseMessage) {
            _logger.LogDebug($"Checking Response message {responseMessage} for sent message {queryMessage}");
            bool ret = false;

            if (queryMessage == null) {
                return false;
            } else {
                if (queryMessage.PyloadCase != Message.PyloadOneofCase.Query) {
                    _logger.LogError(queryMessage.PyloadCase + " is not a Query message");
                } else if (responseMessage.PyloadCase != Message.PyloadOneofCase.QueryResponse) {
                    _logger.LogError(responseMessage.PyloadCase + " is not a QueryResponse message");
                } else if ((queryMessage.Query.Recipient & responseMessage.QueryResponse.Recipient) != responseMessage.QueryResponse.Recipient) {
                    _logger.LogError($" {responseMessage.PyloadCase} QueryResponse recipient{responseMessage.QueryResponse.Recipient} does not match Query recipient {queryMessage.Query.Recipient}");
                } else if (queryMessage.Head.Id != responseMessage.QueryResponse.ResponseId) {
                    _logger.LogError($" {responseMessage.PyloadCase} QueryResponse ID {responseMessage.QueryResponse.ResponseId} does not match Query ID{queryMessage.Head.Id}");
                } else {
                    _logger.LogDebug($"Message {responseMessage} is a Response to {queryMessage} message");
                    return true;
                }
            }
            return ret;
        }

        private bool checkFileMessage(Message queryMessage, Message responseMessage) {
            _logger.LogDebug($"Checking File Response message {responseMessage} for sent message {queryMessage}");
            bool ret = false;
            if (queryMessage == null) {
                return false;
            } else {
                if ((queryMessage.PyloadCase != Message.PyloadOneofCase.Cmd) && (queryMessage.Cmd.CommandCase != UBA_PROTO_CMD.command_message.CommandOneofCase.File)) {
                    _logger.LogError(queryMessage.PyloadCase + " is not a File Command Request message");
                } else if (responseMessage.PyloadCase != Message.PyloadOneofCase.File) {
                    _logger.LogError($"Payload: {responseMessage.PyloadCase} is not a file chank ({queryMessage.Cmd.File.ChunkIndex}) message");
                } else if (queryMessage.Cmd.File.ChunkIndex != responseMessage.File.ChunkIndex) {
                    _logger.LogError($"File Chunk missmatch {queryMessage.Cmd.File.ChunkIndex} != {responseMessage.File.ChunkIndex}");
                } else {
                    _logger.LogDebug($"File Message {responseMessage} is a Response to {queryMessage} message");
                    return true;
                }
            }
            return ret;
        }

        private bool checkFileListMessage(Message queryMessage, Message responseMessage) {
            _logger.LogDebug($"Checking File Response message {responseMessage} for sent message {queryMessage}");
            bool ret = false;
            if (queryMessage == null) {
                return false;
            } else {
                if ((queryMessage.PyloadCase != Message.PyloadOneofCase.Cmd) && (queryMessage.Cmd.CommandCase != UBA_PROTO_CMD.command_message.CommandOneofCase.File)) {
                    _logger.LogError(queryMessage.PyloadCase + " is not a File Command Request message");
                } else if (responseMessage.PyloadCase != Message.PyloadOneofCase.FmList) {
                    _logger.LogError(responseMessage.PyloadCase + " is not a QueryResponse message");
                } else {
                    _logger.LogDebug($"File Message {responseMessage} is a Response to {queryMessage} message");
                    return true;
                }
            }
            return ret;
        }


        public static uint GetRandomUInt32() {
            byte[] buffer = new byte[4];
            RandomNumberGenerator.Fill(buffer);
            return BitConverter.ToUInt32(buffer, 0);
        }

        public async Task<Message?> GetMessage(Message? send, int timeout = 5000) {
           /* if (sp == null || !sp.IsOpen) {
                _logger.LogError("Serial port is not open.");
                failes++;
                return null;
            }*/
            var stopwatch = System.Diagnostics.Stopwatch.StartNew();
            if (send?.PyloadCase == Message.PyloadOneofCase.Query) {
                Message? res = await EnqueueMessageAndWaitForResponseAsync(new Message(send), MessagePriority.DEVICE_QUERY, timeout);
                if (res != null) {
                    _logger.LogDebug($"Received Query Response : {res.QueryResponse.Recipient} in {stopwatch.ElapsedMilliseconds} ms");
                    return new Message(res);
                }
            } else if (send.PyloadCase == Message.PyloadOneofCase.Cmd && send.Cmd?.CommandCase == UBA_PROTO_CMD.command_message.CommandOneofCase.File && send.Cmd.File.Id == UBA_PROTO_FM.CMD_ID.ChunkRequest) {
                Message? res = await EnqueueMessageAndWaitFileChunkAsync(send);
                if (res != null) {
                    _logger.LogDebug($"Received File Chunk : {res.File.ChunkIndex} in {stopwatch.ElapsedMilliseconds} ms");
                    return new Message(res);
                }
            } else if (send.PyloadCase == Message.PyloadOneofCase.Cmd && send.Cmd?.CommandCase == UBA_PROTO_CMD.command_message.CommandOneofCase.File && send.Cmd.File.Id == UBA_PROTO_FM.CMD_ID.FileListRequest) {
                Message? res = await EnqueueMessageAndWaitFileList(send);
                if (res != null) {
                    _logger.LogDebug($"Received File List : {res.FmList.Filenames.Count} / {res.FmList.TotalFiles} in {stopwatch.ElapsedMilliseconds} ms");
                    return new Message(res); ;
                }
            } else if (send.PyloadCase == Message.PyloadOneofCase.Cmd && send.Cmd?.CommandCase == UBA_PROTO_CMD.command_message.CommandOneofCase.File && send.Cmd.File.Id == UBA_PROTO_FM.CMD_ID.BptFile) {
                Message? res = await EnqueueMessageAndWaitFileList(send);
                if (res != null) {
                    _logger.LogDebug($"Received File List : {res.FmList.Filenames.Count} / {res.FmList.TotalFiles} in {stopwatch.ElapsedMilliseconds} ms");
                    return new Message(res); ;
                }
            }
            return null;
        }


        public async Task<Message?> GetMessage(UBA_PROTO_QUERY.RECIPIENT recipient, UInt32 targateAddress = 0xffffffff, int timeout = 5000) {
          /*  if (sp == null || !sp.IsOpen) {
                _logger.LogError("Serial port is not open.");
                failes++;
                if (failes > 0) {
                    this.SwitchCom(this.PortName, true);
                    failes = 0;
                }
                return null;
            }*/
            Message queryMessage = UBA_Message_Factory.CreateQeuryMessage(targateAddress, recipient);
            Message? responseMessage = await EnqueueMessageAndWaitForResponseAsync(queryMessage, MessagePriority.QUERY_MESSAGE, timeout);

            return responseMessage;
        }

        public async Task<Message?> EnqueueMessageAndWaitForResponseAsync(Message? message, MessagePriority priority = MessagePriority.DEFUALT, int timeout = 5000) {
            if (message == null) throw new ArgumentNullException(nameof(message));       
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
            var originalId = message.Head.Id;
            CancellationTokenSource timeoutCts = new(timeout);
            var stopwatch = System.Diagnostics.Stopwatch.StartNew();
            handler = (sender, args) => {
                if (checkQueryMessage(message, args.Msg)) {
                    tcs.TrySetResult(args.Msg);
                }
            };
            MessageReceived += handler;
            try {
                EnqueueMessage(message, priority);
                using (timeoutCts) {
                    var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                    stopwatch.Stop();
                    if (completedTask == tcs.Task) {
                        _logger.LogDebug($"Received response for Message ID: {originalId} in {stopwatch.ElapsedMilliseconds} ms");
                        return tcs.Task.Result;
                    } else {
                        _logger.LogError($"Timeout waiting for response with Message ID: {originalId} after {stopwatch.ElapsedMilliseconds} ms");
                        return null;
                    }
                }
            } finally {
                MessageReceived -= handler;
            }
        }

        public async Task<Message?> EnqueueMessageAndWaitFileChunkAsync(Message message, MessagePriority priority = MessagePriority.FILE_DATA_REQUEST, int timeout = 50000) {
            if (message == null) throw new ArgumentNullException(nameof(message));          
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
            CancellationTokenSource timeoutCts = new(timeout);
            handler = (sender, args) => {
                if (checkFileMessage(message, args.Msg)) {
                    tcs.TrySetResult(args.Msg);
                }
            };
            MessageReceived += handler;
            try {
                EnqueueMessage(message, priority);
                using (timeoutCts) {
                    var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                    if (completedTask == tcs.Task) {
                        _logger.LogDebug($"Received response for Message ID: {message.Head.Id}");
                        return tcs.Task.Result;
                    } else {
                        _logger.LogError($"Timeout waiting for response with Message ID: {message.Head.Id}");
                        return null;
                    }
                }
            } finally {
                MessageReceived -= handler;
            }
        }
        public async Task<Message?> EnqueueMessageAndWaitFileList(Message message, MessagePriority priority = MessagePriority.FILE_NAME_REQUEST, int timeout = 50000) {
            if (message == null) throw new ArgumentNullException(nameof(message));
          
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
            CancellationTokenSource timeoutCts = new(timeout);
            handler = (sender, args) => {
                if (checkFileListMessage(message, args.Msg)) {
                    tcs.TrySetResult(args.Msg);
                }
            };
            MessageReceived += handler;
            try {
                EnqueueMessage(message, priority);
                using (timeoutCts) {
                    var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                    if (completedTask == tcs.Task) {
                        _logger.LogDebug($"Received response for Message ID: {message.Head.Id}");
                        return tcs.Task.Result;
                    } else {
                        _logger.LogError($"Timeout waiting for response with Message ID: {message.Head.Id}");
                        return null;
                    }
                }
            } finally {
                MessageReceived -= handler;
            }
        }

        public override string ToString() {
            return $"UBA_Interface: {sp?.PortName ?? "Not Connected"}";
        }

    }

}
