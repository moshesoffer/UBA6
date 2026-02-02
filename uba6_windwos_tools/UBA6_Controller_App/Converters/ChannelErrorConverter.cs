using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters
{
    class ChannelErrorConverter : IValueConverter {
        // Convert the error of the channel to string display
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is UBA_PROTO_UBA6.ERROR err) {
                if (err == UBA_PROTO_UBA6.ERROR.NoError) {
                    return string.Empty;
                } else { 
                    return $"{err}".ToUpperInvariant();
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
