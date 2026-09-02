using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UBA_PROTO_BPT;
using UBA_PROTO_CHANNEL;
using UBA_PROTO_TR;
using UBA6Library;

namespace UBA6_Controller_App.ViewModel {
    public partial class ChargePageViewModel : ObservableObject {
        [ObservableProperty]
        UBA_PROTO_BPT.SOURCE selectedSource;
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_BPT.SOURCE> sourceOptions = new ObservableCollection<UBA_PROTO_BPT.SOURCE>();
        [ObservableProperty]
        UBA_PROTO_CHANNEL.ID selectedChannel;
        [ObservableProperty]
        UBA_PROTO_BPT.MODE selectedMode ;
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_CHANNEL.ID> channelOptions = new ObservableCollection<UBA_PROTO_CHANNEL.ID>();

        [ObservableProperty]
        int chargeCurrent;
        [ObservableProperty]
        int chargeVoltage;
       
        [ObservableProperty]
        bool isMinTemp;
        [ObservableProperty]
        float minTemp;
        [ObservableProperty]
        bool isMaxTemp;
        [ObservableProperty]
        float maxTemp = float.MaxValue;

        [ObservableProperty]
        bool isCutOffCurrent;
        [ObservableProperty]
        int cutOffCurrent = ProtoHelper.DEFAULT_CHARGE_CUTOFF_CURRENT;

        [ObservableProperty]
        bool isMaxTime;
        [ObservableProperty]
        uint maxTime = uint.MaxValue;

        [ObservableProperty]
        bool isDischargeLimit;
        [ObservableProperty]
        int dischargeLimit = int.MaxValue;
        [ObservableProperty]
        byte storeIndex ;
        [ObservableProperty]
        UInt16 logInterval = 1;

        private UBA6 model;


        public ChargePageViewModel(UBA6 uba) {
            model = uba;
            foreach (UBA_PROTO_BPT.SOURCE item in Enum.GetValues(typeof(UBA_PROTO_BPT.SOURCE))) {
                SourceOptions.Add(item);
            }
            SelectedSource = sourceOptions.FirstOrDefault();
            foreach (UBA_PROTO_CHANNEL.ID ch in Enum.GetValues(typeof(UBA_PROTO_CHANNEL.ID))) {
                ChannelOptions.Add(ch);
            }
            SelectedChannel = channelOptions.FirstOrDefault();
        }
        partial void OnSelectedChannelChanged(ID value) {
            SelectedMode  = value == UBA_PROTO_CHANNEL.ID.Ab ? UBA_PROTO_BPT.MODE.DualChannel: UBA_PROTO_BPT.MODE.SingleChannel;
        }
        partial void OnMaxTempChanged(float value) {
            IsMaxTemp = value != float.MaxValue;
        }
        partial void OnMinTempChanged(float value) {
            IsMinTemp = value != float.MinValue;
        }
        partial void OnCutOffCurrentChanged(int value) {
            IsCutOffCurrent = (value != int.MaxValue);
        }
        partial void OnMaxTimeChanged(uint value) {
            IsMaxTime = value != uint.MaxValue;
        }
        partial void OnDischargeLimitChanged(int value) {
            IsDischargeLimit = value != int.MaxValue;
        }


        [RelayCommand]
        public void sentChargeTRMessage() {
            Test_Routine test_Routine = new Test_Routine();
            List<object> list = new List<object>();
            UBA_PROTO_BPT.charge  step = ProtoHelper.CreateChargeStep(SelectedSource, ChargeCurrent, ChargeVoltage,
                ProtoHelper.CreateChargeStopCondtion(cutOffCurrent: CutOffCurrent,
                                                    limitCapacity: DischargeLimit,
                                                    maxTime: MaxTime,
                                                    maxTemp: MaxTemp),
                MinTemp);
            list.Add(step);            
            model.SentMessage(UBA_Message_Factory.CreateMessage(model.Address, ProtoHelper.CreateTR_Message(StoreIndex, ProtoHelper.CreateTestRoutine(SelectedMode, list, "PC_Charge", LogInterval))));

        }
        [RelayCommand]
        public void storeAndStartChargeStep() {
            sentChargeTRMessage();
            model.StartBPT(SelectedChannel, StoreIndex);
            
        }
    }
}
