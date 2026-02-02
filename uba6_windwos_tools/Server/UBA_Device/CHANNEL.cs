namespace Server.UBA_Device {
    public partial class DeviceDTO
    {
        [Flags]
        public enum CHANNEL
        {
            NONE = 0,
            A = 1,
            B = 2,
            AB = A | B,
        }

    }
}
