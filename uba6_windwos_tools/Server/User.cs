using System.Data;
using System.Text.Json.Serialization;

namespace Server {

    public class UserloginDTO {
        [JsonPropertyName("username")]
        public string UserName { get; set; }  = string.Empty;
        public string Password { get; set; } = string.Empty;
    }
    public class User : UserloginDTO {

        public string Email { get; set; } = string.Empty;
        public string? Name { get; set; }        

        public void Update(User OtherUser) {
            UserName = OtherUser.UserName;  
            Name = OtherUser.Name;
            Email = OtherUser.Email;
            Password = OtherUser.Password;
        }
    }
}
