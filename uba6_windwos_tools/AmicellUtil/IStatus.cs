using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil
{
    public interface IStatus{
        event EventHandler<StatusEventArg> StatusChanged;
        event EventHandler<ExceptionEventArg> ExceptionOccurred;
        public void RaiseNewStatusEvent(string statusStr);
        public void RaiseNewStatusEvent(string statusStr, int progress);
        public Exception RaiseException(Exception ex);

    }
}
