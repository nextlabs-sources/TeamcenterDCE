using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult InstallNXRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Install, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult UninstallNXRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Uninstall, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult RepairNXRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Repair, session);
            return action.Execute();
        }
    }
}
