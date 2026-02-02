using BK_PRECISION9104Libary;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using KeithleyDMM6500Library;
using System.Collections.ObjectModel;
using System.IO.Ports;
using System.Windows.Media;
using UBA6Library;


namespace UBA6_Controller_App.ViewModel {
    public partial class MainWindowViewModel : ObservableObject {
        private UBA6 uba;

        [ObservableProperty]
        string portStr = String.Empty;
        [ObservableProperty]
        int index = 0;

        [ObservableProperty]
        ObservableCollection<string> ports = new ObservableCollection<string>();
        [ObservableProperty]
        string selectedUBAPort = string.Empty;
        [ObservableProperty]
        string selectedPowerSupplyPort = string.Empty;
        [ObservableProperty]
        string selectedLoadCellPort = string.Empty;
        [ObservableProperty]
        string title = String.Empty;

        [ObservableProperty]
        uint bPTListEntery = 0;

        [ObservableProperty]
        ObservableCollection<UBA_PROTO_CHANNEL.ID> channeNames = new ObservableCollection<UBA_PROTO_CHANNEL.ID>();

        [ObservableProperty]
        UBA_PROTO_CHANNEL.ID selectedChannel = UBA_PROTO_CHANNEL.ID.None;

        [ObservableProperty]
        string status = "N/A";
        [ObservableProperty]
        int progress = 0;
        [ObservableProperty]
        DateTime statusTime = DateTime.Now;
        [ObservableProperty]
        Brush fillColor = Brushes.Pink;
        [ObservableProperty]
        bool emulationMode = false;
        [ObservableProperty]
        uint avgCount = 1;
        [ObservableProperty]
        uint measurementDelayMs = 1;
        [ObservableProperty]
        uint pS_DelayMs = 0;
        [ObservableProperty]
        float tolerancePercentage = 5.0f;
        [ObservableProperty]
        uint maxVoltage = 5000; // in mV, default value for UBA6
        [ObservableProperty]
        uint maxDischargeCurrent = 3000; // in mA, default value for UBA6    
        [ObservableProperty]
        uint maxChargeCurrent = 3000; // in mA, default value for UBA6    

        private UBA6 UBA;
        private BK_PRECISION9104 PowerSupply;
        private KeithleyDMM6500 MultiMeter;
        private KelDeviceController LoadCell;
        private Calibration.Calibration Calibration;


        public MainWindowViewModel() : base() {

            SearchAndSetPorts();
            foreach (UBA_PROTO_CHANNEL.ID item in Enum.GetValues(typeof(UBA_PROTO_CHANNEL.ID))) {
                ChanneNames.Add(item);
            }
            SelectedChannel = ChanneNames.First();
            Title = $"UBA {uba?.UBA_Interface?.ToString() ?? "N/A"}";
        }

        public MainWindowViewModel(UBA6 uba, BK_PRECISION9104 ps, KeithleyDMM6500 mm, KelDeviceController lc) : this() {
            this.uba = uba;
            this.PowerSupply = ps;
            this.MultiMeter = mm;
            this.LoadCell = lc;
            uba.StatusChanged += StatusChanged;
            uba.ExceptionOccurred += ExceptionOccurred;
            uba.MessageReceived += Uba_MessageReceived;
            ps.StatusChanged += StatusChanged;
            ps.ExceptionOccurred += ExceptionOccurred;
            mm.StatusChanged += StatusChanged;
            mm.ExceptionOccurred += ExceptionOccurred;
            lc.StatusChanged += StatusChanged;
            lc.ExceptionOccurred += ExceptionOccurred;

            string com = uba?.UBA_Interface?.PortName ?? "N/A";
            string? found = Ports.FirstOrDefault(item => item == com);
            SelectedUBAPort = found;
            com = PowerSupply?.SerialPort?.PortName ?? "N/A";
            found = Ports.FirstOrDefault(item => item == com);
            SelectedPowerSupplyPort = found;
            com = LoadCell?.SerialPort?.PortName ?? "N/A";
            found = Ports.FirstOrDefault(item => item == com);
            SelectedLoadCellPort = found;

        }

        private void Uba_MessageReceived(object? sender, ProtoMessageEventArg e) {
            if (e.Msg.PyloadCase == UBA_MSG.Message.PyloadOneofCase.QueryResponse) {
                if (e.Msg.QueryResponse.StatusCase == UBA_PROTO_QUERY.query_response_message.StatusOneofCase.Device) {
                    Title = $"UBA {uba?.UBA_Interface?.PortName ?? "N/A"} - SN:{e.Msg.QueryResponse.Device.Settings.SN}";
                }
            }
        }

