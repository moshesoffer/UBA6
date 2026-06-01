using Google.Protobuf;
using Microsoft.Extensions.Logging;
using Microsoft.VisualBasic;
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
            BPT_STEP = 11,
            DEVICE_QUERY = 4,
            BPT_QUERY = 5,
            QUERY_MESSAGE = 6,
            FILE_NAME_REQUEST = 7,
            FILE_DATA_REQUEST = 8,
            DEFUALT = 10,

        }
    
        private readonly SemaphoreSlim _semaphore = new SemaphoreSlim(1, 1);

        public UBA_Interface(ILogger<UBA_Interface> logger) {
            _logger = logger;
        }

        //timout control for async requests
//        public const int AWAIT_TIMEOUT = 2000;
//        static async Task<T> WithTimeout<T>(Task<T> task, int milliseconds)
//        {
//            using var cts = new System.Threading.CancellationTokenSource(milliseconds);
//
//            var completedTask = await Task.WhenAny(task, Task.Delay(-1, cts.Token));
//
//            if (completedTask != task)
//                throw new TimeoutException($"Timeout after {milliseconds}ms");
//
//            return await task;
//        }

        public UBA_Interface(ILogger<UBA_Interface> logger, string portName, int baudRate = 115200) : this(logger) {
            sp = new SerialPort(portName, baudRate);
            sp.ReadBufferSize = 8192;
            sp.Parity = Parity.None;
            sp.ReadTimeout = 600;
            sp.WriteTimeout = 600;
            sp.DataReceived += SerialPort_DataReceived;
            _logger.LogDebug($"Initializing UBA_Interface with COM port: {portName}");
            try {
                sp.Open();
            } catch (Exception ex) {
                _logger.LogError($"Initializing COM port {portName}: {ex.Message}");
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
            sp.ReadTimeout = 600;
            sp.WriteTimeout = 600;
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
                    try {
                        ulong messageLength = DecodeVarint(sp.BaseStream);
                        _logger.LogDebug($"Message Length: {messageLength}");
                        messageSize = (int)messageLength;
                    } catch {
                        _logger.LogInformation($"DecodeVarint read failed.");
                    }
                }
                //double check message size
                if (messageSize == 0) {
                    return;
                }

                buffer = new byte[messageSize];// create buffer with size of message
                try
                {
                    int bytesRead = 0;
                    while (bytesRead < messageSize) {
                        ////Moshe - concatenate read buffer parts
                        int currBytes = sp.BaseStream.Read(buffer, bytesRead, messageSize-bytesRead); // read the message from the stream
                        bytesRead += currBytes;
                    }
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
                catch /*(OperationCanceledException)*/
                {
                    ////throw new TimeoutException("Serial read timed out.");
                    _logger.LogInformation($"Serial read timed out.");
                }
            } catch (Exception ex) {
                _logger.LogDebug($"Buffer (hex): {BitConverter.ToString(buffer)} - Exception {ex}");
                _logger.LogInformation($"Error reading from serial: {ex.Message} , Resetting the serial port");
                //Moshe
                ////sp.DiscardInBuffer();
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
                    //Moshe
                    if (Monitor.TryEnter(_serialReadLock, 5000)) {   
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
                        }
                    } else {
                        messageQueue.Enqueue(msg, 1);
                        _logger.LogWarning("Serial port is busy reading, skipping write for this cycle.");
                    }
                    Monitor.Exit(_serialReadLock);
                } else {
                }
                await Task.Delay(50, cancellationToken); // Avoid busy-רקשגןמע
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
            //_logger.Debug($"Checking Response message {responseMessage} for sent message {queryMessage}");
            bool ret = false;

            if (queryMessage == null) {
                return false;
            } else {
                //verify response message
                if (queryMessage.PyloadCase == Message.PyloadOneofCase.Query) {
                    if ((queryMessage.Query.Recipient & responseMessage.QueryResponse.Recipient) != responseMessage.QueryResponse.Recipient) {
                        ////_logger.LogError($" {responseMessage.PyloadCase} QueryResponse recipient{responseMessage.QueryResponse.Recipient} does not match Query recipient {queryMessage.Query.Recipient}");
                    } else if (queryMessage.Head.Id != responseMessage.QueryResponse.ResponseId) {
                        ////_logger.LogError($" {responseMessage.PyloadCase} QueryResponse ID {responseMessage.QueryResponse.ResponseId} does not match Query ID {queryMessage.Head.Id}");
                    } else {
                        ////_logger.LogInformation($"Message {responseMessage} is a Response to {queryMessage} message");
                        ////_logger.LogInformation($"Message ID {responseMessage.QueryResponse.ResponseId} is a Response to Query ID {queryMessage.Head.Id}");
                        return true;
                    }
                } else if ((queryMessage.PyloadCase == Message.PyloadOneofCase.QueryResponse) ||
                           (queryMessage.PyloadCase == Message.PyloadOneofCase.Tr)) {
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
                    _logger.LogError($"Payload: {responseMessage.PyloadCase} is not a file chunk ({queryMessage.Cmd.File.ChunkIndex}) message, ({responseMessage.PyloadCase})");
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
                Message? res = await EnqueueMessageAndWaitForResponseAsync(new Message(send), MessagePriority.DEVICE_QUERY, 5000/*timeout*/);
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


        public async Task<Message?> GetMessage(UBA_PROTO_QUERY.RECIPIENT recipient, UInt32 targateAddress = 0xffffffff, int timeout = 10000) {
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

        public async Task<Message?> EnqueueMessageAndWaitForResponseAsync(Message? message, MessagePriority priority = MessagePriority.DEFUALT, int timeout = 0) {
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

            await _semaphore.WaitAsync();
            try
            {
                MessageReceived += handler;
                try { 
                    EnqueueMessage(message, priority);
                    using (timeoutCts) {
                        var delayTask = Task.Delay(timeout);
////_logger.LogInformation($"==> await Task.WhenAny 1 taskID: {tcs.Task.Id} pri {priority}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
////_logger.LogInformation($"==> response Task.WhenAny 1 taskID: {completedTask.Id}");
                        stopwatch.Stop();
                        if ((priority == MessagePriority.DEVICE_QUERY) || (priority == MessagePriority.BPT_QUERY) ||(priority == MessagePriority.QUERY_MESSAGE) ||
                            (priority == MessagePriority.FILE_NAME_REQUEST) || (priority == MessagePriority.FILE_DATA_REQUEST))
                        {
                            if (completedTask == tcs.Task) {
                                //_logger.LogDebug($"Received response for Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");// in {stopwatch.ElapsedMilliseconds} ms");
                                return tcs.Task.Result;
                            } else if (completedTask == delayTask) {
                                _logger.LogError($"1-Timeout waiting for response with Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");/// after {stopwatch.ElapsedMilliseconds} ms");
                                return null;
                            } else {
                            }
                        }
                    }
                } finally {
                    MessageReceived -= handler;
                }
            }
            finally
            {
                _semaphore.Release();
            }            
            return null;
         }

        public async Task<Message?> EnqueueMessageAndWaitFileChunkAsync(Message message, MessagePriority priority = MessagePriority.FILE_DATA_REQUEST, int timeout = 30000) {
            if (message == null) throw new ArgumentNullException(nameof(message));          
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
            CancellationTokenSource timeoutCts = new(timeout);
            handler = (sender, args) => {
                if (checkFileMessage(message, args.Msg)) {
                    tcs.TrySetResult(args.Msg);
                }
            };

            await _semaphore.WaitAsync();
            try
            {
                MessageReceived += handler;
                try {
                    EnqueueMessage(message, priority);
                    using (timeoutCts) {
////_logger.LogInformation($"==> await Task.WhenAny 2 task ID: {tcs.Task} pri {priority}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
////_logger.LogInformation($"==> response Task.WhenAny 2 taskID: {completedTask.Id}");
                        if (completedTask == tcs.Task) {
                            _logger.LogDebug($"Received response for Message ID: {message.Head.Id}");
                            return tcs.Task.Result;
                        } else {
                            _logger.LogError($"2-Timeout waiting for response with Message ID: {message.Head.Id} {completedTask.Id}");
                            return null;
                        }
                    }
                } finally {
                    MessageReceived -= handler;
                }
            }
            finally
            {
                _semaphore.Release();
            }            
        }

        public async Task<Message?> EnqueueMessageAndWaitFileList(Message message, MessagePriority priority = MessagePriority.FILE_NAME_REQUEST, int timeout = 8000) {
            if (message == null) throw new ArgumentNullException(nameof(message));
          
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
            CancellationTokenSource timeoutCts = new(timeout);
            handler = (sender, args) => {
                if (checkFileListMessage(message, args.Msg)) {
                    tcs.TrySetResult(args.Msg);
                }
            };

            await _semaphore.WaitAsync();
            try
            {
                MessageReceived += handler;
                try {
                    EnqueueMessage(message, priority);
                    using (timeoutCts) {
////_logger.LogInformation($"==> await Task.WhenAny 3 task ID: {tcs.Task} pri {priority}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
////_logger.LogInformation($"==> response Task.WhenAny 3 taskID: {completedTask.Id}");
                        if (completedTask == tcs.Task) {
                            _logger.LogDebug($"Received response for Message ID: {message.Head.Id}");
                            return tcs.Task.Result;
                        } else {
                            _logger.LogError($"3-Timeout waiting for response with Message ID: {message.Head.Id}");
                            return null;
                        }
                    }               
                } finally {
                    MessageReceived -= handler;
                }
            }
            finally
            {
                _semaphore.Release();
            }            
        }

        public override string ToString() {
            return $"UBA_Interface: {sp?.PortName ?? "Not Connected"}";
        }

    }

}
