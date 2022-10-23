using System;
using System.Linq;
using System.IO;
using Microsoft.Win32;
using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public enum RMXCustomActionType
    {
        CA_Install, CA_Uninstall, CA_Repair, CA_ValidateACADDir, CA_FindACADDir, CA_EditACADrx, CA_FindACADIntegrationDir, CA_ValidateACADIntegrationDir
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
            Log("{0}{1} NextLabs Custom Action - InstallAutoCADRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string acadDir = session.CustomActionData["ACAD_DIR"];
            string acadIntegrationDir = session.CustomActionData["ACAD_INTEGRATION_DIR"];

            string acadRXFile = Path.Combine(acadDir, "Support", "acad.rx");
            string rmxARXFile = Path.Combine(installFolder, "AutoCAD 2022", "AutoCADRMX2022.arx");

            Log("At this point of installation, AutoCADRMX2022.arx file exist? : " + File.Exists(rmxARXFile));

            if (!File.Exists(acadRXFile)) {
                var file = File.Create(acadRXFile);
                file.Close();
            }
            Log("Adding NextLabs RMX For AutoCAD module to acad.rx");
            using (StreamWriter sw = File.AppendText(acadRXFile))
            {
                sw.Write(Environment.NewLine);
                sw.WriteLine(rmxARXFile);
            }
                
            Log("{0} NextLabs Custom Action - InstallAutoCADRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class UninstallAction : RMXCustomAction
    {
        public UninstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - UninstallAutoCADRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            RegistryKey acadDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\AutoCADRMX");
            string acadDir = acadDirRegistry.GetValue("AutoCADDir").ToString();
            string installDir = acadDirRegistry.GetValue("InstallDir").ToString();
            string acadIntegrationDir = acadDirRegistry.GetValue("AutoCADIntegrationDir").ToString();

            string acadRXFile = Path.Combine(acadDir, "Support", "acad.rx");
            string rmxARXFile = Path.Combine(installDir, "AutoCAD 2022", "AutoCADRMX2022.arx");

            File.WriteAllLines(acadRXFile, File.ReadAllLines(acadRXFile).Where(line => !(line.Contains("AutoCADRMX2022.arx") || string.IsNullOrEmpty(line))).ToList());

            Log("{0} NextLabs Custom Action - UninstallAutoCADRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class RepairAction : RMXCustomAction
    {
        public RepairAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - RepairAutoCADRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Find CreoDir and Plugin's InstallDir stored in registry to reinstall
            RegistryKey acadDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\AutoCADRMX");
            string acadDir = acadDirRegistry.GetValue("AutoCADDir").ToString();
            string installDir = acadDirRegistry.GetValue("InstallDir").ToString();
            string acadIntegrationDir = acadDirRegistry.GetValue("AutoCADIntegrationDir").ToString();
            
            Log("{0} NextLabs Custom Action - RepairAutoCADRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateACADDirAction : RMXCustomAction
    {
        public ValidateACADDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateACADDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string acadDir = session["ACAD_DIR"];
            Log("Validating following AutoCAD Installation Directory: {0}", acadDir);

            string acadExeFile = Path.Combine(acadDir, "acad.exe");
            bool validPath = Directory.Exists(acadDir) && File.Exists(acadExeFile);
            if (string.IsNullOrEmpty(session["ACAD_DIR_VALID"]))
            {
                session["ACAD_DIR_VALID"] = validPath ? "1" : "0";
            }
            else
            {
                session["ACAD_DIR_VALID"] = null;
                session["ACAD_DIR_VALID"] = validPath ? "1" : "0";
            }

            Log("After validation, ACAD_DIR_VALID = {0}", session["ACAD_DIR_VALID"]);
            Log("{0} NextLabs Custom Action - ValidateACADDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindACADDirAction : RMXCustomAction
    {
        public const string AUTOCAD_SOFTWARE_NAME = "AutoCAD 2022";
        public const string AUTOCAD_REGISTRY_CODE = "{28B89EEF-5101-0000-0102-CF3F3A09B77D}";
        public FindACADDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindACADDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            session["ACAD_DIR"] = "";
            Log("Preset ACAD_DIR to empty value");

            string controlPanelProgram = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

            // RegistryKey programList = Registry.LocalMachine.OpenSubKey(controlPanelProgram);
            RegistryKey programList = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64).OpenSubKey(controlPanelProgram);
            
            Log("[{0}]", string.Join("\n ", programList.GetSubKeyNames()));
            if (programList.GetSubKeyNames().Contains(AUTOCAD_REGISTRY_CODE)) {
                RegistryKey finalRegistry = programList.OpenSubKey(AUTOCAD_REGISTRY_CODE);

                if (finalRegistry.GetValue("DisplayName").ToString().StartsWith(AUTOCAD_SOFTWARE_NAME))
                {
                    string acadInstallLocation = finalRegistry.GetValue("InstallLocation").ToString();
                    Log("Found AutoCAD 2022 at : " + acadInstallLocation);
                    session["ACAD_DIR"] = acadInstallLocation;
                }
            }
            /*
            foreach (string programId in programList.GetSubKeyNames())
            {
                Log("Checking Control Panel Program: " + programId);
                using (RegistryKey subkey = programList.OpenSubKey(programId)) {
                    if (subkey.GetValue("DisplayName").ToString().StartsWith(AUTOCAD_SOFTWARE_NAME)) {
                        string acadInstallLocation = subkey.GetValue("InstallLocation").ToString();
                        Log("Found AutoCAD 2022 at : " + acadInstallLocation);
                        session["ACAD_DIR"] = acadInstallLocation;
                        break;
                    }
                }
            }
            */

            Log("{0} NextLabs Custom Action - FindACADDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class EditACADrx : RMXCustomAction
    {
        public EditACADrx(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            // After AutoCADRMX files is deployed to installFolder, acad.rx file content is with CREO_RMX_ROOT prefix that need to change to full path
            Log("{0}{1} NextLabs Custom Action - EditACADrx - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string acadDir = session.CustomActionData["ACAD_DIR"];

            Log("{0} NextLabs Custom Action - EditACADrx - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindACADIntegrationDirAction : RMXCustomAction
    {
        public FindACADIntegrationDirAction(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindACADIntegrationDirAction - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Not support yet. Let FOUND_ACAD_INTEGRATION be 0 for now
            session["ACAD_INTEGRATION_DIR"] = "C:";
            session["FOUND_ACAD_INTEGRATION"] = "0";

            Log("{0} NextLabs Custom Action - FindACADIntegrationDirAction - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateACADIntegrationDirAction : RMXCustomAction
    {
        public ValidateACADIntegrationDirAction(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateACADIntegrationDirAction - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            session["ACAD_INTEGRATION_DIR_VALID"] = "0";

            Log("{0} NextLabs Custom Action - ValidateACADIntegrationDirAction - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

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
                case RMXCustomActionType.CA_ValidateACADDir:
                    action = new ValidateACADDirAction(session);
                    break;
                case RMXCustomActionType.CA_FindACADDir:
                    action = new FindACADDirAction(session);
                    break;
                case RMXCustomActionType.CA_EditACADrx:
                    action = new EditACADrx(session);
                    break;
                case RMXCustomActionType.CA_FindACADIntegrationDir:
                    action = new FindACADIntegrationDirAction(session);
                    break;
                case RMXCustomActionType.CA_ValidateACADIntegrationDir:
                    action = new ValidateACADIntegrationDirAction(session);
                    break;
                default:
                    action = null;
                    break;
            }
            return action;
        }
    }
}
