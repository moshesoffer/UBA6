using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Animation;
using UBA_PROTO_BPT;
using UBA_PROTO_TR;
using UBA6Library;

namespace UBA6_Controller_App.ViewModel {
    public partial class DischargePageViewModel : ObservableObject {
        
        [ObservableProperty]
        UBA_PROTO_BPT.SOURCE selectedSource;
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_BPT.SOURCE> sourceOptions = new ObservableCollection<UBA_PROTO_BPT.SOURCE>();
        [ObservableProperty]
        int dischargeValue;
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE> dischargeType = new ObservableCollection<UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE>();
        [ObservableProperty]
        UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE selectedDischargeType;
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_CHANNEL.ID> channelOptions = new ObservableCollection<UBA_PROTO_CHANNEL.ID>();
        [ObservableProperty]
        UBA_PROTO_CHANNEL.ID selectedChannel;

        [ObservableProperty]
        UBA_PROTO_BPT.MODE selectedMode;
        [ObservableProperty]
        bool isMinTemp;
        [ObservableProperty]
        float minTemp;
        [ObservableProperty]
        bool isMaxTemp;
        [ObservableProperty]
        float maxTemp = float.MaxValue;

        [ObservableProperty]
        bool isCutOffVoltage;
        [ObservableProperty]
        int cutOffVoltage = int.MaxValue;

        [ObservableProperty]
        bool isMaxTime;
        [ObservableProperty]
        uint maxTime = uint.MaxValue;

        [ObservableProperty]
        bool isDischargeLimit;
        [ObservableProperty]
        int dischargeLimit = int.MaxValue;
        [ObservableProperty]
        byte storeIndex;
        [ObservableProperty]
        UInt16 logInterval = 1;

        private UBA6 model;


        public DischargePageViewModel(UBA6 uba) {
            model = uba;
            foreach (UBA_PROTO_BPT.SOURCE item in Enum.GetValues(typeof(UBA_PROTO_BPT.SOURCE))) {
                SourceOptions.Add(item);
            }
            foreach (UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE item in Enum.GetValues(typeof(UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE))) {
                DischargeType.Add(item);
            }
            foreach (UBA_PROTO_CHANNEL.ID ch in Enum.GetValues(typeof(UBA_PROTO_CHANNEL.ID))) {
                ChannelOptions.Add(ch);
            }
            SelectedChannel = channelOptions.FirstOrDefault();
            SelectedSource = sourceOptions.FirstOrDefault();
            SelectedDischargeType = dischargeType.FirstOrDefault();
        }
        partial void OnMaxTempChanged(float value) {
            IsMaxTemp = value != float.MaxValue;
        }
        partial void OnMinTempChanged(float value) {
            IsMinTemp = value != float.MinValue;
        }
        partial void OnCutOffVoltageChanged(int value) {
            IsCutOffVoltage = value != int.MaxValue;
        }
        partial void OnMaxTimeChanged(uint value) {
            IsMaxTime = value != uint.MaxValue;
        }
        partial void OnDischargeLimitChanged(int value) {
            IsDischargeLimit = value != int.MaxValue;
        }
        partial void OnSelectedChannelChanged(UBA_PROTO_CHANNEL.ID value) {
            SelectedMode = value == UBA_PROTO_CHANNEL.ID.Ab ? UBA_PROTO_BPT.MODE.DualChannel : UBA_PROTO_BPT.MODE.SingleChannel;
        }

        [RelayCommand]
        public void sentDischargeTRMessage() {
            Test_Routine test_Routine = new Test_Routine();
            List<object> list = new List<object>();
            UBA_PROTO_BPT.discharge step = ProtoHelper.CreateDischargeStep(SelectedSource, DischargeValue, SelectedDischargeType,                
                ProtoHelper.CreateDischargeStopCondition(   cutOfVoltage: CutOffVoltage,
                                                            limitCapacity: DischargeLimit,
                                                            maxTime: MaxTime,
                                                            maxTemp: MaxTemp),
                MinTemp);

            list.Add(step);
            model.SentMessage(UBA_Message_Factory.CreateMessage(model.Address, ProtoHelper.CreateTR_Message(StoreIndex, ProtoHelper.CreateTestRoutine(SelectedMode, list, "PC_Disch", LogInterval))));

        }
        [RelayCommand]
        public void storeAndStartDischargeStep() {
            sentDischargeTRMessage();
            UBA_PROTO_BPT.command start = new UBA_PROTO_BPT.command();
            start.BPTListEntery = StoreIndex;
            start.Channel = SelectedChannel;
            start.Id = UBA_PROTO_BPT.CMD_ID.Select;
            model.StartBPT(SelectedChannel, StoreIndex);            
        }

    }
}
