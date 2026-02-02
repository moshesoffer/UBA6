using Grpc.Core;
using System.Diagnostics;
using System.Net;
using System.Text;
using System.Text.Json;
using UBA6Library.WebServerApi.Exceptions;
using UBA6Library.WebServerApi.Services;

namespace UBA6Library.WebServerApi {
    public class WebRequest {

        [Flags]
        public enum RequestType {
            NONE = 0x00,
            GET = 0x01,
            POST = 0x02,
            PUT = 0x04,
            DELETE = 0x08,
            PATCH = 0x10
        }
        public RequestType requestType { get; set; } = RequestType.NONE;

        protected virtual string? requestName { get; }

        protected virtual string? requestUrl { get; }

        protected Uri reqestUri { get { return Uri(); } }

        public string info { get; set; } = "WebRequest";

        private JsonSerializerOptions options = new JsonSerializerOptions {
            WriteIndented = true,
            DefaultIgnoreCondition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingNull
        };
        
        public ControllerAPI Controller { get; set; }

        public WebRequest(RequestType type) {
            requestType = type;
        }

        public WebRequest(RequestType type, ControllerAPI controller) : this(type) {
            Controller = controller;
        }

        public WebRequest(RequestType type, ControllerAPI controller, string requestName) : this(type, controller) {
            this.requestName = requestName;
        }

        public WebRequest(RequestType type, ControllerAPI controller, string requestName, string info) : this(type, controller, requestName) {
            this.info = info;
        }

        public Uri Uri(string surfix = "", string query = "") {
            UriBuilder builder = new UriBuilder();
            builder.Scheme = "Http";
           /* builder.UserName = "amicell";
            builder.Password = "1q!QazAZ";*/
            builder.Query = query;
            builder.Host = "localhost";
            builder.Port = 4000;
            builder.Path = "web-console";
            if (!string.IsNullOrEmpty(Controller.Path_API())) { 
                builder.Path = $"{builder.Path}/{Controller.Path_API()}";
            }
            if (!string.IsNullOrEmpty(requestName)) {
                builder.Path = $"{builder.Path}/{requestName}";
            } 
            if (!string.IsNullOrEmpty(surfix)) {
                builder.Path = $"{builder.Path}/{surfix}";
            }
            Debug.WriteLine("Build Url:\t" + builder.Uri.ToString());
            return builder.Uri;
        }

        protected void statusCodeHandle(HttpStatusCode code) {
            switch (code) {
                case HttpStatusCode.OK:
                    break;
                case HttpStatusCode.Created:
                    break;
                case HttpStatusCode.NoContent:
                    break;
                case HttpStatusCode.BadRequest:
                    throw new Exception("BadRequest");
                case HttpStatusCode.Unauthorized:
                    throw new ServerUnauthorizedException();
                case HttpStatusCode.NotFound:
                    throw new ServerNotFoundException();
                case HttpStatusCode.Conflict:
                    throw new ServerConflictException();//there is a value exiet in the server
                case HttpStatusCode.InternalServerError:
                    throw new Exception("InternalServerError");
                case HttpStatusCode.NotImplemented:
                    throw new ServerNotImplemented();
                default:
                    throw new Exception($"Failed to send request, return code :{code} -{(int)code}");
            }
        }
        protected void statusCodeHandle(HttpStatusCode statusCode, string contents, List<HttpStatusCode> allowedStatusCodes) {
            Debug.WriteLine($"Check Status Code - {((int)statusCode).ToString()}:{statusCode}");
            Debug.WriteLine($"Contents:{contents}");
            if (allowedStatusCodes.Contains(statusCode)) {
                Console.WriteLine($"Status code:{statusCode} is allowed.");
            } else {
                Console.WriteLine("Status code is NOT allowed.");
                statusCodeHandle(statusCode, contents);
            }
        }


        protected void statusCodeHandle(HttpStatusCode code, string contents) {            
            Debug.WriteLine($"Check Status Code - {((int)code).ToString()}:{code}");
            Debug.WriteLine($"Contents:{contents}");            
            switch (code) {
                case HttpStatusCode.OK:
                    // do noting
                    break;
                case HttpStatusCode.Created:
                    break;
                case HttpStatusCode.NoContent:
                    break;
                case HttpStatusCode.BadRequest:
                    throw new Exception("BadRequest");
                case HttpStatusCode.Unauthorized:
                    throw new ServerUnauthorizedException(contents);
                case HttpStatusCode.Forbidden:
                    throw new ServerUnauthorizedException(contents);
                case HttpStatusCode.NotFound:
                    throw new ServerNotFoundException(contents);
                case HttpStatusCode.Conflict:
                    throw new ServerConflictException(contents);//there is a value exiet in the server
                case HttpStatusCode.InternalServerError:
                    throw new Exception($"InternalServerError:{contents} ");
                case HttpStatusCode.NotImplemented:
                    throw new ServerNotImplemented();
                default:
                    throw new Exception($"Failed to send request, return code :{code} -{(int)code}");
            }
        }
        //============================================GET============================================
        protected async Task<TRes> Get<TRes>(HttpClient client, Uri uri) {
            AmicellUtil.Util.EnsureFlagIsSet(this.requestType, RequestType.GET);
            Console.WriteLine($"Get ======= URL:{uri.ToString()}");
            var response = await client.GetAsync(uri);
            var contents = await response.Content.ReadAsStringAsync();
            statusCodeHandle(response.StatusCode, contents);
            return JsonSerializer.Deserialize<TRes>(contents, this.options);
        }

