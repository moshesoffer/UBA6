using BK_PRECISION9104Libary;
using Grpc.Core;
using KeithleyDMM6500Library;
using Microsoft.Extensions.Logging;
using System.IO.Ports;
using System.Windows;
using System.Windows.Controls;

using UBA6_Controller_App.View;
using UBA6_Controller_App.ViewModel;
using UBA6Library;
using static Calibration.Calibration;

namespace UBA6_Controller_App {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        private string comstr = string.Empty;
        private readonly ILogger<MainWindow> _logger;
        public Array StatusList => Enum.GetValues(typeof(UBA_PROTO_CHANNEL.ID));
        private ChannelPage pageChannelA = new ChannelPage();
        private ChannelPage pageChannelB = new ChannelPage();
        private LineClibrationPage LineCalA = new LineClibrationPage();
        private LineClibrationPage LineCalB = new LineClibrationPage();

        private DischargePage DischargePage = new DischargePage();
        private ChargePage ChargePage = new ChargePage();
        private DelayPage DelayPage = new DelayPage();
        private DeviceSettingsPage DeviceSettingsPage = new DeviceSettingsPage();
        private FileManagerPage FileManagerPage = new FileManagerPage();
        private CommandPage CommandPage = new CommandPage();
        private DischargePageViewModel DischargePageVM;


        private ChannelViewModel chA_VM = new ChannelViewModel();
        private ChannelViewModel chB_VM = new ChannelViewModel();
        private MainWindowViewModel MainWindowVM = new MainWindowViewModel();

        private BK_Precision9104Window PS_Window = new BK_Precision9104Window();

