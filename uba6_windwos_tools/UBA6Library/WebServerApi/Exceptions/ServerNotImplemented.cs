namespace UBA6Library.WebServerApi.Exceptions
{
    [Serializable]
    public class ServerNotImplemented : Exception
    {
        public ServerNotImplemented()
        {

        }
        public ServerNotImplemented(string msg) : base(msg)
        {

        }

        public ServerNotImplemented(string msg, Exception inner) : base(msg, inner)
        {

        }
    }

}
