using Microsoft.Deployment.WindowsInstaller;

namespace RMXInstall
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult InstallSldWorksRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Install, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult UninstallSldWorksRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Uninstall, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult RepairSldWorksRMX(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_Repair, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateSldWorksDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateSldWorksDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindSldWorksDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindSldWorksDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult ValidateSwimDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_ValidateSwimDir, session);
            return action.Execute();
        }

        [CustomAction]
        public static ActionResult FindSwimDir(Session session)
        {
            RMXCustomAction action = RMXCustomActionFactory.CreateCustomAction(RMXCustomActionType.CA_FindSwimDir, session);
            return action.Execute();
        }
    }
}
