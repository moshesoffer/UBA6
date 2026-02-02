using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters {
    class TempConverter : IValueConverter {
        // Convert float temperature to string with appropriate unit
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is float temp) {
                return $"{temp.ToString("000.00")}° C";
            }
            return "Invalid";
        }

        // Not used
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}
