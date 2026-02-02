using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AmicellUtil {
    public class ColorConsoleLogger : ILogger {
        private readonly string _categoryName;

        public ColorConsoleLogger(string categoryName) {
            _categoryName = categoryName;
            
        }

        public IDisposable BeginScope<TState>(TState state) => null!;
        public bool IsEnabled(LogLevel logLevel) => true;

        public void Log<TState>(LogLevel logLevel, EventId eventId,
            TState state, Exception exception, Func<TState, Exception?, string> formatter) {
            var originalColor = Console.ForegroundColor;

            Console.ForegroundColor = logLevel switch {
                LogLevel.Information => ConsoleColor.Green,
                LogLevel.Warning => ConsoleColor.Yellow,
                LogLevel.Error => ConsoleColor.Red,
                LogLevel.Critical => ConsoleColor.Magenta,
                LogLevel.Debug => ConsoleColor.Cyan,
                _ => ConsoleColor.Gray
            };

            string logLine = $"{DateTime.Now:HH:mm:ss} [{logLevel}] {_categoryName}: {formatter(state, exception)}";

            Console.WriteLine(logLine);

            Console.ForegroundColor = originalColor;
        }
    }
    public class ColorConsoleLoggerProvider : ILoggerProvider {
        public ILogger CreateLogger(string categoryName)
            => new ColorConsoleLogger(categoryName);

        public void Dispose() { }
    }


}
