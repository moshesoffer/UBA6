using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil {
    public interface IMeasurement {
        public  Task<float> Mesure<TEnum>(TEnum Type)  where TEnum : Enum ;
     
    }
}
