using System;
using System.Linq;
using System.IO;
using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public enum RMXCustomActionType
    {
        CA_Install, CA_Uninstall, CA_Repair
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
            Log("{0}{1} NextLabs Custom Action - InstallNXRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string ugiiRoot = Environment.GetEnvironmentVariable("UGII_ROOT_DIR");
            if (ugiiRoot == null)
            {
                ugiiRoot = Path.Combine(Environment.GetEnvironmentVariable("UGII_BASE_DIR"), "UGII");
            }

            string customDirFile = Path.Combine(ugiiRoot, "menus", "custom_dirs.dat");
            if (!File.Exists(customDirFile))
            {
                Log("File custom_dirs.dat not exist. Installation abort.");
                return ActionResult.Failure;
            }

            string userProfileFolder = Environment.GetEnvironmentVariable("USERPROFILE");
            string ugiiTmpDir = Path.Combine(userProfileFolder, "AppData", "Local", "Temp", "nx_rpm");
            Directory.CreateDirectory(ugiiTmpDir);

            Log("Remove read-only attribute of custom_dirs file");
            RemoveFileReadOnlyAttribute(customDirFile);

            if (!File.Exists(Path.Combine(ugiiRoot, "menus", "custom_dirs.dat.old")))
            {
                Log("Backup custom_dirs.dat file");
                File.Copy(customDirFile, Path.Combine(ugiiRoot, "menus", "custom_dirs.dat.old"));
            }

            // Edit custom_dirs.dat
            string nxRMXFolder = session.CustomActionData["NXRMXFolder"];
            if (nxRMXFolder.EndsWith("\\"))
            {
                nxRMXFolder = nxRMXFolder.Substring(0, nxRMXFolder.Length - 1);
            }

            if (File.Exists(customDirFile) && !File.ReadAllText(customDirFile).Contains(nxRMXFolder))
            {
                Log("Adding NextLabs RMX For NX module to custom_dirs.dat");
                using (StreamWriter sw = File.AppendText(customDirFile))
                {
                    sw.Write(Environment.NewLine);
                    sw.WriteLine("# nextlabs NXIntegration - start");
                    sw.WriteLine(nxRMXFolder);
                    sw.WriteLine("# nextlabs NXIntegration - end");
                }
            }
            else
            {
                Log("NextLabs RMX For NX is already deployed");
            }
            Log("{0} NextLabs Custom Action - InstallNXRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

            return ActionResult.Success;
        }
    }

    public class UninstallAction : RMXCustomAction
    {
        public UninstallAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - UninstallNXRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string ugiiRoot = Environment.GetEnvironmentVariable("UGII_ROOT_DIR");
            if (ugiiRoot == null)
            {
                ugiiRoot = Path.Combine(Environment.GetEnvironmentVariable("UGII_BASE_DIR"), "UGII");
            }

            Log("Remove NextLabs RMX For NX module in custom_dirs file");
            string customDirFile = Path.Combine(ugiiRoot, "menus", "custom_dirs.dat");
            if (File.Exists(customDirFile))
            {
                File.WriteAllLines(customDirFile, File.ReadAllLines(customDirFile).Where(line => !(line.Contains("NXIntegration") || string.IsNullOrEmpty(line))).ToList());
            }

            Log("{0} NextLabs Custom Action - UninstallNXRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);
            return ActionResult.Success;
        }
    }

    public class RepairAction : RMXCustomAction
    {
        public RepairAction(Session session) : base(session) { }
        public override ActionResult Execute()
        {
            Log("{0}{1} NextLabs Custom Action - RepairNXRMX - STARTED {2}", Environment.NewLine, NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR);

            string ugiiRoot = Environment.GetEnvironmentVariable("UGII_ROOT_DIR");
            if (ugiiRoot == null)
            {
                ugiiRoot = Path.Combine(Environment.GetEnvironmentVariable("UGII_BASE_DIR"), "UGII");
            }

            string customDirFile = Path.Combine(ugiiRoot, "menus", "custom_dirs.dat");
            if (!File.Exists(customDirFile))
            {
                Log("File custom_dirs.dat not exist. Installation abort.");
                return ActionResult.Failure;
            }

            Log("Remove read-only attribute of custom_dirs file");
            RemoveFileReadOnlyAttribute(customDirFile);

            if (!File.Exists(Path.Combine(ugiiRoot, "menus", "custom_dirs.dat.old")))
            {
                Log("Backup custom_dirs.dat file");
                File.Copy(customDirFile, Path.Combine(ugiiRoot, "menus", "custom_dirs.dat.old"));
            }

            // Edit custom_dirs.dat
            string nxRMXFolder = session.CustomActionData["NXRMXFolder"];
            if (nxRMXFolder.EndsWith("\\"))
            {
                nxRMXFolder = nxRMXFolder.Substring(0, nxRMXFolder.Length - 1);
            }

            if (File.Exists(customDirFile) && !File.ReadAllText(customDirFile).Contains(nxRMXFolder))
            {
                Log("Adding NextLabs RMX For NX module to custom_dirs.dat");
                using (StreamWriter sw = File.AppendText(customDirFile))
                {
                    sw.Write(Environment.NewLine);
                    sw.WriteLine("# nextlabs NXIntegration - start");
                    sw.WriteLine(nxRMXFolder);
                    sw.WriteLine("# nextlabs NXIntegration - end");
                }
            }
            else
            {
                Log("NextLabs RMX For NX is already deployed");
            }
            Log("{0} NextLabs Custom Action - RepairNXRMX - STOPPED {1}{2}", NEXTLABS_CA_SEPARATOR, NEXTLABS_CA_SEPARATOR, Environment.NewLine);

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
                default:
                    action = null;
                    break;
            }
            return action;
        }
    }
}
