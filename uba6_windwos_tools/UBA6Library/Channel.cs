using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library {
    public class Channel {       

        UBA_PROTO_CHANNEL.ID ID { get; set; }

        UBA_PROTO_CHANNEL.STATE State { get; set; }

        UBA_PROTO_UBA6.ERROR Error { get; set; }

        public Int32 Voltage { get; set; }
        public float Temperature { get; set; }
        public Int32 Current { get; set; }
        public float Capacity { get; set; }
        public bool IsBatteryConnected { get; set; }

        public string Name { 
            get {
                return $"CH {ID}";
            } 
        }


        public Channel() { 
        
        
        }
        public Channel(UBA_PROTO_CHANNEL.ID id) {
            ID= id; 
        }

        public Channel(UBA_PROTO_CHANNEL.status status) {
            ID = (UBA_PROTO_CHANNEL.ID) status.Id;
            State = (UBA_PROTO_CHANNEL.STATE) status.State;
            Voltage = status.Data.Voltage;
            Temperature = status.Data.Temperature;
            Current = status.Data.Current;
            Capacity = status.Data.Capacity;
            IsBatteryConnected = status.Data.IsBatteryConnected;
        }

        public void UpdateFromMessage(UBA_PROTO_CHANNEL.status status) {
            ID = (UBA_PROTO_CHANNEL.ID)status.Id;
            State = (UBA_PROTO_CHANNEL.STATE)status.State;
            Voltage = status.Data.Voltage;
            Temperature = status.Data.Temperature;
            Current = status.Data.Current;
            Capacity = status.Data.Capacity;
            IsBatteryConnected = status.Data.IsBatteryConnected;

        }
    }
}
