using System;
using System.Linq;
using System.IO;
using Microsoft.Win32;
using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public enum RMXCustomActionType
    {
        CA_Install, CA_Uninstall, CA_Repair, CA_ValidateCreoDir, CA_FindCreoDir, CA_EditCreoTkDat, CA_FindIpemDir, CA_ValidateIpemDir
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

        public void RemoveFileReadOnlyAttribute(string filePath)
        {
            FileInfo fileInfo = new FileInfo(filePath);
            fileInfo.IsReadOnly = false;
        }
    }

    public class InstallAction : RMXCustomAction
    {
        public InstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - InstallCreoRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string creoDir = session.CustomActionData["CREO_DIR"];
            string ipemDir = session.CustomActionData["IPEM"];

            string nxlCreoPlugin;

            if (creoDir.Contains("Creo 7"))
            {
                Log("Using dll files for Creo 7.0.");
                nxlCreoPlugin = "creotkdat $CREO_RMX_ROOT\\Creo 7.0\\creotk.dat";
                nxlCreoPlugin = nxlCreoPlugin.Replace("$CREO_RMX_ROOT\\", session.CustomActionData["INSTALLFOLDER"]);
            }
            else  // Other support is Creo 4.0
            {
                Log("Using dll files for Creo 4.0.");
                nxlCreoPlugin = "creotkdat $CREO_RMX_ROOT\\Creo 4.0\\creotk.dat";
                nxlCreoPlugin = nxlCreoPlugin.Replace("$CREO_RMX_ROOT\\", session.CustomActionData["INSTALLFOLDER"]);
            }

            // Deploy CreoRMX Plugin into config.pro file
            string creoRootFolder = Path.GetFullPath(Path.Combine(creoDir, ".."));
            string creoConfigPro = Path.Combine(creoRootFolder, "Common Files", "text", "config.pro");
            if (File.Exists(creoConfigPro) && !File.ReadAllText(creoConfigPro).Contains(nxlCreoPlugin))
            {
                Log("Add NextLabs Creo Plugin into config.pro");
                using (StreamWriter sw = File.AppendText(creoConfigPro))
                {
                    sw.Write(Environment.NewLine);
                    sw.Write(nxlCreoPlugin);
                }
            }
            else if (!File.Exists(creoConfigPro))
            {
                Log("Creo config.pro not exist in " + creoConfigPro);
                Log("Custom configuration file expected. Installer will create a new config.pro file with NextLabs Creo RMX config.");
                using (StreamWriter sw = File.CreateText(creoConfigPro)) {
                    sw.WriteLine(nxlCreoPlugin);
                }
            }
            else
            {
                Log("NextLabs Creo Plugin already deployed");
            }

            // In unmanaged mode, IPEM="C:", so let check if this is valid IPEM
            if (!File.Exists(Path.Combine(ipemDir, "ipem.jar")))
            {
                Log("Unmanaged mode Installation");
            }
            else
            {
                // Need to modify [IPEM]/ipemrunnersoa.bat to include ipemrmx.jar into classpath
                string ipemClasspath = Path.Combine(ipemDir, "ipemrunnersoa.bat");
                if (File.Exists(ipemClasspath)) {
                    Log("Add NextLabs Classpath into ipemrunnersoa.bat");
                    File.Copy(ipemClasspath, ipemClasspath + ".backup");
                    string[] allLines = File.ReadAllLines(ipemClasspath);
                    var insertIndex = Array.FindIndex(allLines, x => x.Contains("%IPEM_DIR%\\ipem_manifest.jar"));
                    if (insertIndex > -1) {
                        // The entry exist
                        string[] jarFiles = { "ipemrmx.jar", "aspectjweaver.jar", "aspectjrt.jar" };
                        var processing = allLines.ToList();
                        foreach (string jar in jarFiles) {
                            string classPathToAdd = "call \"%IPEM_DIR%\\add_classpath\" %CADRMX_DIR%\\" + jar;
                            processing.Insert(insertIndex + 1, classPathToAdd);
                        }

                        // CADRMX_DIR base on Creo version
                        string ipemRmxFolder;
                        if (creoDir.Contains("Creo 7")) {
                            ipemRmxFolder = Path.Combine(installFolder, "Creo 7.0", "ipem");
                        }
                        else {
                            ipemRmxFolder = Path.Combine(installFolder, "Creo 4.0", "ipem");
                        }
                        string lineToAdd = "set CADRMX_DIR=" + ipemRmxFolder;
                        processing.Insert(insertIndex + 1, lineToAdd);
                        File.WriteAllLines(ipemClasspath, processing.ToArray());
                    }

                    // One more point to do line insertion
                    allLines = File.ReadAllLines(ipemClasspath);
                    insertIndex = Array.FindIndex(allLines, x => x.Contains("Djacorb.home="));
                    if (insertIndex > -1) {
                        // The entry exist
                        var processing = allLines.ToList();
                        string lineToAdd = "set ARGS=%ARGS% \"-javaagent:%CADRMX_DIR%\\aspectjweaver.jar\"";
                        processing.Insert(insertIndex + 1, lineToAdd);
                        File.WriteAllLines(ipemClasspath, processing.ToArray());
                    }
                }
            }

            Log("{0} NextLabs Custom Action - InstallCreoRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class UninstallAction : RMXCustomAction
    {
        public UninstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - UninstallCreoRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            RegistryKey creoDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\CreoRMX");
            string creoDir = creoDirRegistry.GetValue("CreoDir").ToString();
            string installDir = creoDirRegistry.GetValue("InstallDir").ToString();
            string ipemDir = creoDirRegistry.GetValue("IpemDir").ToString();

            string nxlCreoPlugin;

            // Detect Creo version and binary files installed
            if (creoDir.Contains("Creo 7"))
            {
                Log("Plugin dll files deployed for Creo 7.0.");
                nxlCreoPlugin = "creotkdat $CREO_RMX_ROOT\\Creo 7.0\\creotk.dat";
                nxlCreoPlugin = nxlCreoPlugin.Replace("$CREO_RMX_ROOT\\", installDir);
            }
            else
            {
                Log("Plugin dll files deployed for Creo 4.0.");
                nxlCreoPlugin = "creotkdat $CREO_RMX_ROOT\\Creo 4.0\\creotk.dat";
                nxlCreoPlugin = nxlCreoPlugin.Replace("$CREO_RMX_ROOT\\", installDir);
            }

            // Remove CreoRMX plugin from config.pro file
            string creoRootFolder = Path.GetFullPath(Path.Combine(creoDir, ".."));
            Log("Remove NextLabs Creo Plugin from config.pro");
            string creoConfigPro = Path.Combine(creoRootFolder, "Common Files", "text", "config.pro");

            if (File.Exists(creoConfigPro) && File.ReadAllText(creoConfigPro).Contains(nxlCreoPlugin))
            {
                File.WriteAllLines(creoConfigPro, File.ReadAllLines(creoConfigPro).Where(line => !(line.Contains(nxlCreoPlugin) || string.IsNullOrEmpty(line))).ToList());
            }

            // In managed mode, remove NextLabs addition class file inside IPEM dir
            if (!File.Exists(Path.Combine(ipemDir, "ipem.jar")))
            {
                Log("Unmanaged Mode Uninstallation");
            }
            else
            {
                // Recover ipemrunnersoa.bat
                string ipemClasspath = Path.Combine(ipemDir, "ipemrunnersoa.bat");
                if (File.Exists(ipemClasspath + ".backup")) {
                    File.Delete(ipemClasspath);
                    File.Move(ipemClasspath + ".backup", ipemClasspath);
                }
            }

            Log("{0} NextLabs Custom Action - UninstallCreoRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class RepairAction : RMXCustomAction
    {
        public RepairAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - RepairCreoRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Find CreoDir and Plugin's InstallDir stored in registry to reinstall
            RegistryKey creoDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\CreoRMX");
            string creoDir = creoDirRegistry.GetValue("CreoDir").ToString();
            string installDir = creoDirRegistry.GetValue("InstallDir").ToString();
            
            string nxlCreoPlugin;

            if (creoDir.Contains("Creo 7"))
            {
                Log("Using dll files for Creo 7.0.");
                nxlCreoPlugin = "creotkdat $CREO_RMX_ROOT\\Creo 7.0\\creotk.dat";
                nxlCreoPlugin = nxlCreoPlugin.Replace("$CREO_RMX_ROOT\\", installDir);
            }
            else
            {
                Log("Using dll files for Creo 4.0.");
                nxlCreoPlugin = "creotkdat $CREO_RMX_ROOT\\Creo 4.0\\creotk.dat";
                nxlCreoPlugin = nxlCreoPlugin.Replace("$CREO_RMX_ROOT\\", installDir);
            }

            // Deploy CreoRMX Plugin into config.pro file
            string creoRootFolder = Path.GetFullPath(Path.Combine(creoDir, ".."));
            string creoConfigPro = Path.Combine(creoRootFolder, "Common Files", "text", "config.pro");
            if (File.Exists(creoConfigPro) && !File.ReadAllText(creoConfigPro).Contains(nxlCreoPlugin))
            {
                Log("Add NextLabs Creo Plugin into config.pro");
                using (StreamWriter sw = File.AppendText(creoConfigPro))
                {
                    sw.Write(Environment.NewLine);
                    sw.Write(nxlCreoPlugin);
                }
            }
            else if (!File.Exists(creoConfigPro))
            {
                Log("Creo config.pro not exist in " + creoConfigPro);
                return ActionResult.Failure;
            }
            else
            {
                Log("NextLabs Creo Plugin already deployed");
            }

            Log("{0} NextLabs Custom Action - RepairCreoRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateCreoDirAction : RMXCustomAction
    {
        public ValidateCreoDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateCreoDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string creoDir = session["CREO_DIR"];
            Log("Validating following Creo Installation Directory: {0}", creoDir);

            string parametricExeFile = Path.Combine(creoDir, "bin", "parametric.exe");
            bool validPath = Directory.Exists(creoDir) && File.Exists(parametricExeFile);
            if (string.IsNullOrEmpty(session["CREO_DIR_VALID"]))
            {
                session["CREO_DIR_VALID"] = validPath ? "1" : "0";
            }
            else
            {
                session["CREO_DIR_VALID"] = null;
                session["CREO_DIR_VALID"] = validPath ? "1" : "0";
            }

            Log("After validation, CREO_DIR_VALID = {0}", session["CREO_DIR_VALID"]);
            Log("{0} NextLabs Custom Action - ValidateCreoDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindCreoDirAction : RMXCustomAction
    {
        public FindCreoDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindCreoDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Find both supported version of Creo (4.0 and 7.0). Pioritize 7.0 if find first
            string creoDir;
            string tmpCreo7Dir = FindCreo7();
            if (tmpCreo7Dir == string.Empty)
            {
                string tmpCreo4Dir = FindCreo4();
                if (tmpCreo4Dir == string.Empty)
                {
                    return ActionResult.Failure;
                }
                else
                {
                    creoDir = tmpCreo4Dir;
                    session["CREO_VER"] = "Creo 4.0";
                }
            }
            else {
                creoDir = tmpCreo7Dir;
                session["CREO_VER"] = "Creo 7.0";
            }
            
            session["CREO_DIR"] = creoDir;

            string creoRootFolder = Path.GetFullPath(Path.Combine(creoDir, ".."));
            string creoCommonDir = Path.Combine(creoRootFolder, "Common Files");
            session["CREO_COMMON_DIR"] = creoCommonDir;

            Log("Retrieved value for Creo Common Files Dir = {0}", session["CREO_COMMON_DIR"]);
            Log("Retrieved value InstallDir = {0}", session["CREO_DIR"]);

            Log("{0} NextLabs Custom Action - FindCreoDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }

        private string FindCreo4() {
            string creoParametricKey = "SOFTWARE\\Wow6432Node\\PTC\\PTC Creo Parametric\\4.0";

            RegistryKey key = Registry.LocalMachine.OpenSubKey(creoParametricKey);
            if (key == null) {
                return string.Empty;
            }

            string datecodeSubKey = key.GetSubKeyNames()[0];
            string creoParametricFullKey = string.Format("{0}\\{1}", creoParametricKey, datecodeSubKey);
            Log("Found full registry key in " + creoParametricFullKey);

            RegistryKey finalRegistry = Registry.LocalMachine.OpenSubKey(creoParametricFullKey);
            string creoDir = finalRegistry.GetValue("InstallDir").ToString();
            return creoDir;
        }

        private string FindCreo7() {
            string creoParametricKey = "SOFTWARE\\PTC\\PTC Creo Parametric\\";

            RegistryKey key = Registry.LocalMachine.OpenSubKey(creoParametricKey);
            if (key == null) {
                return string.Empty;
            }

            string versionSubKey = key.GetSubKeyNames()[0];
            string creoParametricFullKey = string.Format("{0}\\{1}", creoParametricKey, versionSubKey);
            Log("Found full registry key in " + creoParametricFullKey);

            RegistryKey finalRegistry = Registry.LocalMachine.OpenSubKey(creoParametricFullKey);
            string creoDir = finalRegistry.GetValue("InstallDir").ToString();
            string release = finalRegistry.GetValue("Release").ToString();
            if (release.StartsWith("7"))
            {
                return creoDir;
            }
            else {
                return string.Empty;
            }
        }
    }

    public class EditCreoTkDat : RMXCustomAction
    {
        public EditCreoTkDat(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            // After CreoRMX files is deployed to installFolder, creotk.dat file content is with CREO_RMX_ROOT prefix that need to change to full path
            Log("{0}{1} NextLabs Custom Action - EditCreoTkDat - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string creoDir = session.CustomActionData["CREO_DIR"];

            Log("Is Creo 4.0/creotk.dat file already exist : " + File.Exists(Path.Combine(installFolder, "Creo 4.0", "creotk.dat")));
            Log("At this point of execution: INSTALLFOLDER = " + installFolder);

            string creoTkDatFile;
            if (creoDir.Contains("Creo 7"))
            {
                creoTkDatFile = Path.Combine(installFolder, "Creo 7.0", "creotk.dat");
            }
            else
            {
                creoTkDatFile = Path.Combine(installFolder, "Creo 4.0", "creotk.dat");
            }
            
            string creoTkDatFileContent = File.ReadAllText(creoTkDatFile);
            creoTkDatFileContent = creoTkDatFileContent.Replace("$CREO_RMX_ROOT\\", installFolder);
            File.WriteAllText(creoTkDatFile, creoTkDatFileContent);
            Log("{0} NextLabs Custom Action - EditCreoTkDat - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindIpemDirAction : RMXCustomAction
    {
        public const string TC_IPEM_NAME = "Teamcenter Integration for Creo";
        public FindIpemDirAction(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindIpemDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string controlPanelProgram = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

            RegistryKey programList = Registry.LocalMachine.OpenSubKey(controlPanelProgram);
            foreach (string program in programList.GetSubKeyNames())
            {
                if (program.Contains(TC_IPEM_NAME))
                {
                    RegistryKey tcIpem = programList.OpenSubKey(program);
                    string tcIpemInstallLocation = tcIpem.GetValue("InstallLocation").ToString();
                    Log("Found Teamcenter Integration for Creo at : " + tcIpemInstallLocation);
                    session["IPEM"] = tcIpemInstallLocation;
                    // Set FOUND_IPEM to 1 to go through IPEM page
                    session["FOUND_IPEM"] = "1";
                    return ActionResult.Success;
                }
            }

            // In case not found Tc IPEM dir then set to any valid directory. And set FOUND_IPEM to 0 to skip UI page
            session["IPEM"] = "C:";
            session["FOUND_IPEM"] = "0";

            Log("{0} NextLabs Custom Action - FindIpemDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateIpemDirAction : RMXCustomAction
    {
        public ValidateIpemDirAction(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateIpemDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string ipemDir = session["IPEM"];
            Log("Validating following IPEM Installation Directory: {0}", ipemDir);

            string ipemJarFile = Path.Combine(ipemDir, "ipem.jar");
            bool validPath = (Directory.Exists(ipemDir) && File.Exists(ipemJarFile));
            session["IPEM_DIR_VALID"] = validPath ? "1" : "0";

            Log("After validation, IPEM_DIR_VALID = {0}", session["IPEM_DIR_VALID"]);
            Log("{0} NextLabs Custom Action - ValidateIpemDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

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
                case RMXCustomActionType.CA_ValidateCreoDir:
                    action = new ValidateCreoDirAction(session);
                    break;
                case RMXCustomActionType.CA_FindCreoDir:
                    action = new FindCreoDirAction(session);
                    break;
                case RMXCustomActionType.CA_EditCreoTkDat:
                    action = new EditCreoTkDat(session);
                    break;
                case RMXCustomActionType.CA_FindIpemDir:
                    action = new FindIpemDirAction(session);
                    break;
                case RMXCustomActionType.CA_ValidateIpemDir:
                    action = new ValidateIpemDirAction(session);
                    break;
                default:
                    action = null;
                    break;
            }
            return action;
        }
    }
}
