namespace BK_PRECISION9104Libary {
    public partial class BK_PRECISION9104 {
        public class ReadingEventArgs : EventArgs {
            public int Voltage { get; }
            public int Current { get; }            
            public CV_CC_Mode mode { get; } 

            public ReadingEventArgs():base(){
            }
            public ReadingEventArgs(int vol) {
                Voltage = vol;
            }
            public ReadingEventArgs(int vol, int cur): this(vol) {                    
              Current = cur;
            }
            public ReadingEventArgs(int vol, int cur, CV_CC_Mode mode) :this(vol,cur) {
                this.mode = mode;
            }

        }


    }
}
