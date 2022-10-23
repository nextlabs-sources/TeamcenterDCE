using System;
using System.Linq;
using System.IO;
using Microsoft.Win32;
using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public enum RMXCustomActionType
    {
        CA_Install, CA_Uninstall, CA_Repair, CA_ValidateCatiaDir, CA_FindCatiaDir, CA_FindCatiaIntegrationDir, CA_ValidateCatiaIntegrationDir
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
            Log("{0}{1} NextLabs Custom Action - InstallCatiaRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);
            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string catiaDir = session.CustomActionData["CATIA_DIR"];
            string catiaExe = Path.Combine(catiaDir, "code", "bin", "CNEXT.exe");
            string rmxDll = Path.Combine(installFolder, "CatiaRMX_x64.dll");

            RegistryKey skydrmKey = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\SkyDRM", true);
            skydrmKey.CreateSubKey("3rdRMX");

            Log("Adding CNEXT.exe subkey in SkyDRM registry ");
            RegistryKey rmx = skydrmKey.OpenSubKey("3rdRMX", true);
            rmx.CreateSubKey("CNEXT.exe");

            Log("Adding values for CATIA RMX: path and rmx");
            RegistryKey catiaRMX = rmx.OpenSubKey("CNEXT.exe", true);
            catiaRMX.SetValue("path", catiaExe);
            catiaRMX.SetValue("rmx", rmxDll);
                
            Log("{0} NextLabs Custom Action - InstallCatiaRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class UninstallAction : RMXCustomAction
    {
        public UninstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - UninstallCatiaRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            RegistryKey catiaDirRegistry = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\CatiaRMX");
            string catiaDir = catiaDirRegistry.GetValue("CatiaDir").ToString();
            string installDir = catiaDirRegistry.GetValue("InstallDir").ToString();
            string catiaIntegrationDir = catiaDirRegistry.GetValue("CatiaIntegrationDir").ToString();

            Log("Remove CNEXT.exe subkey from SkyDRM Registry");
            Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\SkyDRM\\3rdRMX", true).DeleteSubKey("CNEXT.exe");

            Log("{0} NextLabs Custom Action - UninstallCatiaRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class RepairAction : RMXCustomAction
    {
        public RepairAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - RepairCatiaRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Reinstall operation
            string installFolder = session.CustomActionData["INSTALLFOLDER"];
            string catiaDir = session.CustomActionData["CATIA_DIR"];
            string catiaExe = Path.Combine(catiaDir, "code", "bin", "CNEXT.exe");
            string rmxDll = Path.Combine(installFolder, "CatiaRMX_x64.dll");

            RegistryKey skydrmKey = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NextLabs\\SkyDRM", true);
            skydrmKey.CreateSubKey("3rdRMX");

            RegistryKey rmx = skydrmKey.OpenSubKey("3rdRMX", true);
            rmx.CreateSubKey("CNEXT.exe");

            RegistryKey catiaRMX = rmx.OpenSubKey("CNEXT.exe", true);
            catiaRMX.SetValue("path", catiaExe);
            catiaRMX.SetValue("rmx", rmxDll);

            Log("{0} NextLabs Custom Action - RepairCatiaRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateCatiaDirAction : RMXCustomAction
    {
        public ValidateCatiaDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateCatiaDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string catiaDir = session["CATIA_DIR"];
            Log("Validating following CATIA Installation Directory: {0}", catiaDir);

            string catiaExe = Path.Combine(catiaDir, "code", "bin", "CNEXT.exe");
            bool validPath = Directory.Exists(catiaDir) && File.Exists(catiaExe);
            if (string.IsNullOrEmpty(session["CATIA_DIR_VALID"]))
            {
                session["CATIA_DIR_VALID"] = validPath ? "1" : "0";
            }
            else
            {
                session["CATIA_DIR_VALID"] = null;
                session["CATIA_DIR_VALID"] = validPath ? "1" : "0";
            }

            Log("After validation, CATIA_DIR_VALID = {0}", session["CATIA_DIR_VALID"]);
            Log("{0} NextLabs Custom Action - ValidateCatiaDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindCatiaDirAction : RMXCustomAction
    {
        public FindCatiaDirAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindCatiaDir - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            session["CATIA_DIR"] = "";
            Log("Preset CATIA_DIR to empty value");

            string catiaAppExeRegistry = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CNEXT.exe";
            RegistryKey key = Registry.LocalMachine.OpenSubKey(catiaAppExeRegistry);

            string catiaAppExe = key.GetValue(String.Empty).ToString();
            if (catiaAppExe.EndsWith("CNEXT.exe")) {
                Log("Found (Registry) Catia EXE at : {0}", catiaAppExe);
                string catiaDir = Path.GetFullPath(Path.Combine(Directory.GetParent(catiaAppExe).ToString(), "..\\.."));
                Log("Retrieve Catia Installaton Dir At: {0}", catiaDir);
                session["CATIA_DIR"] = catiaDir;
            }

            Log("{0} NextLabs Custom Action - FindCatiaDir - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class FindCatiaIntegrationDirAction : RMXCustomAction
    {
        public FindCatiaIntegrationDirAction(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - FindCatiaIntegrationDirAction - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            // Not support yet. Let FOUND_CATIA_INTEGRATION be 0 for now
            session["CATIA_INTEGRATION_DIR"] = "C:";
            session["FOUND_CATIA_INTEGRATION"] = "0";

            Log("{0} NextLabs Custom Action - FindCatiaIntegrationDirAction - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class ValidateCatiaIntegrationDirAction : RMXCustomAction
    {
        public ValidateCatiaIntegrationDirAction(Session session) : base(session) { }

        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - ValidateCatiaIntegrationDirAction - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            session["CATIA_INTEGRATION_DIR_VALID"] = "0";

            Log("{0} NextLabs Custom Action - ValidateCatiaIntegrationDirAction - FINISHED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

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
                case RMXCustomActionType.CA_ValidateCatiaDir:
                    action = new ValidateCatiaDirAction(session);
                    break;
                case RMXCustomActionType.CA_FindCatiaDir:
                    action = new FindCatiaDirAction(session);
                    break;
                case RMXCustomActionType.CA_FindCatiaIntegrationDir:
                    action = new FindCatiaIntegrationDirAction(session);
                    break;
                case RMXCustomActionType.CA_ValidateCatiaIntegrationDir:
                    action = new ValidateCatiaIntegrationDirAction(session);
                    break;
                default:
                    action = null;
                    break;
            }
            return action;
        }
    }
}
