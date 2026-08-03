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
using UBA6Library;
using UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks.Models;
using UBA6Library.WebServerApi.Services.WebConsole;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests;
using UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models;
using UBA6Library.WebServerApi.Services.WebConsole.Model;



namespace UBA6Library {
    public class UBA_Interface {
        protected readonly ILogger<UBA_Interface> _logger;
        private readonly int MAX_PORT_READ_RETRIES = 10;
        private SerialPort? sp { get; set; }
        private PriorityQueue<Message, int> messageQueue = new();
        private CancellationTokenSource? _cts;
        private Task? _processingTask;
        private Task? _readerTask;
        private Task? _pendingTask;
        public event EventHandler<ProtoMessageEventArg>? MessageReceived;
        private bool disposed = false;
//        private int messageSize = 0;
        protected static UInt32 messageId = 0;
        private int failes { get; set; } = 0; // the number of failed to open the port 
        public string PortName => sp?.PortName ?? "Not Connected";
        private WebConsoleService wcs;

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
        private readonly MemoryStream _rxBuffer = new();
        private readonly object _lock = new();

        private readonly SemaphoreSlim _semaphore = new SemaphoreSlim(1, 1);

        public UBA_Interface(ILogger<UBA_Interface> logger) {
            _logger = logger;
        }

        public UBA_Interface(ILogger<UBA_Interface> logger, string portName, int baudRate = 115200) : this(logger)
        {
            sp = new SerialPort(portName, baudRate);
            sp.ReadBufferSize = 2048;
            sp.Parity = Parity.None;
            sp.ReadTimeout = 3000;
            sp.WriteTimeout = 300;

            // sp.DataReceived += SerialPort_DataReceived;

            _logger.LogDebug($"Initializing UBA_Interface with COM port: {portName}");

            try
            {
                sp.Open();
            }
            catch (Exception ex)
            {
                _logger.LogError($"Initializing COM port {portName}: {ex.Message}");
            }

            ClearQueueMessage();
            StartProcessing();
        }

        public void Dispose()
        {
_logger.LogInformation($"==> Remove Interface:");
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposed)
                return;

            disposed = true;

            if (disposing)
            {
                // Stop background tasks if you have any
                // e.g. cancellationTokenSource?.Cancel();

                if (sp != null)
                {
                    try
                    {
                        // sp.DataReceived -= SerialPort_DataReceived;

                        if (sp.IsOpen)
                        {
                            sp.Close();
                        }

                        sp.Dispose();
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError($"Error disposing SerialPort: {ex.Message}");
                    }

                    sp = null;
                }
            }
        }

        ~UBA_Interface()
        {
            Dispose(false);
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
//                    sp.DataReceived -= SerialPort_DataReceived;
                if (sp.IsOpen) { 
                    sp.Close();
                }
                sp.Dispose();
            }
            sp = new SerialPort(newComPort, 115200);
            sp.ReadBufferSize = 2048;
            sp.Parity = Parity.None;
            sp.ReadTimeout = 3000;
            sp.WriteTimeout = 300;
//            sp.DataReceived += SerialPort_DataReceived;
            try {
                sp.Open();
                _logger.LogDebug($"COM port switched to {sp.PortName}");
            } catch (Exception ex) {
                _logger.LogError($"Failed to open COM port {sp.PortName}: {ex.Message}");
            } finally {
                failes = 0; 
            }
        }

        private readonly object _portLock = new();

        private void SafeReset()
        {
            lock (_portLock)
            {
                try
                {
                    if (sp == null)
                        return;

                    _logger.LogWarning("Resetting serial port...");

                    if (sp.IsOpen)
                        sp.Close();

                    Thread.Sleep(300);

                    sp.Dispose();

                    sp = new SerialPort(sp.PortName, 115200)
                    {
                        ReadBufferSize = 2048,
                        Parity = Parity.None,
                        ReadTimeout = 3000,
                        WriteTimeout = 300
                    };

                    sp.Open();
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Reset failed");
                }
            }
        }

        public void EnqueueMessage(Message message, MessagePriority priority = MessagePriority.DEFUALT) {
            if (message == null) {
                throw new ArgumentNullException(nameof(message));
            }
            _logger.LogDebug($"Enqueuing message: {message} with priority {priority}");
            messageQueue.Enqueue(message, (int)priority);
        }

        public void ClearQueueMessage() {
            messageQueue.Clear();
        }

