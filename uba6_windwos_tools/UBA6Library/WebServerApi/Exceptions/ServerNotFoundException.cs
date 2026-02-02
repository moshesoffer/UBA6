namespace UBA6Library.WebServerApi.Exceptions
{
    [Serializable]
    public class ServerNotFoundException : Exception
    {
        public ServerNotFoundException()
        {

        }
        public ServerNotFoundException(string msg) : base(msg)
        {

        }

        public ServerNotFoundException(string msg, Exception inner) : base(msg, inner)
        {

        }
    }

}
