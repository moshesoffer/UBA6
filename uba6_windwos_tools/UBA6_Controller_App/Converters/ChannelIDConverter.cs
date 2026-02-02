using System.Globalization;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters {
    class ChannelIDConverter : IValueConverter {
        // Convert the id of the channel to string display
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is UBA_PROTO_CHANNEL.ID id) {
                return $"CH {id}".ToUpperInvariant();
            }
            return "Invalid";
        }

        // Not used
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            throw new NotImplementedException();
        }
    }
}
