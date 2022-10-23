#include <utils_windows.h>
#include <Shlwapi.h>
#include "nx_utils.h"
#include "nxl_app.h"
#include <NXRMX/nx_hook.h>
#include <NXRMX/NXUsageControl.h>

#include <NxlUtils/NxlPath.hpp>
#include <NxlUtils/NxlDllLoader.hpp>

extern bool install_rmxui_items();
extern bool install_secure_overlay();

class NxDllLoader :public NxlDllLoader {
public:
	void OutputMessage(const std::wstring &msg, DWORD code = 0) override {
		auto utf = wstring_to_utf8(msg);
		if (code == 0) {
			NXDBG("%s", utf.c_str());
		}
		else
		{
			NXERR("%s(Error=%lu(%#X))", utf.c_str(), code, code);
		}
	}
};

char sMsgBuf[1024];
//NX application ID
static int app_id = 0;

/*----------------------------------------------------------------------------*
 *  NXL_APP_enter
 *
 *      Enter the application
 *----------------------------------------------------------------------------*/
static void NXL_APP_enter( void)
{
	NXDBG("=========================================");

	/* Place code to enter application here */
	NXDBG("Ends here..." );
}

/*----------------------------------------------------------------------------*
 *  NXL_APP_init
 *
 *      Initialize the application
 *----------------------------------------------------------------------------*/
static void NXL_APP_init( void )
{
	NXDBG("=========================================");

	/* Place code to enter application here */

	NXDBG("Ends here..." );
}

/*----------------------------------------------------------------------------*
 *  NXL_APP_exit
 *
 *      Exit the application
 *----------------------------------------------------------------------------*/
static void NXL_APP_exit( void )
{
	NXDBG("=========================================");

	/* Place code to enter application here */
	//hook_uninstall();
	NXDBG("Ends here..." );
}
#define NXL_UTILS_DLL "NxlUtils64.dll"
void system_info()
{
	//Show environment information
	char path[MAX_PATH];
	get_module_dir(TARGETFILENAME, path, MAX_PATH);
	NXSYS(TARGETFILENAME ": Version=" FILE_VER " Folder='%s'", path);
	NXSYS("CurrentDirectory='%s'", NxlPath::GetCurrectDirectory().tstr());
	nx_get_major_version();
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	wchar_t path[MAX_PATH];
	int len = GetModuleFileNameW(hModule, path, MAX_PATH);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		RMXUtils::initializeLogger();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		TRACELOG("unloading lib-'%S'", path);
		log4cplus::Logger::shutdown();
		break;
	}
	return TRUE;
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
	NxDllLoader loader;
	if (!loader.Load(WTEXT(NXL_UTILS_DLL)))
		return;
	system_info();
	/* Initialize the API environment */
	if( NX_EVAL(UF_initialize()) ) 
	{
		//version check
		if (nx_get_major_version() != NX_MAJOR_VER)
		{
			nx_dialog_show("The version of this NX RMX is NOT matched with your NX");
			//return;
		}
		if (!loader.Load(L"RPMUtils64.dll"))
			return;
		UF_MB_application_t appData;
		char name[] = APP_NAME;


		//register NXL App
		/* Initialize application data */
		
		appData.name       = name;
		appData.id         = 0;
		appData.init_proc  = NXL_APP_init;
		appData.exit_proc  = NXL_APP_exit;
		appData.enter_proc = NXL_APP_enter;
		
		/* Initialize application support flags */

		appData.drawings_supported          = TRUE;
		appData.design_in_context_supported = TRUE;
		
		/* Register the application with UG */
		
		NX_CALL(UF_MB_register_application( &appData ));
		
		app_id = appData.id;
		NXDBG("App ID=%d", app_id);
		
		/*Hook NX api*/
		if (install_nx_hook())
		{
			install_rmxui_items();
			install_usage_control();//TODO:what if failed
			install_secure_overlay();
		}

		/* Terminate the API environment */
		NX_CALL(UF_terminate());
		return;
	}
	/* Failed to initialize */
}