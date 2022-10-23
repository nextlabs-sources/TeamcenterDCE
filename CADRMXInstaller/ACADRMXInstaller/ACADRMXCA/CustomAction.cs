using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult InstallACADRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Install, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult UninstallACADRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Uninstall, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult RepairACADRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Repair, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateACADDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateACADDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindACADDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindACADDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult EditACADrx(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_EditACADrx, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindACADIntegrationDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindACADIntegrationDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateACADIntegrationDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateACADIntegrationDir, session);
            return action.Execute();
        }
    }
}
