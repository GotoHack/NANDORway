namespace NANDORway {
	using System;
	using System.Diagnostics;
	using System.Windows.Forms;

	internal static class Program {
        /// <summary>
		///     The main entry point for the application.
		/// </summary>
		[STAThread] private static void Main() {
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
            EmbeddedAssemblyManager.PreLoadAssemblies();
#if DEBUG
            Debug.Listeners.Add(new TextWriterTraceListener(Console.Out));
#endif
			Application.Run(new Main());
		}

	    
	}
}