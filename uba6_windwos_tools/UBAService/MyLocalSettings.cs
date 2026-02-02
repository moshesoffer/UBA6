using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBAService {
    public class MyLocalSettings {
        public string LogPath { get; set; }
        public int RetryCount { get; set; }

        public string ServerIpAddress { get; set; }
    }
}
