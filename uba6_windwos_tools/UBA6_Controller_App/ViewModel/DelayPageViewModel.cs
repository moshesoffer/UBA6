using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UBA_PROTO_BPT;
using UBA_PROTO_TR;
using UBA6Library;

namespace UBA6_Controller_App.ViewModel {
    public partial class DelayPageViewModel : ObservableObject {
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_BPT.SOURCE> sourceOptions = new ObservableCollection<UBA_PROTO_BPT.SOURCE>();
        [ObservableProperty]
        UBA_PROTO_BPT.SOURCE selectedSource;
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_CHANNEL.ID> channelOptions = new ObservableCollection<UBA_PROTO_CHANNEL.ID>();
        [ObservableProperty]
        UBA_PROTO_CHANNEL.ID selectedChannel;
        [ObservableProperty]
        UBA_PROTO_BPT.MODE selectedMode;
        [ObservableProperty]
        byte storeIndex;
        [ObservableProperty]
        bool isWaitTemp;
        [ObservableProperty]
        float waitTemp = float.MinValue;
        [ObservableProperty]
        bool isDelayTime;
        [ObservableProperty]
        uint delayTime = 0;
        [ObservableProperty]
        UInt16 logInterval = 1;

        private UBA6 model;
        public DelayPageViewModel(UBA6 model) {
            this.model = model;
            foreach (UBA_PROTO_BPT.SOURCE item in Enum.GetValues(typeof(UBA_PROTO_BPT.SOURCE))) {
                SourceOptions.Add(item);
            }
            SelectedSource = sourceOptions.FirstOrDefault();
            foreach (UBA_PROTO_CHANNEL.ID ch in Enum.GetValues(typeof(UBA_PROTO_CHANNEL.ID))) {
                ChannelOptions.Add(ch);
            }
            SelectedChannel = channelOptions.FirstOrDefault();
        }

        partial void OnSelectedChannelChanged(UBA_PROTO_CHANNEL.ID value) {
            SelectedMode = value == UBA_PROTO_CHANNEL.ID.Ab ? UBA_PROTO_BPT.MODE.DualChannel : UBA_PROTO_BPT.MODE.SingleChannel;
        }
        partial void OnWaitTempChanged(float value) {
            IsWaitTemp = value != -273.15;
        }

        partial void OnDelayTimeChanged(uint value) {
            IsDelayTime = value != 0;
        }

        [RelayCommand]
        public void StoreDelayTR() {
            try {
                Test_Routine test_Routine = new Test_Routine();
                List<object> list = new List<object>();
                UBA_PROTO_BPT.delay step = ProtoHelper.CreateDelayStep(DelayTime, WaitTemp);
                UBA_PROTO_BPT.delay step2 = ProtoHelper.CreateDelayStep(DelayTime, WaitTemp);
                list.Add(step);
                list.Add(step2);
                model.SentMessage(UBA_Message_Factory.CreateMessage(model.Address, ProtoHelper.CreateTR_Message(StoreIndex, ProtoHelper.CreateTestRoutine(SelectedMode, list, "PC_Delay", LogInterval))));
            } catch(Exception e) {
                System.Windows.MessageBox.Show(e.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            }

        }

        [RelayCommand]
        public void StoreAndStartDelayTR() {
            StoreDelayTR();
            model.StartBPT(SelectedChannel, StoreIndex);            
        }
    }
}
