#include "hook_log.h"
#include "hook_utils.h"
#include "hook_defs.h"
#include <typeinfo>

char logbuf[LOG_MAX];
wchar_t logbufw[LOG_MAX];
wchar_t func[100];
size_t funcSize;

BOOL hook_init()
{
	InitializeMadCHook();
	DBGLOG("MadCHook initialized");
	SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);
	return TRUE;
}
void hook_final()
{
	FinalizeMadCHook();
	DBGLOG("MadCHook Finalized");
}

#define PROCESS_NAME_VISVIEW "visview.exe"
extern BOOL visview_install();
extern void visview_uninstall();

#define PROCESS_NAME_IGES "iges.exe"
extern BOOL iges_install();
extern void iges_uninstall();

#define PROCESS_NAME_TCSERVER "tcserver.exe"
#define PROCESS_NAME_SRMUTIL "srm_util.exe"
extern BOOL tcserver_install();
extern void tcserver_uninstall();

#define PROCESS_NAME_BRIEFCASEBROWSER "BriefcaseBrowser.exe"
extern BOOL briefcasebrowser_install();
extern void briefcasebrowser_uninstall();

BOOL hook_install(const char *processName)
{
	if(StringEndsWithA(processName, PROCESS_NAME_TCSERVER, FALSE) || StringEndsWithA(processName, PROCESS_NAME_SRMUTIL, FALSE))
		return hook_init() && tcserver_install();

	/*if(StringEndsWithA(processName, PROCESS_NAME_VISVIEW, FALSE))
	{
		return hook_init() && visview_install();
	}
	else if(StringEndsWithA(processName, PROCESS_NAME_IGES, FALSE))
	{
		return hook_init() && iges_install();
	}
	else if(StringEndsWithA(processName, PROCESS_NAME_TCSERVER, FALSE) || StringEndsWithA(processName, PROCESS_NAME_SRMUTIL, FALSE))
	{
		return hook_init() && tcserver_install();
	}
	else if(StringEndsWithA(processName, PROCESS_NAME_BRIEFCASEBROWSER, FALSE))
	{
		return hook_init() && briefcasebrowser_install();
	}*/
	return FALSE;
}

void hook_uninstall(const char *processName)
{
	if(StringEndsWithA(processName, PROCESS_NAME_TCSERVER, FALSE) || StringEndsWithA(processName, PROCESS_NAME_SRMUTIL, FALSE))
	{
		tcserver_uninstall();
		hook_final();
	}

	/*if(StringEndsWithA(processName, PROCESS_NAME_VISVIEW, FALSE))
	{
		visview_uninstall();
		hook_final();
	}
	else if(StringEndsWithA(processName, PROCESS_NAME_IGES, FALSE))
	{
		iges_uninstall();
		hook_final();
	}
	else if(StringEndsWithA(processName, PROCESS_NAME_TCSERVER, FALSE) || StringEndsWithA(processName, PROCESS_NAME_SRMUTIL, FALSE))
	{
		tcserver_uninstall();
		hook_final();
	}
	else if(StringEndsWithA(processName, PROCESS_NAME_BRIEFCASEBROWSER, FALSE))
	{
		briefcasebrowser_uninstall();
		hook_final();
	}*/
}
BOOL WINAPI DllMain( IN HINSTANCE hDllHandle,
         IN DWORD     nReason,
         IN LPVOID    Reserved )
{
	CHAR buff[MAX_PATH];
	GetModuleFileName(NULL, buff, MAX_PATH);
	switch (nReason)
	{
		case DLL_PROCESS_ATTACH:
			//init_nxl_scf_hook_log();
			DBGLOG("[%s]+++++++++++++++++DLL_PROCESS_ATTACH+++++++++++++", buff);
			return hook_install(buff);
		case DLL_THREAD_ATTACH:
			//DBGLOG("<<<<<<<<<<<<<<<DLL_THREAD_ATTACH...");
			//nx_hook_delayed();
			break;
		case DLL_THREAD_DETACH:
			//DBGLOG("DLL_THREAD_DETACH>>>>>>>>>>>>>>>>>>");
			break;
		case DLL_PROCESS_DETACH:
			hook_uninstall(buff);
			DBGLOG("[%s]--------------DLL_PROCESS_DETACH------------------", buff);
			break;
	}
	return TRUE;
}
