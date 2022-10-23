using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult InstallCatiaRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Install, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult UninstallCatiaRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Uninstall, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult RepairCatiaRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Repair, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateCatiaDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateCatiaDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindCatiaDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindCatiaDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindCatiaIntegrationDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindCatiaIntegrationDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateCatiaIntegrationDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateCatiaIntegrationDir, session);
            return action.Execute();
        }
    }
}
