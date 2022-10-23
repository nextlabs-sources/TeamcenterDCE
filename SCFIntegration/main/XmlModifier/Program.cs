using System;
using System.Linq;
using System.Diagnostics;
using System.IO;
using System.Xml.Linq;

namespace XmlModifier
{
    class Program
    {
        private static string logFilePath;
        private static bool logEnable;

        static Program() {
            string tempDir = Path.GetTempPath();
            string logFile = "XmlModifier." + Process.GetCurrentProcess().Id.ToString() + ".log";
            logFilePath = Path.Combine(tempDir, logFile);

            // Logfile will only enable if environment variable NXL_SCF_LOG is set.
            string nxlSCFLog = Environment.GetEnvironmentVariable("NXL_SCF_LOG");
            if (nxlSCFLog != null) {
                logEnable = true;
            } else {
                logEnable = false;
            }
        }

        static void Main(string[] args)
        {
            if (args.Length == 0) return;

            string fmsTicket = args[0];
            DebugLog("FMSTicket='{0}'", fmsTicket);
            string decodedFMSTicket = Uri.UnescapeDataString(fmsTicket);

            string tcXMLDefaultLocation = GetXMLDefaultLocation(decodedFMSTicket);
            string transientVolumeDir = GetTransientVolumeDir(args);
            string tcXMLFileLocation = GetTcXMLFileLocation(tcXMLDefaultLocation, transientVolumeDir);

            if (File.Exists(tcXMLFileLocation)) {
                EditTcXMLFile(tcXMLFileLocation);
            }
        }

        static string GetXMLDefaultLocation(string decodedFMSTicket) {
            DebugLog("DecodedFMSTicket='{0}'", decodedFMSTicket);

            // FMS Ticket (after decoded) is break into different section with each section contains some number of characters
            // We will break down the ticket with all the known field (with dedicated character count)
            // Location of Tc XML File is the last section
            var ticketSections = new[]{
                new {Name="HASH", Len=64},
                new {Name="VER", Len=4},
                new {Name="ACCESS_METHOD", Len=1},
                new {Name="BINARY_FLAG", Len=1},
                new {Name="FILE_GUID", Len=32},
                new {Name="T_DELETE_FLAG", Len=1},
                new {Name="T_EXPIRATION", Len=19},
                new {Name="T_SITE_ID", Len=32},
                new {Name="T_USER_ID", Len=32},
                new {Name="T_VOLUMN_ID", Len=32},
                new {Name="T_DEFAULT_URL", Len=0}
            };
            int index = 0;
            string tcXMLFile = "";

            foreach (var section in ticketSections) {
                if (index > decodedFMSTicket.Length) break;
                if (section.Len == 0)
                {
                    // When come to T_DEFAULT_URL section, representing TcXML File. Collect this information and break out
                    tcXMLFile = decodedFMSTicket.Substring(index);
                    DebugLog("{0}={1}", section.Name, tcXMLFile);
                    break;
                }
                else
                {
                    DebugLog("{0}={1}", section.Name, decodedFMSTicket.Substring(index, section.Len));
                    index += section.Len;
                }
            }

            return tcXMLFile;
        }

        static string GetTcXMLFileLocation(string tcXMLDefaultLocation, string transientVolumeDir) {
            string tcXMLFileLocation = "";
            if (tcXMLDefaultLocation.StartsWith(";"))
            {
                tcXMLFileLocation = tcXMLDefaultLocation.Substring(1);
                DebugLog("Raw TcXML File Location = '{0}'...", tcXMLFileLocation);
            }
            else if (tcXMLDefaultLocation.StartsWith("http"))
            {
                // Potentially 4-tier environment ??
                tcXMLFileLocation = tcXMLDefaultLocation.Substring(tcXMLDefaultLocation.IndexOf(";") + 1);
                DebugLog("HTTP TcXML File Location = '{0}'...", tcXMLFileLocation);
            }
            else if (tcXMLDefaultLocation.StartsWith("\\"))
            {
                // Relative path: Combine File Location with Transient Volume Directory
                if (transientVolumeDir != null)
                {
                    tcXMLFileLocation = transientVolumeDir + tcXMLDefaultLocation;
                }
                DebugLog("Full TcXML File Location = '{0}'...", tcXMLFileLocation);
            }

            return tcXMLFileLocation;
        }

        static string GetTransientVolumeDir(string[] argumentList) {
            int index = 1; // Index 0 is FMSTicket. Should start from 1
            string volumeDir;
            while (index < argumentList.Length - 1)
            {
                DebugLog("Args[{0}]='{1}'='{2}'", index, argumentList[index], argumentList[index + 1]);
                if (argumentList[index] == "-transientvolume")
                {
                    volumeDir = argumentList[index + 1];
                    return volumeDir;
                }
                index ++;
            }
            return null;
        }

        static void EditTcXMLFile(string tcXMLFileLocation) {
            try
            {
                // Backup original Tc XML File
                string tcXMLBackup = string.Format("{0}.bak", tcXMLFileLocation);
                if (File.Exists(tcXMLBackup)) {
                    File.Delete(tcXMLBackup);
                }
                File.Copy(tcXMLFileLocation, tcXMLBackup);

                XDocument xdoc = XDocument.Load(tcXMLFileLocation, LoadOptions.PreserveWhitespace);
                DebugLog("TcXML Load option: PreserveWhitespace");
                if (xdoc != null)
                {
                    DebugLog("RootName={0}", xdoc.Root.Name);
                    var protectedPrtFiles = xdoc.Root.Elements().Where(e => e.Attribute("original_file_name") != null && e.Attribute("original_file_name").Value.EndsWith(".prt.nxl"));

                    DebugLog("NextLab Protected PRT Files = {0}", protectedPrtFiles.Count());
                    foreach (var fileElement in protectedPrtFiles)
                    {
                        var attribute = fileElement.Attribute("original_file_name");
                        if (attribute != null) {
                            var oldValue = attribute.Value;
                            if (oldValue.EndsWith(".prt.nxl", StringComparison.OrdinalIgnoreCase)) {
                                var newValue = oldValue.Substring(0, oldValue.Length - 4);
                                attribute.Value = newValue;
                                DebugLog("Element={0}:original_file_name={1}(OldValue={2})", fileElement.Name, newValue, oldValue);
                            } else {
                                DebugLog("Element={0}:original_file_name={1}", fileElement.Name, oldValue);
                            }
                        } else {
                            DebugLog("Element={0}", fileElement.Name);
                        }
                    }
                    // Re-write whole content back to TcXML File.
                    File.WriteAllText(tcXMLFileLocation, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" + xdoc.ToString());
                }
            }
            catch (Exception ex)
            {
                DebugLog("ERROR:{0}", ex.StackTrace);
            }
        }

        static void DebugLog(string logFormat, params object[] arguments)
        {
            if (logEnable) {
                using (StreamWriter sw = File.AppendText(logFilePath)) {
                    sw.WriteLine("{0}: {1}", DateTime.Now.ToString("[yyyy-MM-dd HH:mm:ss]"), string.Format("xmlmodifier!" + logFormat, arguments));
                }
            }
            Trace.WriteLine(string.Format("xmlmodifier!" + logFormat, arguments));
        }
    }
}