//        public void StartProcessing() {
//            if (_processingTask != null && !_processingTask.IsCompleted) {
//                _logger.LogDebug("Processing task is already running, not starting a new one.");
//                return;
//            }
//            _cts = new CancellationTokenSource();
//            _processingTask = Task.Run(() => ProcessQueueAsync(_cts.Token));
//        }

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

        public void StartProcessing()
        {
            _cts = new CancellationTokenSource();

            // start queue proccesing 
            if (_processingTask != null && !_processingTask.IsCompleted) {
                _logger.LogDebug("Queue processing already running.");
            } else {
                _processingTask = Task.Run(() => ProcessQueueAsync(_cts.Token));
            }

            // start pending UBA resolution 
//Moshe
//            if (_pendingTask != null && !_pendingTask.IsCompleted) {
//                _logger.LogDebug("Pending UBA resolution already running.");
//            } else {
//                _pendingTask = Task.Run(() => ResolvePendingUBAAsync(_cts.Token));
//            }

            // start serial port reader
            if (_readerTask != null && !_readerTask.IsCompleted) {
                _logger.LogDebug("Serial port reader already running.");
            } else {
                _readerTask = Task.Run(() => SerialPortReadLoop(_cts.Token));
            }
        }

        private void ReadExact(Stream stream, byte[] buffer, int count)
        {
            int offset = 0;
            int stallCounter = 0;

            while (offset < count)
            {
                int read = stream.Read(buffer, offset, count - offset);

                if (read > 0)
                {
                    offset += read;
                    stallCounter = 0;
                }
                else
                {
                    stallCounter++;

                    if (stallCounter > 50)
                        throw new IOException("Serial stalled mid-message");
                }
            }
        }
        private void SerialPortReadLoop(CancellationToken token)
        {
            var parser = new MessageParser<Message>(() => new Message());

            while (!token.IsCancellationRequested)
            {
                try
                {
                    if (sp == null || !sp.IsOpen)
                    {
                        Thread.Sleep(250);
                        continue;
                    }

                    // 1. Read protobuf length prefix
                    ulong length = DecodeVarint(sp.BaseStream);

                    if (length == 0)
                        continue;

                    if (length == 0 || length > 10_000_000)
                    {
                        _logger.LogError($"Invalid length: {length}");
                        continue;
                    }
                    if (length > 10_000_000)
                        throw new InvalidDataException($"Invalid length: {length}");

                    byte[] buffer = new byte[(int)length];

                    // 2. Read full payload
                    ReadExact(sp.BaseStream, buffer, (int)length);
                    //_logger.LogInformation($"SerialPortReadLoop: {length}");

                    // 3. Parse protobuf
                    try {
                        Message msg = parser.ParseFrom(buffer);
                        _logger.LogDebug($"RX Message: {msg}");

                        // 4. Raise event
                        MessageReceived?.Invoke(this, new ProtoMessageEventArg(msg));

                    } catch {
                        _logger.LogDebug($"ParseFrom: scan for next valid varint");
                        continue;
                    }
                }
                catch (TimeoutException)
                {
                    // ignore read timeouts
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Serial failure - restarting reader");

                    Task.Run(() =>
                    {
                        Thread.Sleep(500);
                        SafeReset();
                    });
                }
            }
        }

        private List<byte> message2byteArry(Message? msg = null) {
            if (msg == null) {
                msg = new Message();
            }
            List<byte> retByteList = [.. EncodeVarint((ulong)msg.CalculateSize()), .. msg.ToByteArray()];
            _logger.LogDebug($"Packet Size: {retByteList.Count} Message Size:{msg.CalculateSize()} bytes");
            return retByteList;

        }
        private async Task ProcessQueueAsync(CancellationToken cancellationToken) {
        int timeout = 150; 
            var tcs = new TaskCompletionSource<Message?>();

            while (true) {
                CancellationTokenSource timeoutCts = new(timeout);
                var stopwatch = System.Diagnostics.Stopwatch.StartNew();

                try { 
                    using (timeoutCts) {
                        var delayTask = Task.Delay(timeout);
//_logger.LogInformation($"ProcessQueueAsync: timeout= {timeout}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
                        stopwatch.Stop();

                        Message? msg = null;
                        lock (messageQueue) {
                            if (messageQueue.Count > 0) {
                                _logger.LogDebug($"Processing message queue, count: {messageQueue.Count}");
                                try {
                                    messageQueue.TryDequeue(out msg, out _);
                                } catch (Exception) {
                                    _logger.LogInformation($"Failed to dequeue message");
                                }
                            }
                        }
                        if (msg != null) {
                            if (Monitor.TryEnter(_serialReadLock, 500)) {   
                                try {
                                    if (sp?.IsOpen == false) {
                                        sp.Open();
                                    }
                                    msg.Head.SenderAddress = 0;
                                    byte[] byteMessage = message2byteArry(msg).ToArray();
                                    sp?.Write(byteMessage, 0, byteMessage.Length);
//                                    _logger.LogDebug($"Sent message: TargetAddress= {msg.Head.TargetAddress}");
//                                    _logger.LogInformation($"Sent message: TargetAddress= {msg}");
//                                    _logger.LogInformation($"Sent message: {msg}"); //\nSize:{byteMessage[0]} {BitConverter.ToString(byteMessage)}");

                                } catch (Exception) {
                                    _logger.LogError($"Failed to send message: {msg}");
                                    if ((sp?.IsOpen == false) && (failes++ > MAX_PORT_READ_RETRIES)) { 
                                        _logger.LogInformation("Serial port is closed, attempting to reopen.");
                                        this.SwitchCom(sp.PortName, true);
                                    }
                                } finally {
                                }
                            } else {
//_logger.LogInformation($"Enqueuing message: msg");
                                messageQueue.Enqueue(msg, 1);
                                _logger.LogWarning("Serial port is busy reading, skipping write for this cycle.");
                            }
                            Monitor.Exit(_serialReadLock);
                        }
                    }
                } finally {
                }
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
//            _logger.LogInformation($"GetMessage: targateAddress {targateAddress}");
            Message queryMessage = UBA_Message_Factory.CreateQeuryMessage(targateAddress, recipient);
            timeout = 5000;
            Message? responseMessage = await EnqueueMessageAndWaitForResponseAsync(queryMessage, MessagePriority.QUERY_MESSAGE, timeout);

            return responseMessage;
        }

        public async Task<Message?> EnqueueMessageAndWaitForResponseAsync(Message? message, MessagePriority priority = MessagePriority.DEFUALT, int timeout = 0) {
            if (message == null) throw new ArgumentNullException(nameof(message));       
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
            var originalId = message.Head.Id;
            
            timeout = 1000;
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
//_logger.LogInformation($"==> await Task.WhenAny 1 taskID: {tcs.Task.Id} pri {priority} timeout {timeout} message {message.Head}");
//_logger.LogInformation($"EnqueueMessageAndWaitForResponseAsync: timeout= {timeout}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
//_logger.LogInformation($"==> response Task.WhenAny 1 taskID: {completedTask.Id}");
                        stopwatch.Stop();
                        if (priority == MessagePriority.DEVICE_QUERY) {
                            if (completedTask == tcs.Task) {
                                //_logger.LogDebug($"Received response for Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");// in {stopwatch.ElapsedMilliseconds} ms");
                                return tcs.Task.Result;
                            } else if (completedTask == delayTask) {
                                _logger.LogInformation($"1-Timeout waiting for response with Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");/// after {stopwatch.ElapsedMilliseconds} ms");
                                return null;
                            }                             
                        }
                        else if (priority == MessagePriority.QUERY_MESSAGE) {
                            if (completedTask == tcs.Task) {
                                //_logger.LogDebug($"Received response for Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");// in {stopwatch.ElapsedMilliseconds} ms");
                                return tcs.Task.Result;
                            } else if (completedTask == delayTask) {
                                _logger.LogInformation($"1-Timeout waiting for response with Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");/// after {stopwatch.ElapsedMilliseconds} ms");
                                return null;
                            }                             
                        }
                        else if ((priority == MessagePriority.BPT_QUERY) ||
                            (priority == MessagePriority.FILE_NAME_REQUEST) || (priority == MessagePriority.FILE_DATA_REQUEST))
                        {
                            if (completedTask == tcs.Task) {
                                //_logger.LogDebug($"Received response for Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");// in {stopwatch.ElapsedMilliseconds} ms");
                                return tcs.Task.Result;
                            } else if (completedTask == delayTask) {
                                _logger.LogInformation($"1-Timeout waiting for response with Message ID: {originalId} taskID: {completedTask.Id}-{tcs.Task.Id}");/// after {stopwatch.ElapsedMilliseconds} ms");
                                return null;
                            } 
                        }
                        else if (priority == MessagePriority.TEST_ROUTINE) {
                            return null;                            
                        } else {
                            _logger.LogInformation($"1-Wrong priority: {priority}");
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
            //_logger.LogInformation($"1-Return Null Message ID: taskID: {tcs.Task.Id}");
            return null;
         }

        public async Task<Message?> EnqueueMessageAndWaitFileChunkAsync(Message message, MessagePriority priority = MessagePriority.FILE_DATA_REQUEST, int timeout = 30000) {
            if (message == null) throw new ArgumentNullException(nameof(message));

_logger.LogInformation($"==> await EnqueueMessageAndWaitFileChunkAsync-1: {timeout}");
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
                        timeout = 3 * 60 * 1000;//for case of long files and case of dual test
//_logger.LogInformation($"EnqueueMessageAndWaitForResponseAsync-2: timeout= {timeout}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
////_logger.LogInformation($"==> response Task.WhenAny 2 taskID: {completedTask.Id}");
                        if (completedTask == tcs.Task) {
                            _logger.LogDebug($"Received response for Message ID: {message.Head.Id}");
                            return tcs.Task.Result;
                        } else {
                            _logger.LogInformation($"2-Timeout waiting for Chunk File response with Message ID: {message.Head.Id} {completedTask.Id}");
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

        public async Task<Message?> EnqueueMessageAndWaitFileList(Message message, MessagePriority priority = MessagePriority.FILE_NAME_REQUEST, int timeout = 2000) {
            if (message == null) throw new ArgumentNullException(nameof(message));
          
//_logger.LogInformation($"==> await EnqueueMessageAndWaitFileList {timeout}");
            var tcs = new TaskCompletionSource<Message?>();
            EventHandler<ProtoMessageEventArg>? handler = null;
//            timeout = 3 * 60 * 1000; //3 min
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
//_logger.LogInformation($"EnqueueMessageAndWaitFileList: timeout= {timeout}");
                        var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
////_logger.LogInformation($"==> response Task.WhenAny 3 taskID: {completedTask.Id}");
                        if (completedTask == tcs.Task) {
                            _logger.LogDebug($"Received response for Message ID: {message.Head.Id}");
                            return tcs.Task.Result;
                        } else {
                            _logger.LogInformation($"3-Timeout waiting for response with Message ID: {message.Head.Id}");
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

        private async Task ResolvePendingUBAAsync(CancellationToken cancellationToken) {
            int timeout = 1000;
            var tcs = new TaskCompletionSource<Message?>();

            while (true) {
                CancellationTokenSource timeoutCts = new(timeout);
                var stopwatch = System.Diagnostics.Stopwatch.StartNew();

                try { 
                    using (timeoutCts) {
                        var delayTask = Task.Delay(timeout);
//_logger.LogInformation($"ResolvePendingUBAAsync: timeout= {timeout}");
                       var completedTask = await Task.WhenAny(tcs.Task, Task.Delay(timeout, timeoutCts.Token));
//_logger.LogInformation($"==> ResolvePendingUBAAsync: after {stopwatch.ElapsedMilliseconds} ms");
                        stopwatch.Stop();

                        GETPendingTasksDTO pt = await wcs.GetPendingTasks();
                        if (pt != null) {
                            if (pt?.PendingConnectionUbaDevices?.Count > 0) {
                                foreach (var pendingDevice in pt.PendingConnectionUbaDevices) {
                                    Message? t = await GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device, Convert.ToUInt32(pendingDevice.Address));
                                    if (t != null) {
                                        _logger.LogInformation($"1 Received message from UBA Device '{t?.QueryResponse.Recipient}' {t?.QueryResponse.Device.Settings}");
                                        if (t != null) {
                                            await wcs.DeviceFound(t.QueryResponse, pendingDevice.ComPort);
                                        }
                                    } else {
                                        _logger.LogWarning($"wrong message from UBA Device on Port {pendingDevice.ComPort} at Address {pendingDevice.Address}");
                                    }
                                }
                            }
                        }
                    }
                } finally {
                }
            }
        }
    }
}