        public async Task<TRes> Get<TRes>(HttpClient client, string surfix) {
            Uri uri = this.Uri(surfix);
            return await this.Get<TRes>(client, uri);
        }


        public async Task<TRes> Get<TRes>(HttpClient client) {
            return await this.Get<TRes>(client, this.Uri());
        }

        public async Task<TRes> Get<TRes>(HttpClient client, List<KeyValuePair<string, string>> querys) {
            string Query = string.Empty;
            for (int i = 0; i < querys.Count - 1; i++) {
                Query += $"{querys[i].Key}={querys[i].Value}&";
            }
            Query += $"{querys.Last().Key}={querys.Last().Value}";
            Uri t = this.Uri("", Query);
            return await Get<TRes>(client, t);
        }
        public async Task<TRes> Get<TRes>(HttpClient client, string surfix, List<KeyValuePair<string, string>> querys) {
            string Query = string.Empty;
            for (int i = 0; i < querys.Count - 1; i++) {
                Query += $"{querys[i].Key}={querys[i].Value}&";
            }
            Query += $"{querys.Last().Key}={querys.Last().Value}";
            Uri t = this.Uri(surfix, Query);
            return await Get<TRes>(client, t);
        }

        //============================================POST============================================
        protected async Task<TRes> Post<TRes, TArg>(HttpClient client, Uri uri, HttpContent httpContent) {
            AmicellUtil.Util.EnsureFlagIsSet(this.requestType, RequestType.POST);
            Console.WriteLine($"Post ======= URL:{uri.ToString()}");
            var response = await client.PostAsync(uri, httpContent);
            Debug.WriteLine($"the URI: {uri.ToString()} - httpContent: {httpContent}");
            var contents = await response.Content.ReadAsStringAsync();
            statusCodeHandle(response.StatusCode, contents);
            TRes? res = JsonSerializer.Deserialize<TRes>(contents, this.options);
            if (res == null) {
                throw new Exception("Failed to deserialize response");
            }
            return res;
        }

        protected async Task<TRes> Post<TRes, TArg>(HttpClient client, Uri uri, string json) {
            Console.WriteLine(json);
            Debug.WriteLine("POST JSON :" + json);
            StringContent httpContent = new StringContent(json, Encoding.UTF8, "application/json");
            return await Post<TRes, TArg>(client, uri, httpContent);
        }

        public async Task<TRes> Post<TRes, TArg>(HttpClient client, TArg obj)  {
            return await Post<TRes, TArg>(client, this.Uri(), JsonSerializer.Serialize<TArg>(obj, this.options));
        }
        public async Task<TRes> Post<TRes, TArg>(HttpClient client, TArg obj, List<KeyValuePair<string, string>> querys)  {
            string Query = string.Empty;
            for (int i = 0; i < querys.Count - 1; i++) {
                Query += $"{querys[i].Key}={querys[i].Value}&";
            }
            Query += $"{querys.Last().Key}={querys.Last().Value}";
            Uri t = this.Uri("", Query);
            return await Post<TRes, TArg>(client, t, JsonSerializer.Serialize<TArg>(obj));
        }

        //============================================PUT============================================
        protected async Task<TRes?> Put<TRes, TArg>(HttpClient client, Uri uri, HttpContent httpContent) {
            AmicellUtil.Util.EnsureFlagIsSet(this.requestType, RequestType.PUT);
            Console.WriteLine($"Put ======= URL:{uri.ToString()}");
            var response = await client.PutAsync(uri, httpContent);
            Debug.WriteLine($"the URI: {uri.ToString()} - httpContent: {httpContent}");
            if (typeof(TRes) == typeof(object) || response.Content.Headers.ContentLength == 0) {
                return default;
            }
            var contents = await response.Content.ReadAsStringAsync();
            statusCodeHandle(response.StatusCode, contents);
            TRes? res = JsonSerializer.Deserialize<TRes>(contents, this.options);
            if (res == null) {
                throw new Exception("Failed to deserialize response");
            }
            return res;
        }

