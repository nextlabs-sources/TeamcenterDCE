/*=============================================================================
                 Copyright (c) 2004 UGS Corp.
                   Unpublished - All rights reserved

   THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY THE UNITED STATES
  COPYRIGHT LAWS AND IS CONSIDERED A TRADE SECRET BELONGING TO THE COPYRIGHT
                                   HOLDER.
===============================================================================
File description:

    File   : NxlAutoProtect_register.c
    Module : Register NxlAutoProtect.dll

===============================================================================

$HISTORY$
Date            Name				Description of Change 
2014-05-27      Chenwei Low         Initial Version/POC
2015-05-27		Gavin Chen			Version 1.5
=============================================================================*/
//#include <tc/iman.h>
#include <stdarg.h> //va_list() va_start() va_end()
#include <tccore/custom.h> //CUSTOM_register_exit()
#include <tc/emh.h>
#include "dce_mem.h"
#include "tc_utils.h"
#include "nxl_auto_protect.h"
#include <nxl_action_handler.h>
#include <utils_system.h>
#include <utils_string.h>

#include "dce_log.h" //DEC log

/* Definitions */
#define CUSTOM_EXIT_MODULE ( "USER_init_module" ) //Custom Exit module

#define RET_STR(rc) (rc!=ITK_ok ? "Failed" : "Success" )

DCEAPI int NxlAutoProtect_register_handlers(int *, va_list);

#define REGISTER_EXIT(exit, handler) \
{	\
	if(ITK_EVAL(CUSTOM_register_exit(PROJECT_NAME, exit, (CUSTOM_EXIT_ftn_t) handler)))	\
	{	\
		SYSLOG("Registered '%s' Success", exit);	\
	} else {	\
		SYSLOG("Registered '%s' Failed", exit);	\
	}	\
}
void init_nxlautoprotect_log()
{
	//initial log file
	char logFile[MAX_PATH];
	if(GetTempPath(MAX_PATH, logFile) > 0)
	{
		char logName[100] = "";
#if defined(WNT)
		sprintf_s(logName, 100, PROJECT_NAME ".%lu.log", GetCurrentProcessId());
#elif defined(__linux__)
		sprintf_s(logName, 100, PROJECT_NAME ".%d.log", getpid());
#endif
		strcat_s(logFile, MAX_PATH, logName);
		if(!initial_log(logFile))
		{
			ERRLOG("Create LogFile Failed('%s')", logFile);
		}
	}
	else
	{
		ERRLOG("GetTempPath Failed:%lu", GetLastError());
	}
}
void env_debug()
{
	//output environment information
	char *buf = NULL;
	int id;
	if(ITK_EVAL(POM_ask_server_info(&id, &buf)))
	{
		NEED_FREE(buf);
		DBGLOG("Server='%s' NodeType=%d", buf, id);
		TC_FREE(buf);
	}
	DBGLOG("localhost='%s'", host_get_name());
	DBGLOG("4-tier=%d", ITK__ask_managed_mode());
	DBGLOG("TC_ROOT=%s", getenv("TC_ROOT"));
	DBGLOG(ENV_PRODUCT_ROOT"=%s", getenv(ENV_PRODUCT_ROOT));
	DBGLOG("NxlLogFile='%s'", log_get_file());
	//teamcenter syslog file name
	DBGLOG("SysFile='%s'", EMH_ask_system_log());
	DBGLOG("Teamcenter lib version=" TC_LIB_VER);
	//logon user
	if(ITK_EVAL(POM_get_user_id(&buf)))
	{
		NEED_FREE(buf);
		DBGLOG("LogonUser='%s'", buf);
		TC_FREE(buf);
	}
}
/*************************************************************/
/* register the NxlAutoProtect callbacks					 */
/*      Method name must be {LIBRARY_NAME}_register_callbacks*/
/*                                                           */
/* Returns: ITK_ok - success                                 */
/*          !ITK_ok - otherwise                              */
/*************************************************************/ 
DCEAPI int NxlAutoProtect_register_callbacks()
{
	solution_info solution;
	init_nxlautoprotect_log();
	TC_HANDLER_START;
	SYSLOG("=====Nextlabs Auto Protect(" TARGETFILENAME ") - Version: " FILE_VER "=====");
	if((solution = get_solution_info())!=NULL)
	{
		//The callback for register event handler is registered for all features
		REGISTER_EXIT(CUSTOM_EXIT_MODULE, NxlAutoProtect_register_handlers);

		//Note:In order to fix the TextDataset issue
		//we need the dlp hook to set the classification code to allow FCC client to rectify the text content
		//therefore the dlp hook is required for both inMotion and AtRest
		if(solution->components & dlp_exit_comp)
		{
			REGISTER_EXIT(DLP_EXIT_NAME, nxl_add_classification_code);
		}
	}
	else
	{
		ERRLOG("No Nextlabs Feature is installed or enabled");
	}
	
	TC_HANDLER_END;
	return ITK_ok;
}


/*************************************************************/
/* register the NxlAutoProtect specific handlers             */
/*                                                           */
/* Params: decision - customization execution status         */
/*         args - arguments                                  */
/*                                                           */
/* Returns: ITK_ok - success                                 */
/*          !ITK_ok - otherwise                              */
/*************************************************************/
DCEAPI int NxlAutoProtect_register_handlers(int *decision, va_list args)
{
	int rc = ITK_ok;
	solution_info solution;
	
	TC_HANDLER_START;
	*decision = ALL_CUSTOMIZATIONS;
	UNREFERENCED_PARAMETER(args);

	env_debug();
	if((solution = get_solution_info())!=NULL)
	{
		if(solution->components & wf_handler_comp)
		{			  
			ITK_EVAL(rc = EPM_register_action_handler(ACTION_HANDLER_NAME, ACTION_HANDLER_DESC, nxl_auto_protect_handler));
				
			SYSLOG("Registered '%s' %s", ACTION_HANDLER_NAME, RET_STR(rc));
		}
		if(solution->components & checkin_handler_comp)
		{
			REGISTER_EVENT_HANDLER(RES_Type, NULL, RES_cancel_checkout_msg, METHOD_post_action_type, nxl_checkout_handler);
			REGISTER_EVENT_HANDLER(RES_Type, NULL, RES_checkout_msg, METHOD_post_action_type, nxl_checkout_handler);
			REGISTER_EVENT_HANDLER(RES_Type, NULL, RES_checkin_msg, METHOD_pre_action_type, nxl_checkin_handler);
		}
		if(solution->components & set_tool_comp)
		{
			REGISTER_EVENT_HANDLER("Dataset", NULL, AE_save_dataset_msg, METHOD_post_action_type, Nxl3_dataset_save_post);
			REGISTER_EVENT_HANDLER("Dataset", NULL, AE_save_myself_dataset_msg, METHOD_post_action_type, Nxl3_dataset_save_post);
		}
		metadatasync_to_dataset_on_import();
		metadatasync_on_license_changed();
		refresh_duid_install();
	}
	//CALL_DBG(register_event_handlers());

	/*TODO/TBD: should the JVM be created in main thread?
	{
		JNIEnv *env;
		if(JNI_init(&env) == JNI_OK)
		{
			SYSLOG("JNI environment is initialized!");
		}
		else
		{
			ERRLOG("Initialize JNI Environment failed!");
		}
	}*/
	
	TC_HANDLER_END;
	return ITK_ok;
}