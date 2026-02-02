using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers {
    public class AuthController : ControllerAPI {

        public WebRequest Login { get; set; } 
        public WebRequest Logout { get; set; } 

        public AuthController(WebService s) :base(s) {
            Login= new WebRequest(WebRequest.RequestType.POST, this, "login", "Login to the web console");
            Logout = new WebRequest(WebRequest.RequestType.POST, this, "logout", "Logout from the web console");

        }
    }
}

