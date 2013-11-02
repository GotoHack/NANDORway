namespace NANDORway {
	using System;
	using System.Diagnostics;
	using System.Reflection;
	using System.Windows.Forms;

	internal static class Program {
		/// <summary>
		///     The main entry point for the application.
		/// </summary>
		[STAThread] private static void Main() {
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomainAssemblyResolve;
#if DEBUG
            Debug.Listeners.Add(new TextWriterTraceListener(Console.Out));
#endif
			Application.Run(new Main());
		}

        private static Assembly CurrentDomainAssemblyResolve(object sender, ResolveEventArgs args)
        {
            if (string.IsNullOrEmpty(args.Name))
                throw new Exception("DLL Read Failure (Nothing to load!)");
            var name = string.Format("{0}.dll", args.Name.Substring(0, args.Name.IndexOf(',')));
            Debug.WriteLine(string.Format("[{0}] Looking for internal {1}", typeof(Program).Namespace, name));
            using (var stream = Assembly.GetAssembly(typeof(Program)).GetManifestResourceStream(string.Format("{0}.{1}", typeof(Program).Namespace, name)))
            {
                if (stream != null)
                {
                    var data = new byte[stream.Length];
                    stream.Read(data, 0, data.Length);
                    return Assembly.Load(data);
                }
                return LibNANDORway.LibMain.CurrentDomainAssemblyResolve(sender, args); //Try the lib aswell
            }
        }
	}
}