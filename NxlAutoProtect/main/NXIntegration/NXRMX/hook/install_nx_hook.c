#include <uf_cgm.h>
#include <ShlObj.h>

#include <hook/hook_defs.h>
#include <NXRMX/nx_hook.h>
#include <NXRMX/nx_utils.h>
#include <NXRMX/nx_contexts.h>

#include <utils_nxl.h>
#include <hook/rmc_fixes.h>
#include <utils_windows.h>

#include <NxlUtils\NxlPath.hpp>
#include <NxlUtils/nxlrunner.exe.hpp>

logical export_cgm_pre(
    UF_CGM_export_reason_t reason,
    UF_CGM_export_source_t source,
    tag_t drawing_sheet,
    void **appl_data)
{
	NXDBG("Reason=%d Source=%d DrawingSheet=%u", reason, source, drawing_sheet);
	return TRUE;
}
void export_cgm_post(
    UF_CGM_export_reason_t reason,
    UF_CGM_export_source_t source,
    tag_t drawing_sheet,
    void *appl_data)
{
	NXDBG("Reason=%d Source=%d DrawingSheet=%u data=%p", reason, source, drawing_sheet, appl_data);
}
#define REGISTER_PART_EVENT_HANDLER(reason, retFuncId) \
if(NX_EVAL(UF_add_callback_function(reason, part_event_handler, #reason, &retFuncId))){	\
	NXDBG("UF_add_callback_function(%d-"#reason"):%p", reason, retFuncId);	\
}

static bool s_is_protection_enabled = true;
bool nxl_protection_is_enabled() {
	return s_is_protection_enabled;
}

RPMSession *g_rpmSession = nullptr;

#ifdef NXVERSION_SPECIFIC_FEATURES
#define INSTALL_VERSION_SPECIFIC_FEATURES(ver) CONCAT(install_nxrmx,ver)()
#define DEFINE_INSTALL_ENTRY(call) \
	extern void call;
DEFINE_INSTALL_ENTRY(INSTALL_VERSION_SPECIFIC_FEATURES(NX_MAJOR_VER));
#else

#define INSTALL_VERSION_SPECIFIC_FEATURES(ver) 
#endif


bool install_nx_hook()
{
	//initialize RPMSession as global variable
	g_rpmSession = new RPMSession(RPM_MAGIC_SECURITY);
	if (!g_rpmSession->IsRPMLoggedIn())
	{
		auto error = g_rpmSession->GetLastError();
		nx_show_listing_window("RPM Session failed with following error:\n\t");
		nx_show_listing_window(error.Message());
		nx_show_listing_window("\n\nPlease ask your administrator for help.");
		return false;
	}
	g_rpmSession->ListRPMInfo();
	g_rpmSession->ListPolicies();

	UF_registered_fn_p_t cbFuncId = NULL;
	//initialize mad c hook
	InitializeMadCHook();

	//install hooks
	//fcc_hook();
	libpart_hook();
	fix_export_outside_teamcenter();
	fix_fcc_download();
	hook_save_part_with_jt_data();
	fix_rpm_deletefile_install();
	INSTALL_VERSION_SPECIFIC_FEATURES(NX_MAJOR_VER);
	if((s_is_protection_enabled = g_rpmSession->IsNxlProtectionEnabled()))
	{
		libpartutils_hook();
		libugui_hook();
		libsyss_hook();
		libpartdisp_hook();
		liblwks_hook();
		libjadex_hook();
		libugutilsint_hook();
		libjafreeformsurfaces_hook();
		hook_save_bookmark();
		NX_EVAL(UF_CGM_register_callbacks(export_cgm_pre, export_cgm_post, NULL));
	}

	//
	REGISTER_PART_EVENT_HANDLER(UF_create_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_open_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_save_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_save_as_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_close_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_modified_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_rename_part_reason, cbFuncId);
	REGISTER_PART_EVENT_HANDLER(UF_change_work_part_reason, cbFuncId);
	
	char szPath[MAX_PATH];
	if (nx_isugmr())
	{
		//set FCC root folder as RPMDir
		if (SUCCEEDED(SHGetFolderPathA(NULL,
			CSIDL_PROFILE,	//c:\users\<username>
			NULL,
			0,
			szPath)))
		{
			PathAppend(szPath, "FCCCache");
			NxlPath path(szPath);
			bool added;
			if (g_rpmSession->AddRPMFolder(path.wstr(), &added) == 0)
			{
				if (added) {
					//set FCCCache as RPMDir
					NXSYS("Added '%s' as RPM safe directory", path.tstr());
				}
				else
				{
					NXDBG("'%s' is already RPM safe diretory", path.tstr());
				}
			}
		}
	}
#define ENV_SET_RPM "SET_UGII_TMP_DIR_AS_RPM"
	if (GetEnvironmentVariable(ENV_SET_RPM, szPath, sizeof(szPath)) > 0)
	{
		DBGLOG(ENV_SET_RPM"=%s", szPath);
		if (_stricmp(szPath, "1") == 0)
		{
			//set UGII_TMP_PATH as rpm folder
			auto nxtmp = nx_get_tmp();
			if (nxtmp.length() >0)
			{
				NxlPath ugiitmp(nxtmp);
				bool added;
				DBGLOG("NX TMP dir='%s'", ugiitmp.tstr());
				//https://support.sw.siemens.com/en-US/product/209349590/knowledge-base/PL8100836
				//from NX1847, if UGII_TMP_DIR is invalid, the nx will still be start up, and use %TMP% as instead to create syslog
				//if recreate our custom tmp folder here, some of following operation will still use this custom temp folder
				//but it's still highly recommanded to restart NX to ensure everything works no issue.
				if (!ugiitmp.CheckExist()) {
					DBGLOG("'%s' doesn't exist, recreat it now..", ugiitmp.tstr());
					if (!CreateDirectoryW(ugiitmp.wchars(), NULL))
					{
						DWORD error = GetLastError();
						NXERR("CreateDirectory failed with error-%lu", error);
					}
				}
				if (g_rpmSession->AddRPMFolder(ugiitmp.wstr(), &added) == 0)
				{
					if (added) {
						//set FCCCache as RPMDir
						NXSYS("Added %%UGII_TMP_DIR%%('%s') as RPM safe directory", ugiitmp.tstr());
					}
					else
					{
						DBGLOG("%%UGII_TMP_DIR%%('%s') is already RPM safe diretory", ugiitmp.tstr());
					}
				}
			}
		}
	}
	else
	{
		NXWAR("Environment variable-%%" ENV_SET_RPM "%% is not defined, please make sure %%UGII_TMP_DIR%% has been set as RPM folder manually");
	}

	//clear transient rpm dir
	transient_rpm_dir_clear();

	//TODO:check whether installation is complete
	if (g_rpmSession->IsRPMLoggedIn())
	{
		//use nxlrunner to set this application as trusted
		NxlRunner runner;
		if (runner.SetTrustApp()) {

		}
	}
	return true;
}