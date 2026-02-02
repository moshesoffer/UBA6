using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters {
    public class NumberToHexConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null) return "0";
            uint number = System.Convert.ToUInt32(value);
            return number.ToString("X");  // Hex uppercase
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null) return 0;

            string input = value.ToString();
            if (int.TryParse(input, System.Globalization.NumberStyles.HexNumber, culture, out int result))
                return result;

            return 0; // fallback if invalid hex
        }
    }
}
