using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests {
    public class RunningTestsController : ControllerAPI {
        
        public WebRequest PendingTest;
        public WebRequest ChangeRunningTestStatus;
        public WebRequest InstantTestResults;

        [Flags]
        public enum Status : UInt32 {
            STANDBY = 0x0001,
            STOPPED = 0x0002,
            ABORTED = 0x0004,
            FINISHED = 0x0008,
            SAVED = 0x0010,
            RUNNING = 0x0020,
            PAUSED = 0x0040,
            PENDING = 0x0100,
            IS_TEST_RUNNING = PENDING | RUNNING | PAUSED,

        }


        public RunningTestsController(WebService ws) : base(ws, string.Empty) {
            
            PendingTest = new WebRequest(WebRequest.RequestType.GET, this, "pending-tests");
            ChangeRunningTestStatus = new WebRequest(WebRequest.RequestType.PATCH, this, "change-running-test-status");
            InstantTestResults = new WebRequest(WebRequest.RequestType.POST|WebRequest.RequestType.GET, this, "instant-test-results");        
            Requests.Add(PendingTest);
            Requests.Add(ChangeRunningTestStatus);
            Requests.Add(InstantTestResults);
        }

    }
}
