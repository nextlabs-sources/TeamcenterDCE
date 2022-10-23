using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult InstallCreoRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Install, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult UninstallCreoRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Uninstall, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult RepairCreoRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Repair, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateCreoDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateCreoDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindCreoDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindCreoDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult EditCreoTkDat(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_EditCreoTkDat, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindIpemDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindIpemDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateIpemDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateIpemDir, session);
            return action.Execute();
        }
    }
}
