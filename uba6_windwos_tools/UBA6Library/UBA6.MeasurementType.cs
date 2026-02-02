namespace UBA6Library {
    public partial class UBA6 {
       

        [Flags]
        public enum MeasurementType : UInt32 {
            None = 0x00,
            CannelA = UBA_PROTO_QUERY.RECIPIENT.ChannelA,
            CannelB = UBA_PROTO_QUERY.RECIPIENT.ChannelB,
            CannelAB = UBA_PROTO_QUERY.RECIPIENT.ChannelAb,
            LineA = UBA_PROTO_QUERY.RECIPIENT.LineA,
            LineB = UBA_PROTO_QUERY.RECIPIENT.LineB,
            Device = UBA_PROTO_QUERY.RECIPIENT.Device,
            RECIPIENT = CannelA | CannelB | CannelAB | LineA | LineB | Device,
            BAT_Voltage = 0x00010000,
            GEN_Voltage = 0x00020000,
            ChargeCurrent = 0x00040000,
            DischageCurrent = 0x00080000,
            AMB_Temp = 0x00100000,
            NTC_Temp = 0x00200000,
            Capacity = 0x00400000,
            VPS = 0x00800000,
            Type = BAT_Voltage | GEN_Voltage | ChargeCurrent | DischageCurrent | AMB_Temp | NTC_Temp | Capacity | VPS,
        }
    }
}
