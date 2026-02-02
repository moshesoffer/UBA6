using System.Globalization;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters {
    public class VoltageConverter : IValueConverter {
        // Convert int (mV) to string with appropriate unit
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is int millivolts) {
                if (Math.Abs(millivolts) >= 1000) {
                    double volts = millivolts / 1000.0;
                    return $"{volts:000.###} V";
                } else {
                    
                    return $"{(double) millivolts:000.000###} mV";
                }
            }

            return "Invalid";
        }

        // Not used
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}
