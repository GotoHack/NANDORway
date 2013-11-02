namespace NANDORway {
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Reflection;

    internal static class EmbeddedAssemblyManager {
        private static readonly Dictionary<string, Assembly> Assemblies = new Dictionary<string, Assembly>();

        internal static void PreLoadAssemblies() {
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomainAssemblyResolve;
            Assemblies.Clear();
            PreLoadAssemblies(Assembly.GetAssembly(typeof(EmbeddedAssemblyManager)));
            var keys = new string[Assemblies.Count];
            Assemblies.Keys.CopyTo(keys, 0);
            foreach(var key in keys)
                PreLoadAssemblies(Assemblies[key]);
        }

        private static void PreLoadAssemblies(Assembly sourceAssembly) {
            foreach(var assemblyname in sourceAssembly.GetManifestResourceNames()) {
                Debug.WriteLine(assemblyname);
                if(!assemblyname.EndsWith(".dll", StringComparison.CurrentCultureIgnoreCase))
                    continue;
                var ass = LoadEmbeddedAssembly(sourceAssembly, assemblyname);
                Assemblies.Add(ass.FullName, ass);
            }
        }

        private static Assembly LoadEmbeddedAssembly(Assembly sourceAssembly, string name) {
            using(var stream = sourceAssembly.GetManifestResourceStream(name)) {
                if(stream != null) {
                    var data = new byte[stream.Length];
                    stream.Read(data, 0, data.Length);
                    return Assembly.Load(data);
                }
                throw new FileNotFoundException(string.Format("Can't find external nor internal {0}!", name));
            }
        }

        internal static Assembly CurrentDomainAssemblyResolve(object sender, ResolveEventArgs args) {
            return Assemblies[args.Name];
        }
    }
}