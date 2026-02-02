using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters
{
    class SpanTimeConverter : IValueConverter {
        // Convert timespan to string 
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is TimeSpan ts) {
                return ts.ToString(@"hh\:mm\:ss");
            }

            return "N/A";
        }

        // Not used
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}
