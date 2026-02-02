using BK_PRECISION9104Libary;
using Microsoft.Extensions.Logging;
using System.Windows;

namespace UBA6_Controller_App {
    /// <summary>
    /// Interaction logic for BK_Precision9104Window.xaml
    /// </summary>
    public partial class BK_Precision9104Window : Window {
        private ViewModel.BK_Precision9104ViewModel vm ;
        private BK_PRECISION9104 device;
        public BK_Precision9104Window() {
            InitializeComponent();            
        }

        public BK_Precision9104Window(string ComPort)  :this(){
            var loggerFactory = LoggerFactory.Create(builder =>
            {
                builder.AddConsole();
                builder.SetMinimumLevel(LogLevel.Debug);
            });

            ILogger<BK_PRECISION9104> logger = loggerFactory.CreateLogger<BK_PRECISION9104>();

            device = new BK_PRECISION9104(logger,ComPort);
            vm = new ViewModel.BK_Precision9104ViewModel(device);
            this.DataContext = vm;
        }

        private void ToggleSwitchOutput_Toggled(object sender, RoutedEventArgs e) {
            
        }

        private void RadioButton_PresetA_Checked(object sender, RoutedEventArgs e) {
            vm.SetPreset();

        }
    }
}
