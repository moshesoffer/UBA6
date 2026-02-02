using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters
{
    public class CapacityConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is float milliampHours) {
                if (Math.Abs(milliampHours) >= 1000) {
                    double ampHours = milliampHours / 1000.0;
                    return $"{ampHours:0.###} Ah";
                } else {
                    return $"{milliampHours} mAh";
                }
            }
            return "Invalid";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}
