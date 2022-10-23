using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.Configuration;
using System.IO;

namespace Nextlabs.ProxyRunner
{
    class Teamcenter
    {
        public static string GetPortalDir()
        {
            string tcRoot = Environment.GetEnvironmentVariable("TC_ROOT");
            Utils.DebugLog("TC_ROOT='{0}'", tcRoot ?? "<null>");
            if (!string.IsNullOrEmpty(tcRoot)
                && Directory.Exists(tcRoot))
            {
                string tcportal = Path.Combine(tcRoot, "portal");
                return Directory.Exists(tcportal) ? Path.GetFullPath(tcportal) : string.Empty;
            }
            return string.Empty;
        }

        static bool IsValidTcRunner(string tcrunner)
        {
            return !string.IsNullOrEmpty(tcrunner)
                && !string.Equals(tcrunner, Utils.ThisRunner, StringComparison.OrdinalIgnoreCase)
                && File.Exists(tcrunner);
        }
        private static string m_cfgRunner = null;
        static string ConfigurationRunner {
            get
            {
                if (string.IsNullOrEmpty(m_cfgRunner))
                {
                    m_cfgRunner = Utils.ReadConfigItem(Utils.CFG_KEY_TCRUNNER);
                }
                return m_cfgRunner;
            }
            set
            {
                m_cfgRunner = value;
                Utils.SaveConfigItem(Utils.CFG_KEY_TCRUNNER, value);
            }
        }
        static string GetTeamcenterRunner()
        {
            string cfgRunner = ConfigurationRunner;
            if (IsValidTcRunner(cfgRunner))
            {
                return cfgRunner;
            }
            Utils.DebugLog("Failed to locate Teamcenter runner from config file-{0}={1}", Utils.CFG_KEY_TCRUNNER, cfgRunner);
            //configuration file is not well defined, searching runner in default folders
            string[] targetDirectories = { 
                                             GetPortalDir(),    //teamcenter portal directory
                                             Path.GetDirectoryName(Utils.ThisRunner), //current assembly directory
                                             Path.GetFullPath(Environment.CurrentDirectory) //current working directory
                                         };
            string[] targetRunners = { "runner.exe", "tcrunner.exe" };
            foreach (var directory in targetDirectories)
            {
                if (!string.IsNullOrEmpty(directory))
                {
                    Utils.DebugLog("Searching '{0}' ...", directory);
                    if (Directory.Exists(directory))
                    {
                        foreach (var name in targetRunners)
                        {
                            var runner = Path.Combine(directory, name);
                            if (IsValidTcRunner(runner))
                            {
                                //update configuration
                                ConfigurationRunner = runner;
                                return runner;
                            }
                        }
                    }
                }
            }
            //try searching current directory
            return string.Empty;
        }
        public static Process StartRunner(RunnerContext ctx)
        {
            string tcrunner = GetTeamcenterRunner();
            if (!string.IsNullOrEmpty(tcrunner))
            {
                Utils.DebugLog("TeamcenterRunner:'{0}'", tcrunner);
                //build argument string
                StringBuilder argBuilder = new StringBuilder();
                foreach (var arg in ctx.InputArgs)
                {
                    //TODO:handle " and \
                    if (arg.IndexOfAny(new[] { ' ', '\t' }) >= 0)
                    {
                        argBuilder.AppendFormat(" \"{0}\"", arg);
                    }
                    else
                    {
                        argBuilder.AppendFormat(" {0}", arg);
                    }
                }
                //start tc runner
                string argstr = argBuilder.ToString();
                Utils.DebugLog("Arrguments:'{0}'", argstr);
                var runnerProcess = Process.Start(tcrunner, argstr);
                if (runnerProcess != null)
                {
                    Utils.DebugLog("{0} is started in process={1}", tcrunner, runnerProcess.Id);
                }
                else
                {
                    Utils.DebugLog("Failed to start process-'{0}'", tcrunner);
                }
                return runnerProcess;
            }
            return null;
        }
    }
}
