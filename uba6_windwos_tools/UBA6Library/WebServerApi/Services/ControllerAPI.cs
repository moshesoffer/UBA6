using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services {
    public class ControllerAPI {
        public string Name { get; set; } = "ControllerAPI";

        public WebService? ParentService;

        public List<WebRequest> Requests = new List<WebRequest>();

        private int? version;


        protected virtual string? servicePath { get; }


        protected virtual Uri? reqestUri {
            get {
                return BuildUrl(servicePath).Uri;
            }
        }
        protected virtual string? baseUri { get; }
        protected virtual Uri? controllerUri { get; }

        public string? Path { get; set; } = string.Empty;

        public ControllerAPI() {

        }
        public ControllerAPI(WebService ws) : this() {
            this.ParentService = ws;            
        }
        public ControllerAPI(WebService ws, string path) : this(ws) {
            this.Path = path;
            this.ParentService = ws;
        }
        public ControllerAPI(WebService ws, string path, int version) : this(ws, path) {
            this.version = version;

        }
        public ControllerAPI(WebService ws, string path, string name) : this(ws, path) {
            this.Name = name;
        }

        public void Add(WebRequest newReqest) {
            Requests.Add(newReqest);
        }

        public string Path_API() {
            if (version == null) {
                return $"{Path}";
            } else {
                return $"/v{version}/{Path}";
            }
        }

        public UriBuilder BuildUrl(string? path) {
            UriBuilder builder = new UriBuilder();
            builder.Scheme = "Https";
            builder.UserName = "";
            builder.Password = "depdevo.mwdevice.com/";
            if (this.ParentService == null) {
                builder.Host = "";
            } else {
                builder.Host = this.ParentService.Host();
            }
            builder.Path = path;

            Debug.WriteLine("Build Url:\t" + builder.Uri.ToString());
            return builder;

        }
    }
}
