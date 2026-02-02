namespace Server.TestRoutines {
    public class TestRoutinesDTO {
        public string id { get; set; }
        public string testName { get; set; }
        public string batteryPN { get; set; }
        public string batterySN { get; set; }
        public string cellPN { get; set; }
        public string cellsInSerial { get; set; }
        public string cellsInParallel { get; set; }
        public int maxPerBattery { get; set; }
        public int ratedBatteryCapacity { get; set; }
        public string ubaChannel { get; set; }
        public string notes { get; set; }
        public string customer { get; set; }
        public string workOrder { get; set; }
        public string approvedBy { get; set; }
        public string conductedBy { get; set; }
        public string cellSupplier { get; set; }
        public string cellDate { get; set; }
     
    }
}
