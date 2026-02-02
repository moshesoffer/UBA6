using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using UBA_MSG;
using UBA_PROTO_CALIBRATION;
using UBA_PROTO_CMD;
using UBA_PROTO_QUERY;
using UBA_PROTO_TR;

namespace UBA6Library {
    public static class UBA_Message_Factory {
        static UInt32 Message_ID = 1;

        private static Header createHeadr(UInt32 address) {
            Header h = new Header();
            h.TargetAddress = address;
            h.Id = Message_ID++;
            return h;
        }


        public static Message CreateQeuryMessage(UInt32 address, UBA_PROTO_QUERY.RECIPIENT rECIPIENT) {
            Message msg = new Message();            
            msg.Head = createHeadr(address);
            msg.Query = new query_message();
            msg.Query.Recipient = rECIPIENT;
            return msg;
        }


        public static Message CreateMessage(UInt32 address, UBA_PROTO_CHANNEL.command ch_cmd) {
            Message msg = new Message();
            msg.Head = createHeadr(address);
            msg.Cmd = new command_message();
            msg.Cmd.Channel = ch_cmd;
            return msg;
        }

        public static Message CreateMessage(UInt32 address, UBA_PROTO_BPT.command bpt_cmd) {
            Message msg = new Message();
            msg.Head = createHeadr(address);
            msg.Cmd = new command_message();
            msg.Cmd.Bpt = bpt_cmd;
            return msg;
        }
        public static Message CreateMessage(UInt32 address, UBA_PROTO_UBA6.command uba_cmd) {
            Message msg = new Message();
            msg.Head = createHeadr(address);
            msg.Cmd = new command_message();
            msg.Cmd.Uba = uba_cmd;
            return msg;
        }

        public static Message CreateMessage(UInt32 address, UBA_PROTO_LINE.command line_cmd) {
            Message msg = new Message();
            msg.Head = createHeadr(address);
            msg.Cmd = new command_message();
            msg.Cmd.Line = line_cmd;
            return msg;
        }
        public static Message CreateMessage(UInt32 address, UBA_PROTO_FM.command fm_cmd) {
            Message msg = new Message();
            msg.Head = createHeadr(address);
            msg.Cmd = new command_message();
            msg.Cmd.File = fm_cmd;
            return msg;
        }
        public static Message CreateMessage(UInt32 address, Test_Routine_Message tr_message) {
            Message msg = new Message();            
            msg.Head = createHeadr(address);
            msg.Tr = tr_message;
            return msg;
        }

        public static Message CreateMessage(UInt32 address, line_calibration_message a, line_calibration_message b) {
            Message msg = new Message();
            UBA_PROTO_CALIBRATION.calibration_message pyload = new UBA_PROTO_CALIBRATION.calibration_message();
            msg.Head = createHeadr(address);            
            pyload.LineCalibration.Add(a);
            pyload.LineCalibration.Add(b);
            msg.Calibration = pyload;
            return msg;
        }

    }
}
