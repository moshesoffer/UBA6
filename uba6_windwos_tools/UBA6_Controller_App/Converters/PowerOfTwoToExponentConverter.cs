using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace UBA6_Controller_App.Converters {
    public class PowerOfTwoToExponentConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null) return 0;

            int number = System.Convert.ToInt32(value);

            if (number <= 0) return 0;

            int exponent = 0;
            while (number > 1) {
                number >>= 1;   // divide by 2
                exponent++;
            }

            return exponent == 0 ? 1 : exponent; // special case: show 1 for value=1
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value == null) return 0;

            int exponent = System.Convert.ToInt32(value);
            return 0x01 << exponent; // turn back into 2^n
        }
    }
}
