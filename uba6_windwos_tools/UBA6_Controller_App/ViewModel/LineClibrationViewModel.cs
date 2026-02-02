using Calibration;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows;
using System.Windows.Shapes;
using UBA6Library;
using static Calibration.Calibration;

namespace UBA6_Controller_App.ViewModel {
    public partial class LineClibrationViewModel : ObservableObject {
        private UBA6.LineCalibrationData model;
        private Calibration.Calibration cal;
        [ObservableProperty]
        string title = "Channel";

        [ObservableProperty]
        float vBatSlop10 = 1.0f;
        [ObservableProperty]
        float vbatY_Intercept10 = 0.0f;
        [ObservableProperty]
        float vBatSlop30 = 1.0f;
        [ObservableProperty]
        float vbatY_Intercept30 =0.0f;
        [ObservableProperty]
        float vBatSlop60 = 1.0f ;
        [ObservableProperty]
        float vbatY_Intercept60 = 0.0f;

        [ObservableProperty]
        float vPsSlop = 1.0f;
        [ObservableProperty]
        float vPsY_Intercept = 0.0f;
        [ObservableProperty]
        float vGenSlop = 1.0f;
        [ObservableProperty]
        float vGenY_Intercept = 0.0f;
        [ObservableProperty]
        float chargeCurrentSlop = 1.0f;
        [ObservableProperty]
        float chargeCurrentY_Intercept = 0.0f;
        [ObservableProperty]
        float dischargeCurrentSlop = 1.0f;
        [ObservableProperty]
        float dischargeCurrentY_Intercept = 0.0f;
        [ObservableProperty]
        float batteryTempSlop = 1.0f;
        [ObservableProperty]
        float batteryTempY_Intercept = 0.0f;
        [ObservableProperty]
        float ubaTempSlop = 1.0f;
        [ObservableProperty]
        float ubaTempY_Intercept = 0.0f;
        [ObservableProperty]
        UBA_PROTO_LINE.ID lineID;
        [ObservableProperty]
        bool isVbatCheck = false;
        [ObservableProperty]
        bool isVgenCheck = false;
        [ObservableProperty]
        bool isVPS_Check = false;
        [ObservableProperty]
        bool isCC_Check = false;
        [ObservableProperty]
        bool isDC_Check = false;
        [ObservableProperty]
        bool isAmbTempCheck = true;
        [ObservableProperty]
        bool isNTC_TempCheck = true;
        [ObservableProperty]
        static bool isClassEnable = true;
        [ObservableProperty]
        uint maxVoltage = 60000;
        [ObservableProperty]
        uint maxChargeCurrent = 3000;
        [ObservableProperty]
        uint maxDischargeCurrent = 3000;
        public LineClibrationViewModel(UBA6.LineCalibrationData m) {
            model = m;
            //LoadFromModel();
            this.PropertyChanged += OnPropertyChanged;
        }

        public LineClibrationViewModel(Calibration.Calibration calibration, UBA_PROTO_LINE.ID id) : this(id == UBA_PROTO_LINE.ID.A ? calibration.UBA.CalDataLineA : calibration.UBA.CalDataLineB) {
            this.cal = calibration;
            lineID = id;
            Title = $"Calibration Line {lineID}";
           
        }

        private void LoadFromModel() {
            // VBat
            VBatSlop10 = model.Vbat[0].Slop;
            VBatSlop30 = model.Vbat[1].Slop;
            VBatSlop60 = model.Vbat[2].Slop;

            VbatY_Intercept10 = model.Vbat[0].Y_Intercept;
            VbatY_Intercept30 = model.Vbat[1].Y_Intercept;
            VbatY_Intercept60 = model.Vbat[2].Y_Intercept;

            // VPs
            VPsSlop = model.Vps.Slop;
            VPsY_Intercept = model.Vps.Y_Intercept;

            // VGen
            VGenSlop = model.Vgen.Slop;
            VGenY_Intercept = model.Vgen.Y_Intercept;

            // Charge Current
            ChargeCurrentSlop = model.ChargeCurrent.Slop;
            ChargeCurrentY_Intercept = model.ChargeCurrent.Y_Intercept;

            // Discharge Current
            DischargeCurrentSlop = model.DischargeCurrent.Slop;
            DischargeCurrentY_Intercept = model.DischargeCurrent.Y_Intercept;

            // Battery Temp
            BatteryTempSlop = model.NtcTemp.Slop;
            BatteryTempY_Intercept = model.NtcTemp.Y_Intercept;

            // Uba Temp
            UbaTempSlop = model.AmbTemp.Slop;
            UbaTempY_Intercept = model.AmbTemp.Y_Intercept;
        }