        protected async Task<TRes?> Put<TRes, TArg>(HttpClient client, Uri uri, string json) {
            Console.WriteLine(json);
            Debug.WriteLine("Put JSON :" + json);
            StringContent httpContent = new StringContent(json, Encoding.UTF8, "application/json");
            return await Put<TRes?, TArg>(client, uri, httpContent);
        }


        public async Task<TRes?> Put<TRes, TArg>(HttpClient client, TArg obj) where TArg : UBA_JSON_DTO {
            return await Put<TRes?, TArg>(client, this.Uri(), JsonSerializer.Serialize<TArg>(obj, this.options));
        }


        public async Task<T?> Put<T>(HttpClient client, string json) {
            StringContent httpContent = new StringContent(json, Encoding.UTF8, "application/json");
            var response = await client.PutAsync(reqestUri, httpContent);
            var contents = await response.Content.ReadAsStringAsync();
            return JsonSerializer.Deserialize<T>(contents, this.options);
        }
        //============================================PATCH============================================
        protected async Task<TRes> Patch<TRes, TArg>(HttpClient client, Uri uri, HttpContent httpContent) {
            AmicellUtil.Util.EnsureFlagIsSet(this.requestType, RequestType.PATCH);
            Console.WriteLine($"Patch ======= URL:{uri.ToString()}");
            var response = await client.PatchAsync(uri, httpContent);
            var contents = await response.Content.ReadAsStringAsync();
            statusCodeHandle(response.StatusCode, contents);
            TRes? res = typeof(TRes) != typeof(object) ? JsonSerializer.Deserialize<TRes>(contents, this.options) : default;
            return res;
        }

        protected async Task<TRes> Patch<TRes, TArg>(HttpClient client, Uri uri, string json) {
            Console.WriteLine(json);
            StringContent httpContent = new StringContent(json, Encoding.UTF8, "application/json");
            return await Patch<TRes, TArg>(client, uri, httpContent);
        }

        public async Task<TRes> Patch<TRes, TArg>(HttpClient client, TArg obj)  {
            return await Patch<TRes, TArg>(client, this.Uri(), JsonSerializer.Serialize<TArg>(obj, this.options));
        }

        public async Task<TRes> Patch<TRes, TArg>(HttpClient client, TArg obj, List<KeyValuePair<string, string>> querys) {
            string Query = string.Empty;
            for (int i = 0; i < querys.Count - 1; i++) {
                Query += $"{querys[i].Key}={querys[i].Value}&";
            }
            Query += $"{querys.Last().Key}={querys.Last().Value}";
            Uri t = this.Uri("", Query);
            return await Patch<TRes, TArg>(client, t, JsonSerializer.Serialize<TArg>(obj));
        }

        public async Task<TRes> Patch<TRes, TArg>(HttpClient client, string surfix, TArg obj, List<KeyValuePair<string, string>> querys = null )  {
            string Query = string.Empty;
            if (querys == null) {
                //querys = new List<KeyValuePair<string, string>>();
            } else {
                for (int i = 0; i < querys.Count - 1; i++) {
                    Query += $"{querys[i].Key}={querys[i].Value}&";
                }
                Query += $"{querys.Last().Key}={querys.Last().Value}";
            }            
            Uri t = this.Uri(surfix, Query);
            string json = JsonSerializer.Serialize<TArg>(obj);
            Console.WriteLine("============================JSON object:=================================");
            Console.WriteLine(json);
            return await Patch<TRes, TArg>(client, t, json);
        }





        //===============================================DELETE==============================================

        protected async Task<TRes?> Delete <TRes>(HttpClient client, Uri uri) {
            AmicellUtil.Util.EnsureFlagIsSet(this.requestType, RequestType.DELETE);
            Console.WriteLine($"Delete ======= URL:{uri.ToString()}");
            var response = await client.DeleteAsync(uri);
            var contents = await response.Content.ReadAsStringAsync();
            statusCodeHandle(response.StatusCode, contents, new List<HttpStatusCode>() { HttpStatusCode.OK, HttpStatusCode.NoContent });
            TRes? res = typeof(TRes) != typeof(object) ? JsonSerializer.Deserialize<TRes>(contents, this.options) : default;
            return res;
        }


        public async Task<TRes?> Delete<TRes>(HttpClient client, string surfix) {
            Uri uri = this.Uri(surfix);
            return await this.Delete<TRes?>(client, uri);
        }


        public override string ToString() {
            return $"WebRequest: {info} URI: {this.Uri().ToString()}";
        }
    }
}
