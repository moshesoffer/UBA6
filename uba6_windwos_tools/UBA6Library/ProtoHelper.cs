using Grpc.Core;
using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UBA_PROTO_DATA_LOG;
using UBA_PROTO_TR;

namespace UBA6Library {
    public static class ProtoHelper {

        public static readonly uint DEFAULT_TR_LOG_INTRVAL_MS = 500;
        public static readonly int DEFAULT_CHARGE_CUTOFF_CURRENT = Int32.MinValue;
        public static readonly int DEFAULT_DISCHARGE_CUTOFF_VOLTAGE = Int32.MinValue;
        public static readonly UInt32 DEFAULT_MAX_TEMP = UInt32.MaxValue;
        public static readonly uint DEFAULT_MAX_TIME = uint.MaxValue;
        public static readonly int DEFAULT_LIMIT_CAPACITY = int.MaxValue;


        public static Test_Routine_Message CreateTR_Message(uint StoreIndex, Test_Routine tr) {
            Test_Routine_Message test_Routine_Message = new Test_Routine_Message();
            test_Routine_Message.Index = StoreIndex;
            test_Routine_Message.Tr = tr;
            return test_Routine_Message;
        }

        public static UBA_PROTO_BPT.charge_stop_condition CreateChargeStopCondtion(float maxTemp = float.MaxValue,
            UInt32 maxTime = UInt32.MaxValue,
            Int32 cutOffCurrent = Int32.MinValue,
            Int32 limitCapacity = Int32.MaxValue
            ) {

            UBA_PROTO_BPT.charge_stop_condition t = new UBA_PROTO_BPT.charge_stop_condition() {
                MaxTemperature = maxTemp,
                MaxTime = maxTime,
                CutOffCurrent = cutOffCurrent,
                LimitCapacity = limitCapacity,
            };
            Console.WriteLine($"Create Charge Stop Condition: {t.MaxTemperature} {t.MaxTime} {t.CutOffCurrent} {t.LimitCapacity}");
            return t;
        }

        public static UBA_PROTO_TR.Test_Routine CreateTestRoutine(UBA_PROTO_BPT.MODE mode, List<object> steps, string name = "Helper BPT" , UInt16 logIntrval = 500) {
            if (steps.Count == 0) {
                throw new ValidationException("Steps list cannot be empty");
            }
            UBA_PROTO_TR.Test_Routine tr = new UBA_PROTO_TR.Test_Routine() {
                Mode = mode,
                Length = (uint)steps.Count,
                Name = name,
                LogInterval = logIntrval,
            };
            config_step cs;            
                for (int i = 0; i < 10; i++) {
                    cs = new UBA_PROTO_TR.config_step();                    
                    if (i < steps.Count) {
                        if (steps[i] is UBA_PROTO_BPT.charge) {
                            cs.Charge = steps[i] as UBA_PROTO_BPT.charge;
                            cs.TypeId = UBA_PROTO_TR.STEP_TYPE.Charge;                            
                        } else if (steps[i] is UBA_PROTO_BPT.discharge) {
                            cs.Discharge = steps[i] as UBA_PROTO_BPT.discharge;
                            cs.TypeId = UBA_PROTO_TR.STEP_TYPE.Discharge;
                        } else if (steps[i] is UBA_PROTO_BPT.delay) {
                            cs.Delay = steps[i] as UBA_PROTO_BPT.delay;
                            cs.TypeId = UBA_PROTO_TR.STEP_TYPE.Delay;
                        } else if (steps[i] is UBA_PROTO_TR.Loop) {
                            cs.Loop = steps[i] as UBA_PROTO_TR.Loop;
                            cs.TypeId = UBA_PROTO_TR.STEP_TYPE.Loop ;
                        } else {
                            throw new ValidationException("Step must be of type UBA_PROTO_BPT.charge or UBA_PROTO_BPT.discharge");
                        }
                    }
                    tr.Config.Add(cs);
                }            
            return tr;
        }


        public static UBA_PROTO_BPT.charge CreateChargeStep(UBA_PROTO_BPT.SOURCE source, int current, int voltage, UBA_PROTO_BPT.charge_stop_condition sc, float minTemp = -273.0f) {
            if (source != UBA_PROTO_BPT.SOURCE.Internal) {
                throw new NotImplementedException("Only Internal Source is supported for now");
            }
            if (minTemp < -273.0f) {
                throw new ValidationException("Min Temp must be greater than -273.0f");
            }
            UBA_PROTO_BPT.charge cs = new UBA_PROTO_BPT.charge() { Source = source, Current = current, Voltage = voltage, MinTemperature = minTemp, Sc = sc };
            Console.WriteLine($"Create Charge Step: {cs.Source} {cs.Current} mA {cs.Voltage} mV {cs.MinTemperature} C");
            return cs;
        }

        public static UBA_PROTO_BPT.discharge_stop_condition CreateDischargeStopCondition(float maxTemp = float.MaxValue,
            UInt32 maxTime = UInt32.MaxValue,
            Int32 cutOfVoltage = Int32.MinValue,
            Int32 limitCapacity = Int32.MaxValue
            ) {
            UBA_PROTO_BPT.discharge_stop_condition t = new UBA_PROTO_BPT.discharge_stop_condition() {
                MaxTemperature = maxTemp,
                MaxTime = maxTime,
                CutOffVoltag = cutOfVoltage,
                LimitCapacity = limitCapacity,
            };
            Console.WriteLine($"Create Discharge Stop Condition: Max Temp:{t.MaxTemperature} MaxTime:{t.MaxTime} CutOffVoltag: {t.CutOffVoltag} LimitCapacity:{t.LimitCapacity}");
            return t;
        }

