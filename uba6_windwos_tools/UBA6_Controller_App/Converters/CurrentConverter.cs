using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters
{
    public class CurrentConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is int milliamps) {
                if (Math.Abs(milliamps) >= 1000) {
                    double amps = milliamps / 1000.0;
                    return $"{amps:0.###} A";
                } else {
                    return $"{milliamps} mA";
                }
            }
            return "Invalid";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}
