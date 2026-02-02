using System.Globalization;
using System.Windows.Data;
using UBA_PROTO_CHANNEL;

namespace UBA6_Controller_App.Converters {
    public class ChannelStateConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is UBA_PROTO_CHANNEL.STATE state) {
                return state.ToString().ToUpper();
            }
            return "Invalid";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
  
}
