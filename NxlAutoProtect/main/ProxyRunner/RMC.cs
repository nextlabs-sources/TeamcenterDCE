using System;
using System.Collections.Generic;
using System.Text;
using System.ServiceProcess;
using System.IO;

namespace Nextlabs.ProxyRunner
{
    class RMC
    {
        const string SERVICE_RMC = "nxrmserv";
        public static bool IsInstalled()
        {
            try
            {
                using (var svc = new ServiceController(SERVICE_RMC))
                {
                    if (svc != null && svc.Status == ServiceControllerStatus.Running)
                    {
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                Utils.DebugLog("Failed to open RMC service({0})-{1}", SERVICE_RMC, ex.Message);
            }
            return false;
        }
        public static bool IsNxlFile(string file)
        {
            if (File.Exists(file))
            {
                using (TextReader rd = File.OpenText(file))
                {
                    char[] header = new char[6];
                    if (rd.Read(header, 0, header.Length) == header.Length)
                    {
                        string headerString = new string(header);
#if DEBUG
                        Utils.DebugLog("File Header:{0}", headerString);
#endif
                        if (string.Equals("NXLFMT", headerString, StringComparison.Ordinal))
                        {
                            return true;
                        }
                    }
                }
            }
            else
            {
                Utils.DebugLog("Failed to find file-{0}", file);
            }

            return false;
        }
    }
}
