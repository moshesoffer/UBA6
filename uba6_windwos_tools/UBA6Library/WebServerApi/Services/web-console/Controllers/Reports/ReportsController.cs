using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.Reports {
    public class ReportsController : ControllerAPI {
        public WebRequest Reports;

        public ReportsController(WebService ws) : base(ws, string.Empty) { 
            Reports = new WebRequest(WebRequest.RequestType.PATCH , this, "reports");
            Requests.Add(Reports);
        }
    }
}
