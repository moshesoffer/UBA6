using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil {
    public class StatusEventArg : EventArgs {
        public DateTime Timestamp { get; }
        public string Status { get; }
        public int? Progress { get; } = null;

        public StatusEventArg(string status, DateTime timestamp) {
            Status = status;
            Timestamp = timestamp;
        }
        public StatusEventArg(string status) : this(status, DateTime.Now) {
        }
        public StatusEventArg(string status, int progress) : this(status) {
            Progress = progress;
        }

        public override string ToString() {
            return $"{Timestamp:yyyy-MM-dd HH:mm:ss} - {Status} (Progress: {Progress}%)";
        }

    }
}
