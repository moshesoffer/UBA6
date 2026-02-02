using Microsoft.AspNetCore.Mvc;
using System.Diagnostics.Contracts;
using System.Linq;

namespace Server.Controllers {
    [ApiController]
    [Route("[controller]")]
    public class UserController : ControllerBase {
    
        private readonly ILogger<UserController> _logger;

        private static readonly List<User> Users = new List<User>
        {
            new User { Email = "User1@amicell.com", Name = "User1", Password = "1q!QazAZ" },
            new User { Email = "User2@amicell.com", Name = "User2", Password = "1q!QazAZ" }
        };

        public UserController(ILogger<UserController> logger) {
            _logger = logger;
        }       

        [HttpGet(Name = "GetUsers")]
        public IEnumerable<User> Get() {
            _logger.LogInformation("Call Get Users");
            return Users.ToArray();            
        }

        [HttpGet("{Email}")]
        public ActionResult<User> Get(string Email) {
            var item = Users.FirstOrDefault(i => i.Email == Email);
            if (item == null) {
                return NotFound();
            }
            return item;
        }


        [HttpPost]
        public ActionResult<User> Post(User newUser) {            
            Users.Add(newUser);
            return CreatedAtAction(nameof(Get), new { Email = newUser.Email }, newUser);
        }
        
        [HttpPut("{Email}")]
        public ActionResult Put(string Email,User updatedUser) {
            var user = Users.FirstOrDefault(i => i.Email.Equals(Email, StringComparison.OrdinalIgnoreCase));
            if (user == null) { 
                return NotFound();
            }
            user.Update(updatedUser);
            return NoContent();
        }

        [HttpDelete("{Email}")]
        public ActionResult Delete(string Email) { 
            var user = Users.FirstOrDefault(i => i.Email == Email);
            if (user == null) {
                return NotFound();
            }
            Users.Remove(user);
            return NoContent();
        }

        
        [HttpPost("logout")]
        public ActionResult Post() {
            return NoContent();
        }

        [HttpPost("login")]
        public ActionResult<User> Login(UserloginDTO loginUser) {
            var user = Users.FirstOrDefault(i => i.UserName.Equals(loginUser.UserName, StringComparison.OrdinalIgnoreCase));
            if (user == null) {             
                return NotFound();
            } 
            if (user.Password.Equals(loginUser.Password)) {
                return user;
            } else {
                return NotFound();
            }            
        }
    }
}
