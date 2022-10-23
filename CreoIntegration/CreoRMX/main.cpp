// CreoIntegration.cpp : Defines the exported functions for the DLL application.
//
#include "CommonInc.h"

#include "..\common\DelayLoadDllHlp.h"
#include "..\common\CreoRMXLogger.h"
#include <PathEx.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>
#include "OtkXHooks.h"
#include "OtkXTypes.h"
#include "OtkXUIStrings.h"
#include "OtkXUtils.h"

#include <pfcGlobal.h>
#include <wfcSession.h>
#include <xstring.h>

#include <LibRMX.h>
#include "OtkXSessionCache.h"
#include "ProtectController.h"
#include "RPMDirectoryMgr.h"
#include "UsageController.h"
#include "WatermarkController.h"
#include "RMXCommandMgr.h"

using namespace std;
using namespace OtkXUtils;

#define ENABLE_USAGE_CONTROL 1
#define ENABLE_WATERMAKR_OVERLAY

ENABLE_DELAYLOAD_HOOK

static wfcWSession_ptr g_wSession;
static wstring g_rmxAppPath;

extern "C" IMAGE_DOS_HEADER __ImageBase;
static const wchar_t* DLL_LIBRMX = L"LibRMX_x64.dll";

/*A log header for each session*/
#define LOG_BEGIN() \
CREOLOG_INITIALIZE(); \
LOG_INFO_STR(L"***********************************************************"); \
LOG_INFO_STR(L"*      NextLabs Rights Management eXtension for Creo      *"); \
LOG_INFO_STR(L"***********************************************************");

#define LOG_END() \
LOG_INFO_STR(L""); \
LOG_INFO_STR(L""); \
LOG_SHUTDOWN();

bool InitRMX()
{
	// Search the LibRMX dll from the same folder of CreoRMX
	CPathEx libFilePath(g_rmxAppPath.c_str());
	wstring libDir = libFilePath.GetParentPath();
	CPathEx delayLoadDllPath(libDir.c_str());
	delayLoadDllPath /= DLL_LIBRMX;
	// Load library explicitly with a given path
	ADD_DELAYLOAD_DLL(wstring(delayLoadDllPath.c_str()));

	try
	{
		if (!RMX_Initialize())
		{
			wchar_t* szErrMsg = nullptr;
			if (RMX_GetLastError(&szErrMsg) != ERROR_SUCCESS)
			{
				xstring dlgMsg = xstring::Printf("Cannot initialize RMX due to the error: \n%s", RMXUtils::ws2s(szErrMsg).c_str());
				OtkX_ShowMessageDialog(dlgMsg, pfcMESSAGE_ERROR);
				RMX_ReleaseArray((void*)szErrMsg);
			}
			return false;
		}
	}
	catch(...)
	{
		CSysErrorMessage lastError(GetLastError());
		LOG_ERROR_FMT(L"Failed to load '%s' (error: %s)", delayLoadDllPath.c_str(), (LPCTSTR)lastError);
		xstring dlgMsg = xstring::Printf("Cannot load %s due to the error: \n%s", RMXUtils::ws2s(delayLoadDllPath.c_str()).c_str(), RMXUtils::ws2s((LPCTSTR)lastError).c_str());
		OtkX_ShowMessageDialog(dlgMsg, pfcMESSAGE_ERROR);
		return false;
	}
	return true;
}

bool FixOpenFileOnLoad()
{
	bool processed = false;
	// Fix if the file is requested to open before RMX is loaded.
	// Without RMX, the protection file cannot decrypt properly, so try to open it again 
	// after RMX is ready.
	// It will happen when opening nxl file to launch creo app.(e.g.:double click in RPM dir)
	auto cmdLines = RMXUtils::GetCommandLines();
	if (cmdLines.empty())
		return processed;

	pfcModels_ptr pMdls = g_wSession->ListModels();
	if (pMdls != nullptr && pMdls->getarraysize() > 0)
	{
		LOG_INFO("FixOpenFileOnLoad: Ignore. Some model already loaded");
		return processed;
	}

	LOG_DEBUG(L"FixOpenFileOnLoad: Parse command line and retry to open protected file with RMX");
	int i = 0;
	for (auto& cmdLine : cmdLines)
	{
		const wchar_t* szArg = cmdLine.c_str();
		
		LOG_DEBUG_FMT(L"FixOpenFileOnLoad: Check command line args[%d]: %s", i, szArg);
		//if (CPathEx::IsFile(szArg))
		{		
			wstring ext = OtkX_GetFileExtension(szArg);
			wstring plainFilePath = szArg;
			if (wcsicmp(ext.c_str(), NXL_FILE_EXT_WITHOUT_DOT) == 0)
			{
				plainFilePath = plainFilePath.substr(0, wcslen(plainFilePath.c_str()) - 4);
				ext = OtkX_GetFileExtension(plainFilePath.c_str());
				
			}
			if (!OtkX_IsNativeModelType(ext) || !RMX_IsProtectedFile(szArg))
			{
				continue;
			}

			LOG_INFO_FMT(L"FixOpenFileOnLoad: Open request found. Try to open '%s'", szArg);
			CPathEx filePath(szArg);
			string filePathA(RMXUtils::ws2s(szArg));
			if (!RMX_IsRPMFolder(filePath.GetParentPath().c_str()))
			{
				LOG_ERROR(L"FixOpenFileOnLoad: Cannot open the protected file in non RPM folder");
				xstring dlgMsg = xstring::Printf("Cannot open the protected file '%s'.\nYou need to set the folder as SkyDRM folder before openning file.", CA2XS(filePathA.c_str()));
				OtkX_ShowMessageDialog(dlgMsg, pfcMESSAGE_ERROR);
				// Treat as processed to indicates the opening request is handled
				processed = true;
				return processed;
			}
			if (!RMX_CheckRights(filePath.c_str(), RMX_RIGHT_VIEW, true))
			{
				LOG_ERROR(L"FixOpenFileOnLoad: Cannot open the protected file without 'VIEW' permission");
				xstring dlgMsg = xstring::Printf(IDS_ERR_DENY_OPEN, filePathA.c_str());
				OtkX_ShowMessageDialog(dlgMsg, pfcMESSAGE_ERROR);
				// Treat as processed to indicates the opening request is handled
				processed = true;
				return processed;
			}
			
			if (OtkX_OpenFile(xrstring(filePathA.c_str())))
			{
				processed = true;
			}
			
			break;
		}

		++i;
	}

	if (!processed)
		LOG_INFO(L"FixOpenFileOnLoad: Ignore. No open request found");

	return processed;
}

