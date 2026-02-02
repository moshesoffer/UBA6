using System.ComponentModel.DataAnnotations;
using System.Text.Json.Serialization;

namespace UBA6Library.WebServerApi.Services.WebConsole.Controllers.RunningTests.Models {
    public class PATCH_ChangeTR_StatusRequest {
        [Required]
        [JsonPropertyName("runningTestID")]
        public Guid RunningTestID { get; set; }

        [Required]
        [JsonPropertyName("testRoutineChannels")]
        public string TestRoutineChannels { get; set; }

        [Required]
        [JsonPropertyName("ubaSN")]
        public string UbaSN { get; set; }

        [Required]
        [JsonPropertyName("newTestStatus")]
        public int NewTestStatus { get; set; }
    }
}

