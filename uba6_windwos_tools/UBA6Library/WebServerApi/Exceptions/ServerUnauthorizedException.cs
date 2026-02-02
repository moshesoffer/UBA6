namespace UBA6Library.WebServerApi.Exceptions
{
    [Serializable]
    public class ServerUnauthorizedException : Exception
    {
        public ServerUnauthorizedException()
        {

        }
        public ServerUnauthorizedException(string msg) : base(msg)
        {

        }

        public ServerUnauthorizedException(string msg, Exception inner) : base(msg, inner)
        {

        }
    }

}
