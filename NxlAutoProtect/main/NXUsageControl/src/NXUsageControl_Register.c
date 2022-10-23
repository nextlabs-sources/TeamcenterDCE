#include <utils_windows.h>
#include <Shlwapi.h>
#include "nx_utils.h"
#include "nxuc_app.h"

char sMsgBuf[1024];
/*
	Action Registry
	TODO: add the Actions which will be used in NX menuScript into this list
*/
static UF_MB_action_t actionTable[] =
{
 { APP_ACTION_ABOUT, action_about, NULL },
 { APP_ACTION_EVALUATE_WORKPART, action_evaluate_workpart_only, NULL },
 { APP_ACTION_EVALUATE_ALL, action_evaluate_all_parts, NULL },
 { "nxl_edit_workpart", action_evaluate_edit_workpart, NULL },
 { "nxl_edit_assembly", action_evaluate_edit_assembly, NULL },
 { "nxl_print_assembly", action_evaluate_print_assembly, NULL },
 { "nxl_saveas_workpart", action_evaluate_saveas_workpart, NULL },
 { "nxl_saveas_assembly", action_evaluate_saveas_assembly, NULL },
 { "nxl_clipboard_workpart", action_evaluate_clipboard_workpart, NULL },
 //{ APP_ACTION_LIST_COMPS, action_list_comps, NULL },
 { NULL, NULL, NULL }	//action table must end with this item
};
//NX application ID
static int app_id = 0;

/*----------------------------------------------------------------------------*
 *  NXUC_APP_enter
 *
 *      Enter the application
 *----------------------------------------------------------------------------*/
static void NXUC_APP_enter( void)
{
	NXDBG("=========================================");

    /* Place code to enter application here */
	NXDBG("Ends here..." );
}

/*----------------------------------------------------------------------------*
 *  NXUC_APP_init
 *
 *      Initialize the application
 *----------------------------------------------------------------------------*/
static void NXUC_APP_init( void )
{
	NXDBG("=========================================");

    /* Place code to enter application here */

	NXDBG("Ends here..." );
}

/*----------------------------------------------------------------------------*
 *  NXUC_APP_exit
 *
 *      Exit the application
 *----------------------------------------------------------------------------*/
static void NXUC_APP_exit( void )
{
	NXDBG("=========================================");

    /* Place code to enter application here */
	NXDBG("Ends here..." );
}
#define NXL_UTILS_DLL "NxlUtils64.dll"
BOOL load_nxlutils()
{
	HMODULE module;
	//check if loaded
	if((module = GetModuleHandle(NXL_UTILS_DLL)) == NULL)
	{
		//searching NxlUtils64.dll from same folder of NXIntegration
		if((module = GetModuleHandle(TARGETFILENAME)) != NULL)
		{
			char nxiPath[MAX_PATH];
			if(GetModuleFileName(module, nxiPath, MAX_PATH) > 0)
			{
				PathRemoveFileSpec(nxiPath);
				SetDllDirectory(nxiPath);
				NXONLY("SetDllDirectory('%s')", nxiPath);
			}
			else
			{
				NXONLY("Failed to GetModuleFileName("TARGETFILENAME")!(ErrorCode:%#X)", GetLastError());
			}
		}
		module = LoadLibrary(NXL_UTILS_DLL);
		SetDllDirectory(NULL);
	}
	//ask NX to search for us
	if(module != NULL)
	{
		char nxlutils[MAX_PATH];
		if(GetModuleFileName(module, nxlutils, MAX_PATH) > 0)
		{
			NXSYS(NXL_UTILS_DLL" is loaded(Path='%s')", nxlutils);
		}
		else
		{
			NXERR(NXL_UTILS_DLL" is loaded(GetModuleFileName Error=%lu)", GetLastError());
		}
		return TRUE;
	}
	else
	{
		NXONLY("LoadLibrary '"NXL_UTILS_DLL"' Failed(Error=%lu)", GetLastError());
	}
	return FALSE;
}
/*****************************************************************************
**  Activation Methods
*****************************************************************************/
/*  Unigraphics Startup
**      This entry point activates the application at Unigraphics startup 
*/
void ufsta( char *param, int *returnCode, int rlen )
{
	//initial log file
	char logFile[MAX_PATH];
	load_nxlutils();
	if(GetTempPath(MAX_PATH, logFile) > 0)
	{
		char logName[100];
		sprintf_s(logName, 100, PROJECT_NAME".%lu.log", GetCurrentProcessId());
		strcat_s(logFile, MAX_PATH, logName);
		if(initial_log(logFile))
		{
			NXSYS("LogFile='%s'", logFile);
		}
		else
		{
			NXERR("Create LogFile Failed('%s')", logFile);
		}
	}
	else
	{
		NXERR("GetTempPath Failed:%lu", GetLastError());
	}
    /* Initialize the API environment */
    if( NX_EVAL(UF_initialize()) ) 
    {
		UF_MB_application_t appData;
		char name[] = APP_NAME;

		//register actions
		NX_CALL(UF_MB_add_actions( actionTable ));

		//register NXL App
		/* Initialize application data */
		
		appData.name       = name;
		appData.id         = 0;
		appData.init_proc  = NXUC_APP_init;
		appData.exit_proc  = NXUC_APP_exit;
		appData.enter_proc = NXUC_APP_enter;
		
		/* Initialize application support flags */

		appData.drawings_supported          = TRUE;
		appData.design_in_context_supported = TRUE;
		
		/* Register the application with UG */
		
		NX_CALL(UF_MB_register_application( &appData ));
		
		app_id = appData.id;
		NXDBG("App ID=%d", app_id);
		
		/*Hook NX api*/
		//fcc_hook();
		//libpart_hook();
		//libugmr_hook();
		/* Terminate the API environment */
		NX_CALL(UF_terminate());
        return;
    }
	/* Failed to initialize */
}