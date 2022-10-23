using System;
using System.Linq;
using System.IO;
using Microsoft.Win32;
using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public enum RMXCustomActionType
    {
        CA_Install, CA_Uninstall, CA_Repair, CA_ValidateSldWorksDir, CA_FindSldWorksDir, CA_ValidateSwimDir, CA_FindSwimDir
    }
    public abstract class RMXCustomAction
    {
        public Session session;
        public const string NEXTLABS_CA_SEPARATOR = "----------------------------";
        public RMXCustomAction(Session session)
        {
            this.session = session;
        }

        public abstract ActionResult Execute();

        public void Log(string format, params object[] args)
        {
            session.Log(format, args);
        }
    }

    public class InstallAction : RMXCustomAction
    {
        public InstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - InstallSldWorksRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string swimDir = session.CustomActionData["SWIM"];

            // In unmanaged mode, SWIM="C:", so let check if this is valid SWIM
            if (!File.Exists(Path.Combine(swimDir, "swim.jar")))
            {
                Log("Unmanaged mode Installation");
            }
            else
            {
                // Also, need to modify [SWIM]/swimrunner.bat to include swimrmx.jar into classpath
                string swimClasspath = Path.Combine(swimDir, "swimrunner.bat");
                if (File.Exists(swimClasspath)) {
                    Log("Add NextLabs Classpath into swimrunner.bat");
                    File.Copy(swimClasspath, swimClasspath + ".backup");
                    string[] allLines = File.ReadAllLines(swimClasspath);
                    var insertIndex = Array.FindIndex(allLines, x => x.Contains("%SWIM_DIR%\\swim_manifest.jar"));
                    if (insertIndex > -1) {
                        // The entry exist
                        string[] jarFiles = { "swimrmx.jar", "aspectjweaver.jar", "aspectjrt.jar" };
                        var processing = allLines.ToList();
                        foreach (string jar in jarFiles) {
                            string classPathToAdd = "call \"%SWIM_DIR%\\add_classpath\" %CADRMX_DIR%\\" + jar;
                            processing.Insert(insertIndex + 1, classPathToAdd);
                        }

                        string swimRmxFolder = Path.Combine(installFolder, "swim");
                        string lineToAdd = "set CADRMX_DIR=" + swimRmxFolder;
                        processing.Insert(insertIndex + 1, lineToAdd);
                        File.WriteAllLines(swimClasspath, processing.ToArray());
                    }

                    // One more point to do line insertion
                    allLines = File.ReadAllLines(swimClasspath);
                    insertIndex = Array.FindIndex(allLines, x => x.Contains("DIMAN_PORTAL_INSTRFILE"));
                    if (insertIndex > -1) {
                        // The entry exist
                        var processing = allLines.ToList();
                        string lineToAdd = "set ARGS=%ARGS% \"-javaagent:%CADRMX_DIR%\\aspectjweaver.jar\"";
                        processing.Insert(insertIndex + 1, lineToAdd);
                        File.WriteAllLines(swimClasspath, processing.ToArray());
                    }
                }
            }

            Log("{0} NextLabs Custom Action - InstallSldWorksRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class UninstallAction : RMXCustomAction
    {
        public UninstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - UninstallSldWorksRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            RegistryKey sldWorksDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\SldWorksRMX");
            string sldWorksDir = sldWorksDirRegistry.GetValue("SldWorksDir").ToString();
            string swimDir = sldWorksDirRegistry.GetValue("SwimDir").ToString();

            if (!File.Exists(Path.Combine(swimDir, "swim.jar")))
            {
                Log("Unmanaged Mode Uninstallation");
            }
            else
            {
                // Recover swimrunner.bat
                string swimClasspath = Path.Combine(swimDir, "swimrunner.bat");
                if (File.Exists(swimClasspath + ".backup")) {
                    File.Delete(swimClasspath);
                    File.Move(swimClasspath + ".backup", swimClasspath);
                }
            }

            Log("{0} NextLabs Custom Action - UninstallSldWorksRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }

        public void deleteFileFilter(string folderName, string filter)
        {
            DirectoryInfo directory = new DirectoryInfo(folderName);

            foreach (FileInfo file in directory.GetFiles())
            {
                if (file.Extension == filter)
                {
                    file.Delete();
                }
            }

            foreach (DirectoryInfo dir in directory.GetDirectories())
            {
                deleteFileFilter(dir.FullName, filter);
            }
        }
    }

    public class RepairAction : RMXCustomAction
    {
        public RepairAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - RepairSldWorksRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Find CreoDir and Plugin's InstallDir stored in registry to reinstall
            RegistryKey sldWorksDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\SldWorksRMX");
            string sldWorksDir = sldWorksDirRegistry.GetValue("SldWorksDir").ToString();
            string installDir = sldWorksDirRegistry.GetValue("InstallDir").ToString();

            Log("{0} NextLabs Custom Action - RepairSldWorksRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateSldWorksDirAction : RMXCustomAction
    {
        public ValidateSldWorksDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateSldWorksDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string sldWorksDir = session["SLDWORKS_DIR"];
            Log("Validating following SolidWorks Installation Directory: {0}", sldWorksDir);

            string solidWorksExeFile = Path.Combine(sldWorksDir, "SOLIDWORKS", "SLDWORKS.exe");
            bool validPath = Directory.Exists(sldWorksDir) && File.Exists(solidWorksExeFile);
            if (string.IsNullOrEmpty(session["SLDWORKS_DIR_VALID"]))
            {
                session["SLDWORKS_DIR_VALID"] = validPath ? "1" : "0";
            }
            else
            {
                session["SLDWORKS_DIR_VALID"] = null;
                session["SLDWORKS_DIR_VALID"] = validPath ? "1" : "0";
            }

            Log("After validation, SLDWORKS_DIR_VALID = {0}", session["SLDWORKS_DIR_VALID"]);
            Log("{0} NextLabs Custom Action - ValidateSldWorksDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindSldWorksDirAction : RMXCustomAction
    {
        public FindSldWorksDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindSldWorksDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string sldWorksKey = "SOFTWARE\\Wow6432Node\\SolidWorks\\IM";

            RegistryKey key = Registry.LocalMachine.OpenSubKey(sldWorksKey);
            if (key == null)
            {
                return ActionResult.Failure;
            }
            else
            {
                // Only care at SolidWorks 2019 atm
                try
                {
                    session["SLDWORKS_DIR"] = key.GetValue("InstallDir 2019").ToString();
                    Log("Retrieved value InstallDir 2019 = {0}", session["SLDWORKS_DIR"]);
                }
                catch (Exception ex)
                {
                    return ActionResult.Failure;
                }
            }

            Log("{0} NextLabs Custom Action - FindSldWorksDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateSwimDirAction : RMXCustomAction
    {
        public ValidateSwimDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateSwimDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string swimDir = session["SWIM"];
            Log("Validating following SWIM Installation Directory: {0}", swimDir);

            string swimJarFile = Path.Combine(swimDir, "swim.jar");
            bool validPath = (Directory.Exists(swimDir) && File.Exists(swimJarFile));
            session["SWIM_DIR_VALID"] = validPath ? "1" : "0";

            Log("After validation, SWIM_DIR_VALID = {0}", session["SWIM_DIR_VALID"]);
            Log("{0} NextLabs Custom Action - ValidateSwimDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindSwimDirAction : RMXCustomAction
    {
        public const string TC_SWIM_NAME = "Teamcenter Integration for SolidWorks";
        public FindSwimDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindSwimDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string controlPanelProgram = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

            RegistryKey programList = Registry.LocalMachine.OpenSubKey(controlPanelProgram);
            foreach (string program in programList.GetSubKeyNames())
            {
                if (program.Contains(TC_SWIM_NAME))
                {
                    RegistryKey tcSwim = programList.OpenSubKey(program);
                    string tcSwimInstallLocation = tcSwim.GetValue("InstallLocation").ToString();
                    Log("Found Teamcenter Integration for SolidWorks at : " + tcSwimInstallLocation);
                    session["SWIM"] = tcSwimInstallLocation;
                    // Set FOUND_SWIM to 1 to go through SWIM page
                    session["FOUND_SWIM"] = "1";
                    return ActionResult.Success;
                }
            }

            // In case not found Tc SWIM dir then set to any valid directory. And set FOUND_SWIM to 0 to skip UI page
            session["SWIM"] = "C:";
            session["FOUND_SWIM"] = "0";

            Log("{0} NextLabs Custom Action - FindSldWorksDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class RMXCustomActionFactory
    {
        public static RMXCustomAction CreateCustomAction(RMXCustomActionType type, Session session)
        {
            RMXCustomAction action;
            switch (type)
            {
                case RMXCustomActionType.CA_Install:
                    action = new InstallAction(session);
                    break;
                case RMXCustomActionType.CA_Uninstall:
                    action = new UninstallAction(session);
                    break;
                case RMXCustomActionType.CA_Repair:
                    action = new RepairAction(session);
                    break;
                case RMXCustomActionType.CA_ValidateSldWorksDir:
                    action = new ValidateSldWorksDirAction(session);
                    break;
                case RMXCustomActionType.CA_FindSldWorksDir:
                    action = new FindSldWorksDirAction(session);
                    break;
                case RMXCustomActionType.CA_ValidateSwimDir:
                    action = new ValidateSwimDirAction(session);
                    break;
                case RMXCustomActionType.CA_FindSwimDir:
                    action = new FindSwimDirAction(session);
                    break;
                default:
                    action = null;
                    break;
            }
            return action;
        }
    }
}