        public static UBA_PROTO_BPT.discharge_current CreateDischargeCurrent(int value, UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE type) {
            if (value < 0) {
                throw new ValidationException("Discharge current must be greater than or equal to 0");
            }
            UBA_PROTO_BPT.discharge_current dc = new UBA_PROTO_BPT.discharge_current() { Value = value, Type = type };
            Console.WriteLine($"Create Discharge Current: {dc.Value} {dc.Type}");
            return dc;
        }

        public static UBA_PROTO_BPT.discharge CreateDischargeStep(UBA_PROTO_BPT.SOURCE source, int current, UBA_PROTO_BPT.DISCHARGE_CURRENT_TYPE type, UBA_PROTO_BPT.discharge_stop_condition sc, float minTemp = -273.0f) {
            if (source != UBA_PROTO_BPT.SOURCE.Internal) {
                throw new NotImplementedException("Only Internal Source is supported for now");
            }
            if (minTemp < -273.0f) {
                throw new ValidationException("Min Temp must be greater than -273.0f");
            }
            UBA_PROTO_BPT.discharge cs = new UBA_PROTO_BPT.discharge() {
                Source = source,
                Current = new UBA_PROTO_BPT.discharge_current() { Value = current, Type = type },
                MinTemperature = minTemp,
                Sc = sc
            };
            Console.WriteLine($"Create Discharge Step: {cs.Source} {cs.Current.Value} [{cs.Current.Type}] {cs.MinTemperature} C");
            return cs;
        }

        public static UBA_PROTO_BPT.delay CreateDelayStep(UInt32 delayTime = UInt32.MaxValue ,float cooldownTemp  = -273.15f) {
            if (delayTime == 0) {
                throw new ValidationException("Delay time must be greater than 0");
            }
            UBA_PROTO_BPT.delay ds = new UBA_PROTO_BPT.delay() { 
                CoolDownTemperature = cooldownTemp,
                DelayTime = delayTime,
                

            };
            
            return ds;
        }


        public static UBA_PROTO_LINE.command CreateLineCommand(UBA_PROTO_LINE.CMD_ID cmdID, UBA_PROTO_LINE.ID lineID = UBA_PROTO_LINE.ID.None,
            byte state = (byte)UBA_PROTO_LINE.STATE.Init, Int32 voltage = 0, Int32 current = 0) {
            UBA_PROTO_LINE.command cmd = new UBA_PROTO_LINE.command();
            cmd.Id = cmdID;
            cmd.LineId = lineID;
            cmd.State = state;
            cmd.Voltage = voltage;
            cmd.Current = current;
            return cmd;
        }

        public static UBA_PROTO_CHANNEL.command CreateChannelCommand(UBA_PROTO_CHANNEL.CMD_ID cmdID, UBA_PROTO_CHANNEL.ID id = UBA_PROTO_CHANNEL.ID.None) {
            UBA_PROTO_CHANNEL.command cmd = new UBA_PROTO_CHANNEL.command();
            cmd.Id = cmdID;
            cmd.Channel = id;
            return cmd;
        }

        public static UBA_PROTO_UBA6.command CreateDeviceCommand(UBA_PROTO_UBA6.CMD_ID cmdID, UInt32 value = 0, string name = "") {
            UBA_PROTO_UBA6.command cmd = new UBA_PROTO_UBA6.command();
            cmd.Id = cmdID;
            cmd.Value = value;
            cmd.Name = name;
            return cmd;
        }
        public static UBA_PROTO_BPT.command CreateBPTCommand(UBA_PROTO_BPT.CMD_ID cmdId, UBA_PROTO_CHANNEL.ID ch, uint listIndex = 0) {
            UBA_PROTO_BPT.command cmd = new UBA_PROTO_BPT.command();
            cmd.Id = cmdId;
            cmd.Channel = ch;
            cmd.BPTListEntery = listIndex;
            return cmd;
        }

        public static UBA_PROTO_FM.command CreateFileCommand(UBA_PROTO_FM.CMD_ID cmdId, string filename, UInt32 chunkIndex = 0) {
            UBA_PROTO_FM.command cmd = new UBA_PROTO_FM.command();
            cmd.Id = cmdId;
            cmd.Filename = filename;
            cmd.ChunkIndex = chunkIndex;
            return cmd;
        }

        public static ulong DecodeVarint(Stream stream) {
            ulong result = 0;
            int shift = 0;
            while (true) {
                int b = stream.ReadByte();
                if (b == -1) break;
                result |= ((ulong)(b & 0x7F)) << shift;
                if ((b & 0x80) == 0) break;
                shift += 7;
            }
            return result;
        }


        public static List<UBA_PROTO_DATA_LOG.data_log> DecodeDataLogMessages(byte[] data) {
            List<UBA_PROTO_DATA_LOG.data_log> logs = new List<UBA_PROTO_DATA_LOG.data_log>();
            using var ms = new MemoryStream(data);
            while (ms.Position < ms.Length) {
                // Decode varint (message length)
                ulong length = DecodeVarint(ms);
                if (length == 0 || ms.Position + (long)length > ms.Length)
                    break;

                // Read message bytes
                byte[] msgBytes = new byte[length];
                ms.Read(msgBytes, 0, (int)length);

                // Parse protobuf message
                data_log dataLog = data_log.Parser.ParseFrom(msgBytes);

                // Map to struct

                logs.Add(dataLog);
            }
            return logs;
        }
    }
}
