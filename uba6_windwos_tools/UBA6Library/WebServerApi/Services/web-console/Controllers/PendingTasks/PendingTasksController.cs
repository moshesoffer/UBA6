using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.web_console.Controllers.PendingTasks {
    public class PendingTasksController : ControllerAPI {
        public WebRequest PendingTasks;

        public PendingTasksController(WebService ws) : base(ws, string.Empty) {
            PendingTasks = new WebRequest(WebRequest.RequestType.GET| WebRequest.RequestType.POST, this, "pending-tasks");
            Requests.Add(PendingTasks);
        }
    }
}
