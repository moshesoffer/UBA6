using UBA_MSG;

namespace UBA6Library {
    public class ProtoMessageEventArg : EventArgs {
        public Message Msg { get; }
        public DateTime Timestamp { get; }

        public ProtoMessageEventArg(Message msg, DateTime timestamp) {
            Msg = new Message(msg);
            Timestamp = timestamp;
        }
        public ProtoMessageEventArg(Message msg) : this(msg, DateTime.UtcNow) { }
    }

}
