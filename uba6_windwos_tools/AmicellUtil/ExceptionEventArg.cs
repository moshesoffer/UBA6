using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil {
    public class ExceptionEventArg {
        public DateTime Timestamp { get; }
        public Exception Exception { get; }
        public int Progress { get; }

        public ExceptionEventArg(Exception ex, DateTime timestamp) {
            Exception = ex;
            Timestamp = timestamp;

        }
        public ExceptionEventArg(Exception ex) : this(ex, DateTime.UtcNow) {
        }
    }
}
