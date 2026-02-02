using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Exceptions {
    
    [Serializable]
    public class ServerConflictException : Exception {
        public ServerConflictException() {

        }
        public ServerConflictException(string msg) : base(msg) {

        }

        public ServerConflictException(string msg, Exception inner) : base(msg, inner) {

        }
    }
}