        private void OnPropertyChanged(object? sender, PropertyChangedEventArgs e) {
            switch (e.PropertyName) {
                case nameof(VBatSlop10):
                    model.Vbat[0].Slop = VBatSlop10;
                    break;
                case nameof(VBatSlop30):
                    model.Vbat[1].Slop = VBatSlop30;
                    break;
                case nameof(VBatSlop60):
                    model.Vbat[2].Slop = VBatSlop60;
                    break;
                case nameof(VbatY_Intercept10):
                    model.Vbat[0].Y_Intercept = VbatY_Intercept10;
                    break;
                case nameof(VbatY_Intercept30):
                    model.Vbat[1].Y_Intercept = VbatY_Intercept30;
                    break;
                case nameof(VbatY_Intercept60):
                    model.Vbat[2].Y_Intercept = VbatY_Intercept60;
                    break;
                case nameof(VPsSlop):
                    model.Vps.Slop = VPsSlop;
                    break;
                case nameof(VPsY_Intercept):
                    model.Vps.Y_Intercept = VPsY_Intercept;
                    break;
                case nameof(VGenSlop):
                    model.Vgen.Slop = VGenSlop;
                    break;
                case nameof(VGenY_Intercept):
                    model.Vgen.Y_Intercept = VGenY_Intercept;
                    break;
                case nameof(ChargeCurrentSlop):
                    model.ChargeCurrent.Slop = ChargeCurrentSlop;
                    break;
                case nameof(ChargeCurrentY_Intercept):
                    model.ChargeCurrent.Y_Intercept = ChargeCurrentY_Intercept;
                    break;
                case nameof(DischargeCurrentSlop):
                    model.DischargeCurrent.Slop = DischargeCurrentSlop;
                    break;
                case nameof(DischargeCurrentY_Intercept):
                    model.DischargeCurrent.Y_Intercept = DischargeCurrentY_Intercept;
                    break;
                case nameof(BatteryTempSlop):
                    model.NtcTemp.Slop = BatteryTempSlop;
                    break;
                case nameof(BatteryTempY_Intercept):
                    model.NtcTemp.Y_Intercept = BatteryTempY_Intercept;
                    break;
                case nameof(UbaTempSlop):
                    model.AmbTemp.Slop = UbaTempSlop;
                    break;
                case nameof(UbaTempY_Intercept):
                    model.AmbTemp.Y_Intercept = UbaTempY_Intercept;
                    break;
                default:
                    break;
            }
        }


