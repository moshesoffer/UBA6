using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil {
    public abstract class AmicellDevice<TDevice> : IMeasurement, IStatus where TDevice : AmicellDevice<TDevice> {
        protected readonly ILogger<TDevice> _logger;
        public bool IsInEmulationMode { get; set; } = false;

        public event EventHandler<StatusEventArg>? StatusChanged;
        public event EventHandler<ExceptionEventArg>? ExceptionOccurred;
        protected AmicellDevice(ILogger<TDevice> logger) {
            _logger = logger;
        }
        /*  
         */
        /// <summary>
        /// This method is used to measure a specific type of data.
        /// The type of data is defined by the TEnum parameter, which must be an Enum.
        /// The method returns a Task that resolves to a float value representing the measurement.
        /// Voltages return in mV ampres in mA, resistances in Ohm, and temperatures in Celsius.
        /// </summary>
        /// <typeparam name="TEnum"></typeparam>
        /// <param name="Type"></param>
        /// <returns></returns>
        public abstract Task<float> Mesure<TEnum>(TEnum Type) where TEnum : Enum;

        public async Task ApaandToCSV<TEnum>(TEnum Type, string data) where TEnum : Enum {
            string filePath = $"{typeof(TDevice).Name}_{DateTime.Now:yyyyMMdd}.csv";

            _logger.LogDebug("Appending data to CSV file: {FilePath}", filePath);
            if (string.IsNullOrWhiteSpace(filePath)) {
                throw RaiseException(new ArgumentException("File path cannot be null or empty.", nameof(filePath)));
            }

            try {
                // Check if the file exists, if not create it with a header
                if (!File.Exists(filePath)) {
                    await File.WriteAllTextAsync(filePath, "Timestamp,Type,Data" + Environment.NewLine);
                    _logger.LogInformation("File created with header: {FilePath}", filePath);
                }
                await File.AppendAllTextAsync(filePath, $"{DateTime.Now:yyyy-MM-dd HH:mm:ss},{Type},{data}" + Environment.NewLine);
                _logger.LogInformation("Data appended successfully to {FilePath}", filePath);
            } catch (Exception ex) {
                RaiseException(ex);
            }
        }

        public async Task<float> Mesure<TEnum>(TEnum Type, TimeSpan delay, uint mesureTimeNumber) where TEnum : Enum {
            float[] results = new float[mesureTimeNumber];
            RaiseNewStatusEvent($"Measuring {Type} {mesureTimeNumber} times with a delay of {delay.TotalSeconds} seconds.", 0);
            for (int i = 0; i < mesureTimeNumber; i++) {
                RaiseNewStatusEvent($"Measuring {Type} ({i + 1}/{mesureTimeNumber})", (100 * (i + 1) / (int)mesureTimeNumber));
                _logger.LogWarning("Delaing for {Delay} milliseconds before next measurement.", delay.TotalMilliseconds);
                await Task.Delay(delay);
                results[i] += await Mesure(Type);
                _logger.LogDebug("Measurement {Index} of {Type} completed: {Result}", i + 1, Type, results[i]);
                await ApaandToCSV(Type, results[i].ToString());

            }
            RaiseNewStatusEvent($"Measurement of {Type} completed. Average result: {results.Average()}", 100);
            _logger.LogDebug($"Average result : {results.Average()}");
            return results.Average();
        }



        public void RaiseNewStatusEvent(string statusStr) {
            StatusEventArg newEvent = new StatusEventArg(statusStr);
            _logger.LogInformation("Status changed: {Status}", newEvent);
            StatusChanged?.Invoke(this, new StatusEventArg(statusStr));
        }

        public void RaiseNewStatusEvent(string statusStr, int progress) {
            StatusEventArg newEvent = new StatusEventArg(statusStr, progress);
            _logger.LogInformation("Status changed: {Status}", newEvent);
            StatusChanged?.Invoke(this, newEvent);
        }

        public Exception RaiseException(Exception ex) {
            _logger.LogError(ex, "An exception occurred in: {Message}", ex.Message);
            ExceptionOccurred?.Invoke(this, new ExceptionEventArg(ex));
            return ex;
        }
    }
}
