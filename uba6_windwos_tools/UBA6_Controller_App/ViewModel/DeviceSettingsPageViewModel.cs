using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Google.Protobuf.WellKnownTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UBA_MSG;
using UBA6Library;

namespace UBA6_Controller_App.ViewModel {
    public partial class DeviceSettingsPageViewModel : ObservableObject {
        [ObservableProperty]
        string firmware = "XX.XX.XX.XX";
        [ObservableProperty]
        string hardware = "XX.XX";
        [ObservableProperty]
        string name = "Unknoun Device";
        [ObservableProperty]
        UInt32 sN;
        [ObservableProperty]
        UInt32 address2Set;
        [ObservableProperty]
        UInt32 address;
        [ObservableProperty]
        UInt32 unixTimestamp;
        [ObservableProperty]
        UBA_PROTO_UBA6.ERROR error = UBA_PROTO_UBA6.ERROR.NoError;
        [ObservableProperty]
        bool isBuzzerEnabled = false;
        UBA6 model;
        public DeviceSettingsPageViewModel(UBA6 model) {
            this.model = model;
            model.MessageReceived += Model_MessageReceived;
        }

        private void Model_MessageReceived(object? sender, ProtoMessageEventArg e) {
            if (e.Msg.PyloadCase == UBA_MSG.Message.PyloadOneofCase.QueryResponse) {
                if (e.Msg.QueryResponse.StatusCase == UBA_PROTO_QUERY.query_response_message.StatusOneofCase.Device) {
                    Name = e.Msg.QueryResponse.Device.Settings.Name;
                    SN = e.Msg.QueryResponse.Device.Settings.SN;                    
                    Address = e.Msg.QueryResponse.Device.Settings.Address;
                    IsBuzzerEnabled = e.Msg.QueryResponse.Device.Settings.Buzzer >0;
                }
            }
        }
        partial void OnAddress2SetChanged(UInt32 value) {
            Message msg = UBA_Message_Factory.CreateMessage(model.Address,ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Addresss, value));
            model.SentMessage(msg);
            model.Address = (1u << (int)value);
            Address = model.Address;
        }
        partial void OnIsBuzzerEnabledChanged(bool value) {
            UBA_PROTO_UBA6.command cmd =  ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Buzzer, value ? 1u : 0u);
            Message msg = UBA_Message_Factory.CreateMessage(model.Address, cmd);
            model.SentMessage(msg);
        }
        partial void OnNameChanged(string value) {
            UBA_PROTO_UBA6.command cmd = ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Name,name:value);
            Message msg = UBA_Message_Factory.CreateMessage(model.Address, cmd);
            model.SentMessage(msg);            

        }
        partial void OnSNChanged(uint value) {
            UBA_PROTO_UBA6.command cmd = ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Sn, value: value);
            Message msg = UBA_Message_Factory.CreateMessage(model.Address, cmd);
            model.SentMessage(msg);            
        }

        [RelayCommand]
        public async Task Refresh() {            
             Message? m = await model.UBA_Interface.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device, 0xffffffff);
            if (m != null) {
                if (m.PyloadCase == UBA_MSG.Message.PyloadOneofCase.QueryResponse) {
                    if (m.QueryResponse.StatusCase == UBA_PROTO_QUERY.query_response_message.StatusOneofCase.Device) {
                        Name = m.QueryResponse.Device.Settings.Name;
                        SN = m.QueryResponse.Device.Settings.SN;
                        model.Address = m.QueryResponse.Device.Settings.Address;
                        Address = model.Address;
                        Address2Set = (UInt32)Math.Log(Address, 2);
                        IsBuzzerEnabled = m.QueryResponse.Device.Settings.Buzzer > 0;                        
                    }
                }
            }
        }
        [RelayCommand]
        public void setRTC() { 
            UnixTimestamp = (UInt32)(DateTimeOffset.UtcNow.ToUnixTimeSeconds());
        }
        [RelayCommand]
        public void ApplyRTC() {
            UBA_PROTO_UBA6.command cmd = ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Time, value: UnixTimestamp);
            Message msg = UBA_Message_Factory.CreateMessage(model.Address, cmd);
            model.SentMessage(msg);          
        }

        [RelayCommand]
        public void Reboot() {
            UBA_PROTO_UBA6.command cmd = ProtoHelper.CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID.Boot);
            Message msg = UBA_Message_Factory.CreateMessage(model.Address, cmd);
            model.SentMessage(msg);            
        }
    }
}
