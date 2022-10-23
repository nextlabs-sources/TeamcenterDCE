using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Configuration;

namespace Nextlabs.ProxyRunner
{
    class NxlRunner
    {
        public const string NxlRunnerName = "nxlrunner.exe";
        private static string m_nxlrunner = null;
        static string ConfigurationRunner
        {
            get
            {
                if (string.IsNullOrEmpty(m_nxlrunner))
                {
                    m_nxlrunner = Utils.ReadConfigItem(Utils.CFG_KEY_NXLRUNNER);
                }
                return m_nxlrunner;
            }
            set
            {
                m_nxlrunner = value;
                Utils.SaveConfigItem(Utils.CFG_KEY_NXLRUNNER, value);
            }
        }
        public static Process Start(RunnerContext ctx)
        {
            Process runnerProcess = null;
            string filePath = ctx.NxlFile;
            if (filePath != null)
            {
                //TODO:load path from configuration
                string nxlrunner = ConfigurationRunner;
                if (string.IsNullOrEmpty(nxlrunner)//no nxlrunner is defined in configuration
                    || !File.Exists(nxlrunner)) //nxlrunner defined in configuration cannot be found
                {
                    var rmxroot = Environment.GetEnvironmentVariable("RMX_ROOT");
                    if (!string.IsNullOrEmpty(rmxroot))
                    {
                        //try search nxlrunner from teamcenter portal
                        nxlrunner = Path.Combine(rmxroot, "bin32");
                        nxlrunner = Path.Combine(nxlrunner, NxlRunnerName);
                        //save config
                        ConfigurationRunner = nxlrunner;
                    }
                    else
                    {
                        //ask windows to search the command
                        nxlrunner = NxlRunnerName;
                    }
                }
                try
                {
                    runnerProcess = Process.Start(nxlrunner, string.Format("-open \"{0}\"", filePath));
                    if (runnerProcess != null)
                    {
                        Utils.DebugLog("{0}({3}) is started in process={1} from '{2}'", runnerProcess.ProcessName, runnerProcess.Id, filePath, nxlrunner);
                    }
                    else
                    {
                        Utils.DebugLog("Failed to start process-'{0}'", nxlrunner);
                    }
                }
                catch (Exception ex)
                {
                    Utils.DebugLog("Failed to start process-'{0}'(Error-{1})", nxlrunner, ex.Message);
                }
            }

            return runnerProcess;
        }
    }
}