/*
Entry point for Toolkit application
	Create a Navigator Pane Object and Add a pane for current Creo Session
	Create a Model action listener class for post model retrieve event
	Set Toolkit Notification for pre window vacate event
*/
extern "C" int user_initialize(
	int argc,
	char *argv[],
	char *version,
	char *build,
	wchar_t errbuf[80])
{	 
	try
	{
		g_wSession = wfcWSession::cast(pfcGetProESession());
		g_rmxAppPath = CXS2W(g_wSession->GetApplicationPath());
		
		// Set up CREO_RMX_ROOT env var.
		CPathEx appTextPath(CXS2W(g_wSession->GetApplicationTextPath()));
		if (RMXUtils::getEnv(L"CREO_RMX_ROOT").empty())
		{
			if (appTextPath.GetParentPath().empty())
				return (int)wfcTK_APP_INIT_FAIL;
			
			wstring rmxRootEnv = L"CREO_RMX_ROOT=" + appTextPath.GetParentPath();
			if (_wputenv(rmxRootEnv.c_str()) != 0)
				return (int)wfcTK_APP_INIT_FAIL;
		}	

		// Init logging system
		LOG_BEGIN();
		LOG_INFO_STR(L"RMX for Creo is initializing...");

		// Dump Creo information
		LOG_INFO(L"Creo Information");
		LOG_INFO_FMT(L"\t-Release: %d", g_wSession->GetReleaseNumericVersion());
		LOG_INFO_FMT(L"\t-Datecode: %d", CXS2W(g_wSession->GetDisplayDateCode()));
		xstring currDir = g_wSession->GetCurrentDirectory();
		LOG_INFO_FMT(L"\t-Working directory: %s", CXS2W(currDir));
		bool uiMode = OtkX_RunInGraphicMode();
		LOG_INFO_FMT(L"\t-Batch mode: %s", uiMode ? L"No" : L"Yes");
		bool iPEMLoaded = false;
		if (g_wSession->UIGetCommand(OTKX_COMMAND_TC_ABOUT) != nullptr)
		{
			LOG_INFO(L"\t-Teamcenter Integration for Creo: Running");
			iPEMLoaded = true;
		}
		
		LOG_INFO(L"RMX Information");
		LOG_INFO_FMT(L"\t-Version: %s", _W(VER_FILEVERSION_STR));
		LOG_INFO_FMT(L"\t-Build: %s", _W(VER_BUILD_STR));
		LOG_INFO_FMT(L"\t-Exec path: %s", g_rmxAppPath.c_str());
		LOG_INFO_FMT(L"\t-Text path: %s", appTextPath.c_str());
		// Init RMX instance.
		if (!InitRMX())
		{
			LOG_ERROR(L"Failed to start RMX application.");
			UnloadDelayLoadedDlls();
			return (int)wfcTK_APP_INIT_FAIL;
		}

		// Install hooks
		HookInitializer::Startup();

		OtkXSessionCacheHolder().Start();

		CRPMDirectoryMgr::GetInstance().Start();

		CProtectController::GetInstance().Start();

		CUsageController::GetInstance().Start();

		// No need to activate watermark in batch mode
		if (uiMode)
		{			
			CWatermarkController::GetInstance().Start();
			RMXCommandMgrHolder().Start();
		}

		bool fileOpenedOnLoad = false;
		if (!iPEMLoaded)
		{
			// Only for unmanaged mode
			fileOpenedOnLoad = FixOpenFileOnLoad();			
		}		

		// Add current directory as RPM folder, except the file is explicitly requested to open such as 
		// double click, open with from Windows Explorer in unmanaged mode
		if (!currDir.IsEmpty() && !fileOpenedOnLoad)
			CRPMDirectoryMgr::GetInstance().UpdateCurrentDirectory(CXS2W(currDir));

		return (int)wfcTK_NO_ERROR;
	}

	OTKX_EXCEPTION_HANDLER();
	return (int) wfcTK_GENERAL_ERROR;	
}
/*
Exit point for Toolkit application
	Delete a Navigator Pane Object
	Delete a Model action listener class
	Unset Toolkit Notification for pre window vacate event
*/
extern "C" void user_terminate()
{
	// Uninstall hooks
	HookInitializer::Shutdown();

	CProtectController::GetInstance().Stop();

	CRPMDirectoryMgr::GetInstance().Stop();

	CUsageController::GetInstance().Stop();

	if (OtkX_RunInGraphicMode())
	{		
		CWatermarkController::GetInstance().Stop();
		RMXCommandMgrHolder().Stop();
	}

	OtkXSessionCacheHolder().Stop();

	RMX_Terminate();
	
	UnloadDelayLoadedDlls();

	LOG_INFO(L"RMX for Creo terminated");
	LOG_END();
}