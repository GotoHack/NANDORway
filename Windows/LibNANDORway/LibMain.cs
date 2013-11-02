namespace LibNANDORway {
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Reflection;

    public static class LibMain {
        public const int VID = 0xB00B;
        public const int PID = 0xBABE;
        private static readonly string BaseName;
        private static readonly Version Ver = Assembly.GetAssembly(typeof(LibMain)).GetName().Version;

        static LibMain() {
            BaseName = string.Format("{0} v{{0}}.{{1}} (Build: {{2}}) {{3}}", typeof(LibMain).Namespace);
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomainAssemblyResolve;
#if DEBUG
            Debug.Listeners.Add(new TextWriterTraceListener(Console.Out));
#endif
        }

        public static string LibVersion {
            get {
#if DEBUG
                return string.Format(BaseName, Ver.Major, Ver.Minor, Ver.Build, "[ DEBUG BUILD ]");
#else
                return string.Format(BaseName, Ver.Major, Ver.Minor, Ver.Build, "");
#endif
            }
        }

        public static bool DeviceConnected { get; private set; }

        public static event EventHandler<EventArg<bool>> ConnectedChanged;

        internal static void OnConnectedChanged(bool state) {
            DeviceConnected = state;
            var handler = ConnectedChanged;
            if(handler != null)
                handler(null, new EventArg<bool>(state));
        }

        public static event EventHandler<EventArg<string>> StatusChanged;

        internal static void UpdateStatus(string message, object arg0, object arg1, object arg2) {
            message = string.Format(message, arg0, arg1, arg2);
            var handler = StatusChanged;
            if(handler != null)
                handler(null, new EventArg<string>(message));
        }

        internal static void UpdateStatus(string message, object arg0, object arg1) {
            message = string.Format(message, arg0, arg1);
            var handler = StatusChanged;
            if(handler != null)
                handler(null, new EventArg<string>(message));
        }

        internal static void UpdateStatus(string message, object arg0) {
            message = string.Format(message, arg0);
            var handler = StatusChanged;
            if(handler != null)
                handler(null, new EventArg<string>(message));
        }

        internal static void UpdateStatus(string message, params object[] args) {
            if(args.Length > 0)
                message = string.Format(message, args);
            var handler = StatusChanged;
            if(handler != null)
                handler(null, new EventArg<string>(message));
        }

        public static event EventHandler<ProgressEventArg<long, long>> Progress;

        internal static void UpdateProgress(long current, long total) {
            var handler = Progress;
            if(handler != null)
                handler(null, new ProgressEventArg<long, long>(current, total));
        }

        public static Assembly CurrentDomainAssemblyResolve(object sender, ResolveEventArgs args) {
            if(string.IsNullOrEmpty(args.Name))
                throw new Exception("DLL Read Failure (Nothing to load!)");
            var name = string.Format("{0}.dll", args.Name.Substring(0, args.Name.IndexOf(',')));
            Debug.WriteLine(string.Format("[{0}] Looking for internal {1}", typeof(LibMain).Namespace, name));
            using(var stream = Assembly.GetAssembly(typeof(LibMain)).GetManifestResourceStream(string.Format("{0}.{1}", typeof(LibMain).Namespace, name))) {
                if(stream != null) {
                    var data = new byte[stream.Length];
                    stream.Read(data, 0, data.Length);
                    return Assembly.Load(data);
                }
                throw new FileNotFoundException(string.Format("Can't find external nor internal {0}!", name));
            }
        }
    }
}