        [RelayCommand]
        public async Task BatteryVoltageCal() {
            try {
                IsClassEnable = false;
                model.Vbat = (await cal.BatteryCalibration(lineID)).ToArray();
                model.MaxVoltage = cal.MaxVoltage;
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }
        [RelayCommand]
        public async Task InputVoltageCal() {
            try {
                IsClassEnable = false;
                model.Vps = (await cal.VPS_Calibration(lineID));
                model.MaxVoltage = cal.MaxVoltage;
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }
        [RelayCommand]
        public async Task GenVoltageCal() {
            try {
                IsClassEnable = false;
                model.Vgen = await cal.GenVoltage_Calibration(lineID);
                model.MaxVoltage = cal.MaxVoltage;
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }
        [RelayCommand]
        public async Task ChargeCurrentCal() {
            try {
                IsClassEnable = false;
                model.ChargeCurrent = await cal.ChargeCuurentCalibration(lineID);
                model.MaxChargeCurrent = cal.MaxChargeCurrent+500;
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }
        [RelayCommand]
        public async Task DischargeCurrentCal() {
            try {
                IsClassEnable = false;
                model.DischargeCurrent = await cal.DischargeCuurentCalibration(lineID);
                model.MaxDischargeCurrent = cal.MaxDischargeCurrent +500;
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }
        [RelayCommand]
        public async Task BatteryTempCal() {
            try {
                IsClassEnable = false;
                model.NtcTemp = await cal.BatteryTempCalibration(lineID);
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }
        [RelayCommand]
        public async Task AmbientTempCal() {
            try {
                IsClassEnable = false;
                model.AmbTemp = await cal.AmbiantTempCalibration(lineID);
                LoadFromModel();
            } catch (Exception ex) {
                MessageBox.Show(ex.Message);
            } finally {
                IsClassEnable = true;
            }
        }


        [RelayCommand]
        public void CreateCalibrationFile() {
           

        }

        public Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE GetSelectedVoltages() {

            Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE ret = Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.NONE;

            if (IsVbatCheck) {
                ret |= Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V | Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V | Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE60V;
            }
            if (IsVgenCheck) {
                ret |= Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.GEN_VOLTAGE;
            }
            if (IsVPS_Check) {
                ret |= Calibration.Calibration.UBA_CALIBRATION_VOLTAGE_TYPE.VPS;
            }
            return ret;
        }

        public Calibration.Calibration.UBA_CALIBRATION_TEMP_TYPE GetSelectedTemp() {
            Calibration.Calibration.UBA_CALIBRATION_TEMP_TYPE ret = Calibration.Calibration.UBA_CALIBRATION_TEMP_TYPE.NONE;
            if (IsAmbTempCheck) {
                ret |= Calibration.Calibration.UBA_CALIBRATION_TEMP_TYPE.AMBIANT_TEMP;
            }
            if (IsNTC_TempCheck) {
                ret |= Calibration.Calibration.UBA_CALIBRATION_TEMP_TYPE.BATTERY_TEMP;
            }
            return ret;
        }

        public void SetUI_FromTest(List<VoltageCalibration> list ) {
            foreach (VoltageCalibration vc in list) {
                if (vc.LineID == LineID) {
                    switch (vc.TYPE) {
                        case UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE10V:
                            model.Vbat[0] =  vc.Equation; 
                            break;
                        case UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE30V:
                            model.Vbat[1] = vc.Equation;
                            break;
                        case UBA_CALIBRATION_VOLTAGE_TYPE.BATTERY_RANGE60V:
                            model.Vbat[2] = vc.Equation;
                            break;
                        case UBA_CALIBRATION_VOLTAGE_TYPE.VPS:
                            model.Vps = vc.Equation;
                            break;
                        case UBA_CALIBRATION_VOLTAGE_TYPE.GEN_VOLTAGE:
                            model.Vgen = vc.Equation;
                            break;
                        default:
                            throw new ArgumentOutOfRangeException(nameof(vc.TYPE), $"Unknown voltage type: {vc.TYPE} for line {LineID}.");

                    }                   
                }
            }
            LoadFromModel();
        }

        public void SetUI_FromTest(List<TempCalibration> list) {
            foreach (TempCalibration vc in list) {
                if (vc.LineID == LineID) {
                    switch (vc.Type) {
                        case UBA_CALIBRATION_TEMP_TYPE.BATTERY_TEMP:
                            model.NtcTemp = vc.Equation;
                            break;
                        case UBA_CALIBRATION_TEMP_TYPE.AMBIANT_TEMP:                        
                            model.AmbTemp = vc.Equation;
                            break;                        
                        default:
                            throw new ArgumentOutOfRangeException(nameof(vc.Type), $"Unknown Temp type: {vc.Type} for line {LineID}.");

                    }
                }
            }
            LoadFromModel();
        }

        public static void SetBusy(bool isBusy) {
            isClassEnable = !isBusy;
        }
    }
}

