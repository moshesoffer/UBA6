using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil
{
    public class OutOfBoundException : Exception
    {
        public OutOfBoundException() { 
        }
        public OutOfBoundException(string message) : base(message) { }
        public OutOfBoundException(int value,int buttom, int top) : base($"Value:{value} is OOB {buttom}-{top}") { 
        }
        
        public OutOfBoundException(string message, Exception inner) : base(message, inner) { }
        public OutOfBoundException(int value, int buttom, int top, Exception inner) : base($"Value:{value} is OOB {buttom}-{top}", inner) {
        }
    }
}
