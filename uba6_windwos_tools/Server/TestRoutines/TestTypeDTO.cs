namespace Server.TestRoutines {
    public class TestTypeDTO {
        public int id { get; set; }
        public string type { get; set; }
        public bool isCollapsed { get; set; }
        public string source { get; set; }
        public bool isMinTemp { get; set; }
        public object minTemp { get; set; }
        public bool isMaxTemp { get; set; }
        public object maxTemp { get; set; }
        public bool isMaxTime { get; set; }
        public object maxTime { get; set; }
        public string delayTime { get; set; }
        public bool isChargeLimit { get; set; }
        public string chargeLimit { get; set; }
        public bool isDischargeLimit { get; set; }
        public string dischargeLimit { get; set; }
        public string chargeCurrent { get; set; }
        public string dischargeCurrent { get; set; }
        public bool isCutOffCurrent { get; set; }
        public string cutOffCurrent { get; set; }
        public bool isCutOffVoltage { get; set; }
        public object cutOffVoltage { get; set; }
        public object chargePerCell { get; set; }
        public string waitTemp { get; set; }
        public object goToStep { get; set; }
        public object repeatStep { get; set; }
    }
}
