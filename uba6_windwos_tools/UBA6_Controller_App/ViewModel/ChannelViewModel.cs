using CommunityToolkit.Mvvm.ComponentModel;
using System.Windows;
using UBA6Library;

namespace UBA6_Controller_App.ViewModel {
    public partial class ChannelViewModel : ObservableObject {
        #region SCREEN
        [ObservableProperty]
        UBA_PROTO_LINE.STATE lineState = UBA_PROTO_LINE.STATE.Init;
        [ObservableProperty]
        UBA_PROTO_UBA6.ERROR lineError = UBA_PROTO_UBA6.ERROR.NoError;
        [ObservableProperty]
        UBA_PROTO_CHANNEL.ID channelID = UBA_PROTO_CHANNEL.ID.None;
        [ObservableProperty]
        UBA_PROTO_CHANNEL.STATE channleState = UBA_PROTO_CHANNEL.STATE.Standby;
        [ObservableProperty]
        UBA_PROTO_UBA6.ERROR channelError = UBA_PROTO_UBA6.ERROR.NoError;
        [ObservableProperty]
        UBA_PROTO_BPT.STATE bPT_State = UBA_PROTO_BPT.STATE.Init;
        [ObservableProperty]
        UBA_PROTO_UBA6.ERROR bPT_Error = UBA_PROTO_UBA6.ERROR.NoError;
        [ObservableProperty]
        int bUCK_BOOT_State = 0;
        [ObservableProperty]
        int step = 0;
        [ObservableProperty]
        int totalSteps = 0;
        [ObservableProperty]
        TimeSpan runTime = new TimeSpan(0);
        [ObservableProperty]
        Int32 batVoltage = 15600; /*in mV */
        [ObservableProperty]
        Int32 current = 456;/*in mA (negtive for discharge postive for charge)*/
        [ObservableProperty]
        float capsity = 456.56f; /*the capsity from the*/
        [ObservableProperty]
        float batTemp = 55.56f;
        [ObservableProperty]
        string testName = "Demo Test";
        #endregion

        #region Other
        [ObservableProperty]
        Int32 genVoltage = 12320; /*in mV */
        [ObservableProperty]
        float ambTemp = 654.321f;
        [ObservableProperty]
        Int32 vpsVoltage = 5000; /*in mV */
        [ObservableProperty]
        bool isBatConnected;
        #endregion


        #region ADC
        [ObservableProperty]
        UInt16 adcVbat = 1;
        [ObservableProperty]
        UInt16 adcVps = 2;
        [ObservableProperty]
        UInt16 adcVgen = 3;
        [ObservableProperty]
        UInt16 adcAmbTemp = 4;
        [ObservableProperty]
        UInt16 adcNtcTemp = 5;
        [ObservableProperty]
        UInt16 adcDischargeCurr = 6;
        [ObservableProperty]
        UInt16 adcChargeCurr = 7;
        #endregion

        [ObservableProperty]
        uint buck_presentage;
        [ObservableProperty]
        uint boost_presentage;

        private UBA6 uba;

        public ChannelViewModel() : base() {
        }
        public ChannelViewModel(Channel ch_model) : this() {


        }
        public ChannelViewModel(UBA6 uba) : this() {
            this.uba = uba;
        }


        public void UpdateFromStatusMessage(UBA_PROTO_CHANNEL.status st) {
            try {
                if ((UBA_PROTO_CHANNEL.ID)st.Id == ChannelID) {
                    ChannelID = (UBA_PROTO_CHANNEL.ID)st.Id;
                    ChannleState = (UBA_PROTO_CHANNEL.STATE)st.State;
                    Step = 0;
                    TotalSteps = 0;
                    RunTime = new TimeSpan(0);
                    if (st.Data == null) {
                        return;
                    }
                    BatVoltage = st.Data.Voltage;
                    Current = st.Data.Current;
                    Capsity = st.Data.Capacity;
                    BatTemp = st.Data.Temperature;
                    if (st.LineStatus[0].Data != null) {
                        GenVoltage = st.LineStatus[0].Data.GenVoltage;
                        VpsVoltage = st.LineStatus[0].Data.Vps;
                        AmbTemp = st.LineStatus[0].Data.AmbTemperature;
                        IsBatConnected = st.LineStatus[0].Data.IsBatteryConnected;
                        AdcVbat = (UInt16)st.LineStatus[0].AdcData.Vbat;
                        AdcVgen = (UInt16)st.LineStatus[0].AdcData.Vgen;
                        AdcAmbTemp = (UInt16)st.LineStatus[0].AdcData.AmbTemp;
                        AdcNtcTemp = (UInt16)st.LineStatus[0].AdcData.NtcTemp;
                        AdcDischargeCurr = (UInt16)st.LineStatus[0].AdcData.DischargeCurr;
                        AdcChargeCurr = (UInt16)st.LineStatus[0].AdcData.ChargeCurr;
                        AdcVps = (UInt16)st.LineStatus[0].AdcData.Vps;
                    }
                    LineState = st.LineStatus[0].State;
                    LineError = st.LineStatus[0].Error;
                    ChannleState = (UBA_PROTO_CHANNEL.STATE)st.State;
                    ChannelError = (UBA_PROTO_UBA6.ERROR)st.Error;
                }
            } catch (Exception ex) {
                MessageBox.Show($"Error:{ex}", "Proto Error");
            }
        }

        public void UpdateFromStatusMessage(UBA_PROTO_BPT.status_message st) {
            try {
                UpdateFromStatusMessage(st.ChannelStatus);
                BPT_State = st.State;
                BPT_Error = st.Error;
            } catch (Exception ex) {
                MessageBox.Show($"Error:{ex}", "Proto Error");
            }
        }

        internal void UpdateFromMessage(object? sender, ProtoMessageEventArg e) {
            if (e.Msg.PyloadCase == UBA_MSG.Message.PyloadOneofCase.QueryResponse) {
                if (e.Msg.QueryResponse.StatusCase == UBA_PROTO_QUERY.query_response_message.StatusOneofCase.Channel) {
                    UpdateFromStatusMessage(e.Msg.QueryResponse.Channel);
                } else if (e.Msg.QueryResponse.StatusCase == UBA_PROTO_QUERY.query_response_message.StatusOneofCase.Bpt) {
                    UpdateFromStatusMessage(e.Msg.QueryResponse.Bpt);
                }
            }
        }
    }
}
