using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers {
    public class UBADevicesController : ControllerAPI {
        public WebRequest WebRq;
        public UBADevicesController(WebService ws) : base(ws, "uba-devices") {
            WebRq = new WebRequest(WebRequest.RequestType.GET | WebRequest.RequestType.POST | WebRequest.RequestType.PATCH | WebRequest.RequestType.DELETE, this, string.Empty, "get create updated and delete uba devices");
        }
    }
}