        private UBA6 uba;
        public BK_PRECISION9104 PowerSupply;
        public KeithleyDMM6500 MultiMeter;
        public KelDeviceController LoadCell;
        public Calibration.Calibration Calibration { get; set; }
        public MainWindow() {
            using var loggerFactory = LoggerFactory.Create(builder => {
                builder.AddConsole(); // Add console logging
                builder.SetMinimumLevel(LogLevel.Debug);// Set the minimum log level to Debug

            });
            ILogger<Calibration.Calibration> logger = loggerFactory.CreateLogger<Calibration.Calibration>();
            ILogger<UBA6> ubaLogger = loggerFactory.CreateLogger<UBA6>();
            ILogger<BK_PRECISION9104> BK_logger = loggerFactory.CreateLogger<BK_PRECISION9104>();
            ILogger<KelDeviceController> Kel_logger = loggerFactory.CreateLogger<KelDeviceController>();
            ILogger<KeithleyDMM6500> MM_logger = loggerFactory.CreateLogger<KeithleyDMM6500>();
            ILogger<UBA_Interface> intreface_logger = loggerFactory.CreateLogger<UBA_Interface>();

            string deviceAddress = Properties.Settings.Default.UBA_COM?.ToString() ?? "COM1";
            UBA_Interface uBA_Interface = new UBA_Interface(intreface_logger, deviceAddress);
            uba = new UBA6(ubaLogger, uBA_Interface);
            deviceAddress = Properties.Settings.Default.MultimeterName?.ToString() ?? "COM1";
            MultiMeter = new KeithleyDMM6500(MM_logger,deviceAddress);
            deviceAddress = Properties.Settings.Default.LoadCell_COM?.ToString() ?? "COM1";
            LoadCell = new KelDeviceController(Kel_logger,deviceAddress);
            deviceAddress = Properties.Settings.Default.PS_COM?.ToString() ?? "COM1";
            PowerSupply = new BK_PRECISION9104(BK_logger,deviceAddress);
            InitializeComponent();
            BK_PRECISION9104.TolerancePercentage = Properties.Settings.Default.TolerancePercentage;
            uint calibrationAvgRepeats = Properties.Settings.Default.AvgCount;
            TimeSpan timeSpan = TimeSpan.FromSeconds(Properties.Settings.Default.MeasurementDelayMs);
            Calibration = new Calibration.Calibration(logger,uba, PowerSupply, MultiMeter, LoadCell, timeSpan,calibrationAvgRepeats);
            MainWindowVM = new MainWindowViewModel(Calibration);
        //  MainWindowVM = new MainWindowViewModel(uba, PowerSupply, MultiMeter, LoadCell);

            this.DataContext = MainWindowVM;
            //            ComboBox_ch.ItemsSource = Enum.GetValues(typeof(UBA_PROTO_CHANNEL.ID));
            Frame_channel_A.Navigate(pageChannelA);
            Frame_channel_B.Navigate(pageChannelB);
            pageChannelA.DataContext = chA_VM;
            pageChannelB.DataContext = chB_VM;
            chA_VM.ChannelID = UBA_PROTO_CHANNEL.ID.A;
            chB_VM.ChannelID = UBA_PROTO_CHANNEL.ID.B;
            uba.MessageReceived += chA_VM.UpdateFromMessage;
            uba.MessageReceived += chB_VM.UpdateFromMessage;
            LineCalA.DataContext = new LineClibrationViewModel(Calibration, UBA_PROTO_LINE.ID.A);
            LineCalB.DataContext = new LineClibrationViewModel(Calibration, UBA_PROTO_LINE.ID.B);
            Frame_Calibration_A.Navigate(LineCalA);
            Frame_Calibration_B.Navigate(LineCalB);

            ChargePage.DataContext = new ChargePageViewModel(uba);
            DischargePage.DataContext = new DischargePageViewModel(uba);
            DelayPage.DataContext = new DelayPageViewModel(uba);
            DeviceSettingsPage.DataContext = new DeviceSettingsPageViewModel(uba);
            FileManagerPage.DataContext = new FileManagerPageViewModel(uba);

            Frame_Discharge.Navigate(DischargePage);
            Frame_Charge.Navigate(ChargePage);
            Frame_Delay.Navigate(DelayPage);
            Frame_DeviceSettings.Navigate(DeviceSettingsPage);
            Frame_FM.Navigate(FileManagerPage);
            Frame_Commands.Navigate(CommandPage);


#if DEBUG
            if (AmicellUtil.Util.IsComPortExists(Properties.Settings.Default.UBA_COM)) {                
                uba.UBA_Interface.SwitchCom( Properties.Settings.Default.UBA_COM);
            }
#endif      
        }
        public MainWindow(ILogger<MainWindow> logger) : this() {
            _logger = logger;
            _logger.LogInformation("MainWindow initialized at {Time}", DateTime.Now);
        }
        
        private void ComPortMenuItem_SubmenuOpened(object sender, RoutedEventArgs e) {
            //  MainWindowVM.SearchAndSetPorts();          

        }

        private void RadioButton_Checked(object sender, RoutedEventArgs e) {
            try {
                //Frame_TR.Navigate(new ChargePage(uba));

            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            }
        }

        private async void CalibrateAllSelected(object sender, RoutedEventArgs e) {
            try {
                LineClibrationViewModel.SetBusy(true);
                List<VoltageCalibration> var = await this.Calibration.CalibrateAllVoltages(((LineClibrationViewModel)LineCalA.DataContext).GetSelectedVoltages(), ((LineClibrationViewModel)LineCalB.DataContext).GetSelectedVoltages() );
                ((LineClibrationViewModel)LineCalA.DataContext).SetUI_FromTest(var);
                ((LineClibrationViewModel)LineCalB.DataContext).SetUI_FromTest(var);
                List<TempCalibration> tempCal = await this.Calibration.CalibrateAllTemp(((LineClibrationViewModel)LineCalA.DataContext).GetSelectedTemp(), ((LineClibrationViewModel)LineCalA.DataContext).GetSelectedTemp());
                ((LineClibrationViewModel)LineCalA.DataContext).SetUI_FromTest(tempCal);
                ((LineClibrationViewModel)LineCalB.DataContext).SetUI_FromTest(tempCal);
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                LineClibrationViewModel.SetBusy(false);
            }
        }
    }
}