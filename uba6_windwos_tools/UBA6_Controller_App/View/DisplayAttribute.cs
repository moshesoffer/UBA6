using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6_Controller_App.View {
    [AttributeUsage(AttributeTargets.Property)]
    public class DisplayAttribute : Attribute {
        public string Type { get; set; }

        public string Description { get; set; }
        public DisplayAttribute(string type){ 
            Type = type;
        }
    }
}
