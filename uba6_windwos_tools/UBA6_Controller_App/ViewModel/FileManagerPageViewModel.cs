using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Google.Protobuf.Compiler;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UBA_PROTO_DATA_LOG;
using UBA6Library;

namespace UBA6_Controller_App.ViewModel {
    public partial class FileManagerPageViewModel : ObservableObject {

        private UBA6 model;
        [ObservableProperty]
        string filename = "Channel B_20250902091602_PC_Delay.pb";
        [ObservableProperty]
        ObservableCollection<UBA_PROTO_FM.CMD_ID> cMD_IDs = new ObservableCollection<UBA_PROTO_FM.CMD_ID>();

        [ObservableProperty]
        ObservableCollection<UBA_PROTO_DATA_LOG.data_log> fileData =  new ObservableCollection<data_log>();
        [ObservableProperty]
        ObservableCollection<string> files = new ObservableCollection<string>();

        public FileManagerPageViewModel(UBA6Library.UBA6 model) {
            this.model = model;
        }



        [RelayCommand]
        public async Task LoadFile() {
            var dialog = new OpenFileDialog {
                Filter = "Protocol Buffer Files (*.pb)|*.pb",
                Title = "Select a .pb file"
            };
            if (dialog.ShowDialog() == true) {                
                FileData.Clear();
                string filePath = dialog.FileName;
                byte[] file = await File.ReadAllBytesAsync(filePath);
                List<UBA_PROTO_DATA_LOG.data_log> logs = ProtoHelper.DecodeDataLogMessages(file);
                foreach (data_log log in logs) {
                    FileData.Add(log);
                }

            }
        }
        [RelayCommand]
        public async Task FetchFilesName() {
            Files.Clear();
            List<string> uba_file = await model.FeatchFileList();
            foreach (string f in uba_file) {
                Files.Add(f);
            }
        
        }

            [RelayCommand]
        public async Task FetchFile() {
            var dialog = new SaveFileDialog {
                Filter = "Protocol Buffer Files (*.pb)|*.pb",
                Title = "Save  .pb file",
                FileName = this.Filename
            };
            string saveas = this.Filename;
            if (dialog.ShowDialog() == true) { 
                saveas = dialog.FileName;
            }
                try {
                FileData.Clear();
                byte[] file = await model.FeatchFileToByteArray(Filename);
                File.WriteAllBytes(saveas, file);             
                List<UBA_PROTO_DATA_LOG.data_log> logs = ProtoHelper.DecodeDataLogMessages(file);
                foreach (data_log log in logs) {
                    FileData.Add(log);
                }                
            } catch (Exception ex) {
                Debug.WriteLine("Error fetching file: " + ex.Message);
            } finally {
                dialog.Reset();
            }
        }
    }
}
