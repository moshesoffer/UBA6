using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services {
    public class WebService {
        protected const string DEV_ENV = "localhost";
        protected const string DEV_STAGING = "localhost";
        protected const string PROD_ENV = "192.168.12.245"; // TODO:change to production server address

        protected const string HOST  = ":4000";

        protected string port { get; set; } = "4000"; // default port for the service
        protected string host { get; set; } = "localhost"; // default host for the service
        

        protected virtual string serviceName { set; get; } = string.Empty;
        protected virtual int? version { get; }
        protected string? servicePath {
            get {          
                if (version != null) {
                    return $"{host}.{port}/{serviceName}/v{version}";
                } else {
                    return $"{host}.{port}/{serviceName}";
                }
            }
        }       

        public string? ServiceName { get { return serviceName; } }

        public List<ControllerAPI> Controllers = new List<ControllerAPI>();
        public WebService(string host,string port,string serviceName) {
            this.host = host;
            this.port = port;
            this.serviceName = serviceName;
        }
        public WebService(string host, string port, string serviceName, int version) :this(host,port,serviceName) {
            this.version = version;
        }


        public static string GetLocalIPv4() {
            foreach (NetworkInterface ni in NetworkInterface.GetAllNetworkInterfaces()) {
                if (ni.OperationalStatus == OperationalStatus.Up &&
                    ni.NetworkInterfaceType != NetworkInterfaceType.Loopback &&
                    !ni.Description.ToLower().Contains("virtual") &&
                    !ni.Description.ToLower().Contains("pseudo")) {
                    foreach (UnicastIPAddressInformation ip in ni.GetIPProperties().UnicastAddresses) {
                        if (ip.Address.AddressFamily == AddressFamily.InterNetwork) {
                            return ip.Address.ToString();
                        }
                    }
                }
            }

            return "IP Address Not Found";
        }

        public static string GetMacAddress() {
            var mac = NetworkInterface
                .GetAllNetworkInterfaces()
                .Where(nic =>
                    nic.OperationalStatus == OperationalStatus.Up &&
                    nic.NetworkInterfaceType != NetworkInterfaceType.Loopback &&
                    nic.GetPhysicalAddress().GetAddressBytes().Length == 6)
                .Select(nic => nic.GetPhysicalAddress().ToString())
                .FirstOrDefault();

            return mac ?? "No MAC address found.";
        }

        public string Host() {
            return servicePath;
        }
    }
}
