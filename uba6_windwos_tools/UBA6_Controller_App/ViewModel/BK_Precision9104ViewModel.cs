using BK_PRECISION9104Libary;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Google.Protobuf.WellKnownTypes;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Windows.Media;
using static BK_PRECISION9104Libary.BK_PRECISION9104;

namespace UBA6_Controller_App.ViewModel {
    public partial class BK_Precision9104ViewModel: ObservableObject {
        private BK_PRECISION9104 model;
        [ObservableProperty]
        string title = "BK PRECISION 9104";
        [ObservableProperty]
        bool isCC_Mode;
        [ObservableProperty]
        bool isCV_Mode;
        [ObservableProperty]
        int voltage;
        [ObservableProperty]
        int current;
        [ObservableProperty]
        int uVL;
        [ObservableProperty]
        int uCL;
        [ObservableProperty]
        bool isOutputOn;
        [ObservableProperty]
        bool isAutoRead;
        [ObservableProperty]
        bool isEditable =true;
        [ObservableProperty]
        ObservableCollection<bool> isPresetSelect = new ObservableCollection<bool>() { false,false,false, true };
        [ObservableProperty]
        ObservableCollection<int> presetVoltage = new ObservableCollection<int>() { 0,1,2,3};
        [ObservableProperty]
        ObservableCollection<int> presetCurrent = new ObservableCollection<int>() { 4, 5, 6,7 };

        [ObservableProperty]
        string status = "N/A";
        [ObservableProperty]
        int progress = 0;
        [ObservableProperty]
        DateTime statusTime = DateTime.Now;
        [ObservableProperty]
        Brush fillColor = Brushes.Pink;

        public BK_Precision9104ViewModel(BK_PRECISION9104 m ) { 
            this.model = m;
            this.model.ReadingReceived += Model_ReadingReceived;
            this.model.StatusChanged += Model_StatusChanged;
            this.model.ExceptionOccurred += Model_ExceptionOccurred;
            model.Getinformation().ContinueWith(t =>
            {
                if (t.IsFaulted) {
                    // Handle error if needed
                    Console.WriteLine(t.Exception);
                    return;
                }               
                UpdateVM();
            });
        }

        private void Model_ExceptionOccurred(object? sender, AmicellUtil.ExceptionEventArg e) {
            Status = e.Exception.Message;
            StatusTime = DateTime.Now;
            FillColor = Brushes.Red;            
        }

        private void Model_StatusChanged(object? sender, AmicellUtil.StatusEventArg e) {
            Status = e.Status;
            StatusTime = e.Timestamp;
            FillColor = Brushes.Green;
        }

        private void Model_ReadingReceived(object? sender, ReadingEventArgs e) {
            Voltage = e.Voltage;
            Current= e.Current;
            IsCC_Mode = (e.mode == CV_CC_Mode.CC_MODE);
            IsCV_Mode = (e.mode == CV_CC_Mode.CV_MODE);
        }

        public async Task SetPreset() { 
            for(int i = 0; i< isPresetSelect.Count; i++){
                if (isPresetSelect[i]) {
                    await model.SetABC_Select((BK_PRECISION9104.ABC_PRESET)i);
                }
            }            
        }

        [RelayCommand]
        public async Task ConfigPreset() {
            for (int i = 0; i < isPresetSelect.Count; i++) {
                if (isPresetSelect[i]) {
                    
                    await model.SetABC_Select((BK_PRECISION9104.ABC_PRESET)i);
                }
            }
        }

        partial void OnVoltageChanged(int value) {
            bool temp = model.IsAutoRead;
            model.StopAutoRead();
            model.SetABC_Select(ABC_PRESET.NORMAL);
            model.SetOutputVoltage(ABC_PRESET.NORMAL, value);
            if (temp) { 
                model.StartAutoRead();
            }            
        }

        partial void OnUVLChanged(int value) {
            model.SetOverVoltage(value);
        }

        partial void OnUCLChanged(int value) {
            model.SetOverCurrent(value);
        }

        partial void OnCurrentChanged(int value) {
            model.SetOutputCurrnt(ABC_PRESET.NORMAL, value);
        }
        partial void OnIsOutputOnChanged(bool oldValue, bool newValue) {
            model.SetOutput(newValue);
        }

        partial void OnIsAutoReadChanged(bool value) {
            IsEditable = !value;
            if (value) {
                model.StartAutoRead();
            } else { 
                model.StopAutoRead();
            }
        }

        partial void OnPresetVoltageChanged(ObservableCollection<int> value) {
            model.PreSetValues[0].Voltage = value[0];
            model.PreSetValues[0].Voltage = value[1];
            model.PreSetValues[0].Voltage = value[2];
            model.ConfigPreset(model.PreSetValues);
        }
        [RelayCommand]
        public async Task QueryAll() {
            try {
                await model.Getinformation();
                await model.GetPreSetSelection();
                await model.GetReadingVoltCurrAndMode();
                await UpdateVM();
            } catch (Exception ex) { 
                Debug.Write(ex);
            }
        }

        [RelayCommand]
        public async Task UpdateVM() { 
            Title = model.DeviceName();
            IsCC_Mode = (model.CV_CC_mode == BK_PRECISION9104.CV_CC_Mode.CC_MODE);
            IsCV_Mode = (model.CV_CC_mode == BK_PRECISION9104.CV_CC_Mode.CV_MODE);
            Voltage = model.Voltage;
            Current = model.Current;
            UVL = model.UVL;
            UCL= model.UCL;
            IsOutputOn = model.IsOutput;
            IsAutoRead = model.IsAutoRead;
                        
            for (int index = 0; index< model.PreSetValues.Count; index++) {
                PresetCurrent[index] = model.PreSetValues[index].Current;
                PresetVoltage[index] = model.PreSetValues[index].Voltage;
                IsPresetSelect[index] = false;                
            }
            if ((int)model.Preset < IsPresetSelect.Count) {
                IsPresetSelect[(int)model.Preset] = true;
            }

        }

        private async Task SetPresetConfig(ABC_PRESET preset) {
            try {
                await model.SetPresetVoltageAndCuurent(preset, PresetVoltage[(int)preset], PresetCurrent[(int)preset]);
                await model.GetPresetVoltageAndCuurent(preset);
                PresetVoltage[(int)preset] = model.PreSetValues[(int)preset].Voltage;
                PresetCurrent[(int)preset] = model.PreSetValues[(int)preset].Current;
            } catch (Exception ex) { 
                Console.WriteLine(ex.ToString());
            }

        }

        [RelayCommand]
        public async Task SetPreset1Config() {
            await SetPresetConfig(ABC_PRESET.A);
        }
        [RelayCommand]
        public async Task SetPreset2Config() {
            await SetPresetConfig(ABC_PRESET.B);
        }

        [RelayCommand]
        public async Task SetPreset3Config() {
            await SetPresetConfig(ABC_PRESET.C);
        }


    }
}
