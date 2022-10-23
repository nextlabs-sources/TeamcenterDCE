using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;
using System.Configuration;
using System.Globalization;

namespace Nextlabs.ProxyRunner
{
    static class Utils
    {
        public const string NxlExtension = ".nxl";
        public const string CFG_KEY_TCRUNNER = "TeamcenterRunner";
        public const string CFG_KEY_NXLRUNNER = "NxlRunner";
        public const string CFG_KEY_ARGENCODING = "ArgEncoding";

        public static void DebugLog(string logfmt, params object[] args)
        {
            Trace.WriteLine(string.Format("proxyRunner!" + logfmt, args));
        }
        public static int ShowError(string message, int code)
        {
            DebugLog("ERROR-{1}:{0}", message, code);
            MessageBox.Show(message, string.Format("Error({0}) - Nextlabs ProxyRunner", code), MessageBoxButtons.OK, MessageBoxIcon.Error);
            return code;
        }
        private static string s_thisRunner = null;
        public static string ThisRunner
        {
            get { return s_thisRunner ?? (s_thisRunner = Path.GetFullPath(Application.ExecutablePath)); }
        }
        public static bool IsValidPath(string path)
        {
            try
            {
                if (!string.IsNullOrEmpty(path)
                    && Path.IsPathRooted(path))
                {
                    var fullPath = Path.GetFullPath(path);
#if DEBUG
                    Utils.DebugLog("GetFullPath={0}", fullPath);
#endif
                    return true;
                }
            }
            catch (Exception ex)
            {
                Utils.DebugLog("Failed to GetFullPath('{0}'):{0}", path, ex.Message);
            }
            return false;
        }
        public static string ReadConfigItem(string key)
        {
            if (string.IsNullOrEmpty(key)) return string.Empty;
            try
            {
                var appSettings = ConfigurationManager.AppSettings;

                if (appSettings != null && appSettings.Count > 0)
                {
                    var value = appSettings[key];
#if DEBUG
                    Utils.DebugLog("AppSettings['{0}']='{1}'", key, value);
#endif
                    return value;
                }
            }
            catch (ConfigurationErrorsException ex)
            {
                Utils.DebugLog("Failed to read app settings:{0}", ex.Message);
            }
            return string.Empty;
        }
        public static bool SaveConfigItem(string key, string value)
        {
            if (string.IsNullOrEmpty(key) || string.IsNullOrEmpty(value)) return false;
            try
            {
                //save to configuration
                var cfg = ConfigurationManager.OpenExeConfiguration(Utils.ThisRunner);
                cfg.AppSettings.Settings.Remove(key);
                cfg.AppSettings.Settings.Add(key, value);
                cfg.Save();
                return true;
            }
            catch (Exception ex)
            {
                Utils.DebugLog("Failed to write setting to configuration file {0}.config:{1}", Utils.ThisRunner, ex.Message);
            }
            return false;
        }
        private static Encoding GetEncoding()
        {
            Encoding encoding = null;
            var encodingName = Utils.ReadConfigItem(Utils.CFG_KEY_ARGENCODING);
            if (!string.IsNullOrEmpty(encodingName))
            {
                try
                {
                    encoding = Encoding.GetEncoding(encodingName);
                }
                catch (Exception ex)
                {
                    Utils.DebugLog("Failed get encoding by Name('{0}')(Exception:{1})", encodingName, ex.Message);
                }
            }
            if (encoding == null)
            {
                encoding = Encoding.BigEndianUnicode;
                Utils.SaveConfigItem(Utils.CFG_KEY_ARGENCODING, encoding.WebName);
            }
            Utils.DebugLog("EncodingName='{1}' WebName='{0}'", encoding.WebName, encoding.EncodingName);
            return encoding;
        }
        public static string DecodeArgString(string encodedArg)
        {
            if (string.IsNullOrEmpty(encodedArg)) return string.Empty;
            if (encodedArg.Length % 2 != 0)
            {
                DebugLog("Invalid string length-{0}", encodedArg.Length);
                return string.Empty;
            }
            //convert encoded string to bytes
            var nbytes = encodedArg.Length / 2;
            var bytes = new byte[nbytes];
            for (int i = 0; i < nbytes; i++)
            {
                var s = encodedArg.Substring(i * 2, 2);
                bytes[i] = Byte.Parse(s, NumberStyles.HexNumber);
            }
            //covert bytes to char[] by encoding
            var decodedString = new string(GetEncoding().GetChars(bytes));
            DebugLog("==>Decoded:'{0}'", decodedString);
            return decodedString;
        }
        public static bool Install()
        {
            string portalDir = Teamcenter.GetPortalDir();
            if (!string.IsNullOrEmpty(portalDir))
            {
                string runner = Path.Combine(portalDir, "runner.exe");
                string tcrunner = Path.Combine(portalDir, "tcrunner.exe");
                if (File.Exists(runner))
                {
                    int currentStep = 0;
                    string runnerbak = string.Format("{0}.{1}", runner, DateTime.Now.ToString("yyyyMMddHHmmss"));
                    try
                    {
                        //1, backup existing runner.exe
                        currentStep++;
                        Console.WriteLine("{0}.Backing up '{1}' to {2}", currentStep, runner, runnerbak);
                        File.Move(runner, runnerbak);

                        //2, copying proxyrunner as runner.exe
                        currentStep++;
                        Console.WriteLine("{0}.Copying '{1}' to {2}", currentStep, ThisRunner, runner);
                        File.Copy(ThisRunner, runner);

                        //3, create tcrunner.exe
                        currentStep++;
                        if (File.Exists(tcrunner))
                        {
                            Console.WriteLine("{1}.'{0}' already exists", tcrunner, currentStep);
                        }
                        else
                        {
                            //copy backup runner as tcrunner
                            Console.WriteLine("{2}.Copying '{0}' to {1}", runnerbak, tcrunner, currentStep);
                            File.Copy(runnerbak, tcrunner);
                        }

                        //4, setup configuration
                        currentStep++;
                        Console.WriteLine("4.Creating configuration file '{0}.config'", runner);
                        var cfg = ConfigurationManager.OpenExeConfiguration(runner);
                        cfg.AppSettings.Settings.Remove(CFG_KEY_TCRUNNER);
                        cfg.AppSettings.Settings.Add(CFG_KEY_TCRUNNER, tcrunner);
                        cfg.Save();

                        Console.WriteLine("Done!");
                        return true;
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("==>Failed:{0}", ex.Message);
                    }
                    //roll back
                    if (currentStep >= 4)
                    {
                        //don't need rollback
                        Console.WriteLine("Done with error.");
                        return true;
                    }
                    if (currentStep > 1)
                    {
                        //roll back step 1
                        File.Move(runnerbak, runner);
                    }
                    Console.WriteLine("Rolled back!");
                }
                else
                {
                    Console.WriteLine("Failed to find Original Teamcenter runner.exe");
                }
            }
            return false;
        }
    }
}
