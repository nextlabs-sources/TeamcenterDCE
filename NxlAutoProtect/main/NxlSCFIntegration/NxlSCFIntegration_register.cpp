#include <NxlSCFIntegration.h>
#include <dce_log.h>
#include <dce_mem.h>
#include <tc_utils.h>
#include <tccore/custom.h>
#include <stdarg.h>
#include <tie/tie.h>
#include <ae/dataset_msg.h>
#include <nxl_action_handler.h>

#define REGISTER_EXIT(exit, handler) \
{	\
	if(ITK_EVAL(CUSTOM_register_exit(PROJECT_NAME, exit, (CUSTOM_EXIT_ftn_t) handler)))	\
	{	\
		SYSLOG("Registered '%s' Success", exit);	\
	} else {	\
		SYSLOG("Registered '%s' Failed", exit);	\
	}	\
}
int NxlSCFIntegration_register_handlers(int *decision, va_list args)
{
	int rc = ITK_ok;
	solution_info solution;

	TC_HANDLER_START;
	*decision = ALL_CUSTOMIZATIONS;
	UNREFERENCED_PARAMETER(args);
	
	REGISTER_EVENT_HANDLER("Briefcase", NULL, AE_save_dataset_msg, METHOD_post_action_type, Nxl3_bcz_modifier);

	TC_HANDLER_END;
	return ITK_ok;
}

int NxlSCFIntegration_register_callbacks()
{
	TC_HANDLER_START;
	SYSLOG("=====Nextlabs SCF Integration(" TARGETFILENAME ") - Version: " FILE_VER "=====");

	//inistall pdm server for integration in tcin_import.exe
	REGISTER_EXIT("USER_invoke_pdm_server", NXRMX_invoke_pdm_server);
	//The callback for register event handler is registered for all features
	REGISTER_EXIT("USER_init_module", NxlSCFIntegration_register_handlers);

	TC_HANDLER_END;
	return ITK_ok;
}