        public MainWindowViewModel(Calibration.Calibration cal) : this(cal.UBA, cal.PowerSupply, cal.MultiMeter, cal.LoadCell) {
            Calibration = cal;
            EmulationMode = Properties.Settings.Default.EmulationMode;
            AvgCount = Properties.Settings.Default.AvgCount;
            MeasurementDelayMs = Properties.Settings.Default.MeasurementDelayMs;
            PS_DelayMs = Properties.Settings.Default.PS_DelayMs;
            TolerancePercentage = Properties.Settings.Default.TolerancePercentage;
            MaxVoltage = Properties.Settings.Default.UBA_MaxVoltage;
            MaxDischargeCurrent = Properties.Settings.Default.UBA_MaxDischargeCurrent;
            MaxChargeCurrent = Properties.Settings.Default.UBA_MaxChargeCurrent;

        }
        partial void OnEmulationModeChanged(bool value) {
            Calibration.IsInEmulationMode = value;
            Properties.Settings.Default.EmulationMode = value;
            Properties.Settings.Default.Save();
        }
        partial void OnAvgCountChanged(uint value) {
            Calibration.AvgCount = value;
            Properties.Settings.Default.AvgCount = value;
            Properties.Settings.Default.Save();
        }
        partial void OnMeasurementDelayMsChanged(uint value) {
            Calibration.MesuremntDelay = TimeSpan.FromMilliseconds(value);
            Properties.Settings.Default.MeasurementDelayMs = value;
            Properties.Settings.Default.Save();
        }
        partial void OnPS_DelayMsChanged(uint value) {
            Calibration.PS_Delay = TimeSpan.FromMilliseconds(value);
            Properties.Settings.Default.PS_DelayMs = value;
            Properties.Settings.Default.Save();
        }

        partial void OnTolerancePercentageChanged(float value) {
            BK_PRECISION9104.TolerancePercentage = value;
            Properties.Settings.Default.TolerancePercentage = value;
            Properties.Settings.Default.Save();
        }

        partial void OnMaxVoltageChanged(uint value) {
            Calibration.MaxVoltage = value;
        }
        partial void OnMaxDischargeCurrentChanged(uint value) {
            Calibration.MaxDischargeCurrent = value;
        }
        partial void OnMaxChargeCurrentChanged(uint value) {
            Calibration.MaxChargeCurrent = value;
        }




        partial void OnSelectedUBAPortChanged(string? oldValue, string newValue) {
            if (newValue == null) {
                return;
            }
            uba.UBA_Interface.SwitchCom(newValue);
            Properties.Settings.Default.UBA_COM = newValue;
            Properties.Settings.Default.Save();
            Title = $"UBA {uba?.UBA_Interface?.PortName ?? "N/A"}";


        }
        partial void OnSelectedPowerSupplyPortChanged(string? oldValue, string newValue) {
            if (newValue == null) {
                return;
            }
            PowerSupply.SetPort(newValue);
            Properties.Settings.Default.PS_COM = newValue;
            Properties.Settings.Default.Save();
        }

        partial void OnSelectedLoadCellPortChanging(string? oldValue, string newValue) {
            if (newValue == null) {
                return;
            }
            LoadCell.SetPort(newValue);
            Properties.Settings.Default.LoadCell_COM = newValue;
            Properties.Settings.Default.Save();
        }


        public void SearchAndSetPorts() {
            Ports.Clear();
            string[] ports = SerialPort.GetPortNames();
            foreach (string port in ports) {
                Ports.Add(port);
            }
        }

        private void ExceptionOccurred(object? sender, AmicellUtil.ExceptionEventArg e) {
            Status = e.Exception.Message;
            StatusTime = DateTime.Now;
            FillColor = Brushes.Red;
        }

        private void StatusChanged(object? sender, AmicellUtil.StatusEventArg e) {
            Status = e.Status;
            StatusTime = e.Timestamp;
            FillColor = Brushes.Green;

            if (e.Progress.HasValue) {
                Progress = (int)e.Progress.Value;
            }
        }

        [RelayCommand]
        public void SendCalibrationData() {
            uba.SentMessage(UBA_Message_Factory.CreateMessage(uba.Address, uba.CalDataLineA.CreateProtoMessage(), uba.CalDataLineB.CreateProtoMessage()));
        }
        [RelayCommand]
        public async Task Query() {
            await uba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.Device);
            await uba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.LineA);
            await uba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.LineB);
            await uba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.BptA);
            await uba.GetMessage(UBA_PROTO_QUERY.RECIPIENT.BptB);
        }
        [RelayCommand]
        public void StopBPT() {
            uba.StopBPT(SelectedChannel);
        }
        [RelayCommand]
        public void StartBPT() {
            uba.StartBPT(SelectedChannel, BPTListEntery);
        }
        [RelayCommand]
        public async Task DisableCalibration() {
            uba.SentMessage(UBA_Message_Factory.CreateMessage(uba.Address, ProtoHelper.CreateLineCommand(UBA_PROTO_LINE.CMD_ID.Calibration, UBA_PROTO_LINE.ID.A, 0)));
            await uba.UBA_Interface.WaitForQueueToBeEmptyAsync();
            uba.SentMessage(UBA_Message_Factory.CreateMessage(uba.Address, ProtoHelper.CreateLineCommand(UBA_PROTO_LINE.CMD_ID.Calibration, UBA_PROTO_LINE.ID.B, 0)));
            await uba.UBA_Interface.WaitForQueueToBeEmptyAsync();

        }
    }
}
