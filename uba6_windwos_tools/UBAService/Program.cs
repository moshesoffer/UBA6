using AmicellUtil;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging.Console;
using UBAService;



IHost host = Host.CreateDefaultBuilder(args)
    .ConfigureAppConfiguration((hostingContext, config) => {
        // Load configuration from appsettings.json and environment variables
        config.AddJsonFile("appsettings.json", optional: true, reloadOnChange: true)
              .AddEnvironmentVariables();
    }).ConfigureLogging((context, logging) => {
        logging.ClearProviders();                    // Optional: clear default providers
        logging.AddConsole();                        // You can add file, event log, etc.
        logging.AddConfiguration(context.Configuration.GetSection("Logging"));
        logging.AddSimpleConsole(options => {
            options.IncludeScopes = false;
            options.SingleLine = true;
            options.TimestampFormat = "HH:mm:ss ";
            options.ColorBehavior = LoggerColorBehavior.Enabled;
        });
    })

    .UseWindowsService() // This makes it run as a Windows Service
    .ConfigureServices((hostContext, services) => {
        services.AddHostedService<Worker>();
        services.Configure<MyLocalSettings>(
            hostContext.Configuration.GetSection("MyLocalSettings"));
        /*services.AddLogging(builder =>
        {
            builder.ClearProviders();
            builder.AddProvider(new ColorConsoleLoggerProvider());
        });*/
    })
    .Build();

host.Run();
