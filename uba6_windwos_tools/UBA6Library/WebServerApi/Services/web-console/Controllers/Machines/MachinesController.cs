using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers
{
    public class MachinesController : ControllerAPI{
        public WebRequest WebRq;
        public MachinesController(WebService ws): base(ws,"machines") {
            // Constructor logic if needed

            WebRq = new WebRequest(WebRequest.RequestType.GET| WebRequest.RequestType.POST| WebRequest.RequestType.PATCH| WebRequest.RequestType.DELETE, this,string.Empty, "Get list of machines");
        }
    }
}
