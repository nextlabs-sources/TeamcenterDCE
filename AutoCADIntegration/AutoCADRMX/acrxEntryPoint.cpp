// (C) Copyright 2002-2012 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

//-----------------------------------------------------------------------------
//----- acrxEntryPoint.cpp
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"

#include <CommonTypes.h>
#include <DelayLoadDllHlp.h>

#include <LibRMX.h>

#include "AcInc.h"
#include "AcRMXUtils.h"

#include "AcRMXApDocReactor.h"
#include "AcRMXEditorReactor.h"
#include "AcRMXPublishReactor.h"

#include "AcRMXAccessControl.h"
#include "AcRXMProtectControl.h"
#include "AcRMXWatermarkControl.h"
#include "AcRMXSessionCacheMgr.h"
#include "AcRMXRPMDirMgr.h"

#include "AcRMXHooks.h"

using namespace std;

//-----------------------------------------------------------------------------
#define szRDS _RXST("nxrm")
ENABLE_DELAYLOAD_HOOK

extern "C" IMAGE_DOS_HEADER __ImageBase;
static const wchar_t* DLL_LIBRMX = L"LibRMX_x64.dll";

//-----------------------------------------------------------------------------
//----- ObjectARX EntryPoint
class CAutoCADRMXApp : public AcRxArxApp {
private:
	CPathEx m_appPath;
	bool m_bAppInited;
	CPathEx m_cuiPath;

private:
	bool InitRMXInstance()
	{
		//
		// Init RMX
		//
		const wstring& strAppDir = m_appPath.GetParentPath();
		// The envionrment variable cannot take effect until machine is restarted, after RMX installed. 
		// For this case, set the env var here.
		if (RMXUtils::getEnv(ENV_RMX_ROOT).empty())
		{
			// Folder structure should be ${AUTOCAD_RMX_ROOT}\AutoCAD 2022\nxrmAutoCADRMX2022.arx
			// The log config file locates in ${AUTOCAD_RMX_ROOT}
			// So grab the RMX root dir based on app path as below
			CPathEx rmxRootPath(strAppDir.c_str());
			rmxRootPath = rmxRootPath.GetParentPath().c_str();
			wstring strRMXRootEnv(ENV_RMX_ROOT);
			strRMXRootEnv += L"=" + rmxRootPath.ToString();
			if (_wputenv(strRMXRootEnv.c_str()) != 0) {
				LOG_ERROR(L"Cannot init RMX (Error: Failed to set AUTOCAD_RMX_ROOT env var)");
				return false;
			}
		}

		RMXLOG_BEGIN();
		LOG_INFO_STR(L"RMX for AutoCAD is initializing...");

		// Load library explicitly with a given path
		// Search the LibRMX dll from the same folder of RMX
		CPathEx dllPath(strAppDir.c_str());
		dllPath /= DLL_LIBRMX;
		ADD_DELAYLOAD_DLL(dllPath.ToString());

		try
		{
			if (!RMX_Initialize())
			{
				wchar_t* szErrMsg = nullptr;
				if (RMX_GetLastError(&szErrMsg) != ERROR_SUCCESS)
				{
					LOG_ERROR_FMT(L"Cannot initialize RMX due to the error: \n%s", szErrMsg);
					const CString& msg = AcRMXUtils::FormatString(IDS_ERROR_APP_LOAD, szErrMsg);
					AcRMXUtils::AlertError(msg);
					RMX_ReleaseArray((void*)szErrMsg);
				}

				UnloadDelayLoadedDlls();

				return false;
			}
		}
		catch (...)
		{
			CSysErrorMessage lastError(GetLastError());
			LOG_ERROR_FMT(L"Failed to load '%s' (error: %s)", dllPath.c_str(), (LPCTSTR)lastError);
			const CString& msg = AcRMXUtils::FormatString(IDS_ERROR_APP_LOAD, (LPCTSTR)lastError);
			AcRMXUtils::AlertError(msg);
			return false;
		}

		return true;

	}

	void DeleteRMXInstance()
	{
		RMX_Terminate();

		UnloadDelayLoadedDlls();

		LOG_INFO(L"RMX for AutoCAD terminated");

		RMXLOG_END();
	}

	/// <summary>
	/// Call rxInit on all classes we have defined that are derived from AcRxObject or any of its dependants
	/// </summary>
	/// <param name="pkt"></param>
	/// <returns></returns>
	void RegisterClasses()
	{

		LOG_DEBUG(L"Register classes...");
		CAcRMXApDocReactor::rxInit();
		CAcRMXEditorReactor::rxInit();

		acrxBuildClassHierarchy();
	}

	/// <summary>
	/// Reverse everything done in RegisterClasses()
	/// </summary>
	void UnregisterClasses()
	{

		LOG_DEBUG(L"Unregister classes...");
		deleteAcRxClass(CAcRMXApDocReactor::desc());
		deleteAcRxClass(CAcRMXEditorReactor::desc());
	}

	void AddReactors()
	{
		CAcRMXApDocReactor::getInstance();
		CAcRMXEditorReactor::getInstance();
		CAcRMXPublishReactor::getInstance();
		CAcRMXApWinowMgrReactor::getInstance();
	}

	void RemoveReactors()
	{
		CAcRMXApDocReactor::destroyInstance();
		CAcRMXEditorReactor::destroyInstance();
		CAcRMXPublishReactor::destroyInstance();
		CAcRMXApWinowMgrReactor::destroyInstance();
	}

	void LoadCUI()
	{
		m_cuiPath = m_appPath.GetParentPath().c_str();
		m_cuiPath /= L"Resources\\en-US\\autocadrmx.cuix";
		if (!acedIsMenuGroupLoaded(L"NextLabsRMX"))
		{
			acedLoadPartialMenu(m_cuiPath.c_str());
		}
	}

	void UnloadCUI()
	{
		if (acedIsMenuGroupLoaded(L"NextLabsRMX"))
		{
			acedUnloadPartialMenu(m_cuiPath.c_str());
		}
	}

public:
	CAutoCADRMXApp() : AcRxArxApp()
		, m_bAppInited(false)
	{
	}

	virtual AcRx::AppRetCode On_kInitAppMsg(void* pkt)
	{
		m_appPath = acedGetAppName();

		AcRx::AppRetCode retCode = AcRx::kRetError;
		if (InitRMXInstance())
		{
			// You *must* call On_kInitAppMsg here
			retCode = AcRxArxApp::On_kInitAppMsg(pkt);
			if (retCode == AcRx::kRetOK)
			{
				LOG_INFO(L"RMX App loaded");

				LOG_INFO(L"AcRMX Information");
				LOG_INFO_FMT(L"\t-Version: %s", _W(VER_FILEVERSION_STR));
				LOG_INFO_FMT(L"\t-Build: %s", _W(VER_BUILD_STR));
				LOG_INFO_FMT(L"\t-Path: %s", m_appPath.c_str());

				AcDbHostApplicationServices* pAppSrv = acdbHostApplicationServices();
				if (pAppSrv != NULL)
				{
					LOG_INFO(L"Host App Service Information");
					LOG_INFO_FMT(L"\t-Product: %s", pAppSrv->product());
					LOG_INFO_FMT(L"\t-Release: %s", pAppSrv->releaseMajorMinorString());
				}
				
				RegisterClasses();

				AddReactors();

				LoadCUI();

				SessionCacheMgr().Start();
				ProtectController().Start();
				AccessController().Start();
				WatermarkController().Start();
				RPMDirMgr().Start();

				AcRMXHooks::InstallGlobalHooks();

				// In case the file is opened via DWG Launcher, that cuases file gets
				// opened before RMX plugin loaded(acad.exe added as trusted app in RPM register.xml), 
				// file open reactor cannot be caught by RMX
				auto pCurDoc = acDocManager->mdiActiveDocument();
				if (pCurDoc != NULL && SessionCacheMgr().AddNxlFile(pCurDoc->fileName()))
				{
					WatermarkController().UpdateViewOverlay(pCurDoc);
					RMX_AddActivityLog(pCurDoc->fileName(), RMX_OView, RMX_RAllowed);
					if (!SessionCacheMgr().CheckRight(pCurDoc->fileName(), RMX_RIGHT_EDIT))
					{
						// Alert the no edit permission granted to save changes
						CString msg = AcRMXUtils::LoadString(IDS_WRAN_CANNOTSAVE);
						msg += pCurDoc->fileName();
						LOG_ERROR((LPCTSTR)msg);

						AcRMXUtils::AlertWarn(msg);
					}
				}
			
				// Disable auto-save
				// TODO: Disave if no edit and save as permission.
				/*struct resbuf res;
				res.restype = RTSHORT;
				res.resval.rint = 0;
				acedSetVar(L"SAVETIME", &res);*/

//#if !defined(NDEBUG)
//				AcRMXUtils::PrintAllCommands();
//#endif // NDEBUG

				m_bAppInited = true;

			}
		}
		if (retCode == AcRx::kRetError)
		{
			LOG_INFO(L"Failed to load RMX App");
		}
		return (retCode);
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg(void* pkt)
	{
		if (m_bAppInited)
		{
			// Don't allow to unload RMX plugin in case any protected file is still openning
			if (SessionCacheMgr().HasNxlFileLoaded())
			{
				AcRMXUtils::AlertError(IDS_ERROR_DENY_UNLOADAPP);
				return AcRx::kRetError;
			}

			AcRMXHooks::UnhookGlobalHooks();
			SessionCacheMgr().Stop();
			ProtectController().Stop();
			AccessController().Stop();
			WatermarkController().Stop();
			RPMDirMgr().Stop();

			UnloadCUI();

			RemoveReactors(); 

			UnregisterClasses();

			DeleteRMXInstance();

		}
		
		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode = AcRxArxApp::On_kUnloadAppMsg(pkt);
		if (retCode == AcRx::kRetOK)
		{
			LOG_INFO(L"RMX App unloaded");
		}
		else
		{
			LOG_INFO(L"Failed to unload RMX App");
		}
		return (retCode);
	}

	virtual void RegisterServerComponents() {
	}

	// The ACED_ADSFUNCTION_ENTRY_AUTO / ACED_ADSCOMMAND_ENTRY_AUTO macros can be applied to any static member 
	// function of the CAutoCADRMXApp class.
	// The function may or may not take arguments and have to return RTNORM, RTERROR, RTCAN, RTFAIL, RTREJ to AutoCAD, but use
	// acedRetNil, acedRetT, acedRetVoid, acedRetInt, acedRetReal, acedRetStr, acedRetPoint, acedRetName, acedRetList, acedRetVal to return
	// a value to the Lisp interpreter.
	//
	// NOTE: ACED_ADSFUNCTION_ENTRY_AUTO / ACED_ADSCOMMAND_ENTRY_AUTO has overloads where you can provide resourceid.

	//- ACED_ADSFUNCTION_ENTRY_AUTO(classname, name, regFunc) - this example
	//- ACED_ADSSYMBOL_ENTRYBYID_AUTO(classname, name, nameId, regFunc) - only differs that it creates a localized name using a string in the resource file
	//- ACED_ADSCOMMAND_ENTRY_AUTO(classname, name, regFunc) - a Lisp command (prefix C:)
	//- ACED_ADSCOMMAND_ENTRYBYID_AUTO(classname, name, nameId, regFunc) - only differs that it creates a localized name using a string in the resource file

	// Lisp Function is similar to ARX Command but it creates a lisp 
	// callable function. Many return types are supported not just string
	// or integer.
	// ACED_ADSFUNCTION_ENTRY_AUTO(CAutoCADRMXApp, RMXLispFunction, false)
	static int ads_RMXADDRPMDIR() {
		struct resbuf *rbArgs = acedGetArgs () ;

		if (rbArgs == NULL) {
			acutPrintf(_T("\nArgument not specified. Please specify a folder path."));
			
			return RTERROR;
		}
			
		if (rbArgs->restype != RTSTR)
		{
			acutPrintf(_T("\nInvalid type. Please specify a folder path"));
			return RTERROR;
		}

		if (RMX_IsRPMFolder(rbArgs->resval.rstring))
		{
			acedRetStr(L"ERROR. Already RPM folder");
		}
		else if (RMX_AddRPMDir(rbArgs->resval.rstring))
		{
			acedRetStr(L"RPM folder added successfully");
		}
		else {
			acedRetStr(L"ERROR. Failed to add RPM folder. More error details please refer to autocadrmx.log");
		}

		// Return a value to the AutoCAD Lisp Interpreter
		// acedRetNil, acedRetT, acedRetVoid, acedRetInt, acedRetReal, acedRetStr, acedRetPoint, acedRetName, acedRetList, acedRetVal

		return (RTNORM);
	}

	static int ads_RMXREMOVERPMDIR() {
		struct resbuf* rbArgs = acedGetArgs();

		if (rbArgs == NULL) {
			acutPrintf(_T("\nArgument not specified. Please specify a folder path."));

			return RTERROR;
		}

		if (rbArgs->restype != RTSTR)
		{
			acutPrintf(_T("\nInvalid type. Please specify a folder path"));
			return RTERROR;
		}

		if (!RMX_IsRPMFolder(rbArgs->resval.rstring))
		{
			acedRetStr(L"ERROR. RPM folder not found");
		}
		else if (RMX_RemoveRPMDir(rbArgs->resval.rstring))
		{
			acedRetStr(L"RPM folder removed successfully");
		}
		else
		{
			acedRetStr(L"Failed to unset RPM folder. More error details please refer to autocadrmx.log");
		}

		// Return a value to the AutoCAD Lisp Interpreter
		// acedRetNil, acedRetT, acedRetVoid, acedRetInt, acedRetReal, acedRetStr, acedRetPoint, acedRetName, acedRetList, acedRetVal

		return (RTNORM);
	}

	static void NextLabsRMXrmxabout()
	{
		LOG_DEBUG(L"Executing rmxabout command...");
		const CString& msg = AcRMXUtils::FormatString(IDS_ABOUT_TEXT, _W(VER_FILEVERSION_STR), _W(VER_BUILD_STR), _W(VER_COPYRIGHT_Y));
		AcRMXUtils::AlertInfo(msg);
	}

	static void NextLabsRMXrmxviewinfo()
	{
		LOG_DEBUG(L"Executing rmxviewinfo command...");
		auto pCurDoc = acDocManager->mdiActiveDocument();
		if (pCurDoc != NULL)
		{
			if (!CPathEx::IsFile(pCurDoc->fileName()))
			{
				acutPrintf(L"\nCommand cancelled. File not found in disk.");
				LOG_ERROR(L"rmxviewinfo cancelled. File not found in disk.");
			}
			else if (SessionCacheMgr().IsNxlFile(pCurDoc->fileName()))
			{
				RMX_ShowFileInfoDialog(pCurDoc->fileName(), nullptr, ACRMX_MESSAGEDLG_LABEL);
			}
			else
			{
				acutPrintf(L"\nCommand cancelled. File not protected.");
				LOG_ERROR(L"rmxviewinfo cancelled. File not protected.");
			}
		}
	}

	static void NextLabsRMXrmxprotect()
	{
		LOG_DEBUG(L"Executing rmxprotect command...");
		ProtectController().ProtectCurDocument();
	}

	static void NextLabsRMXrmxprotectwithxrefs()
	{
		LOG_DEBUG(L"Executing rmxprotectwithxrefs command...");
		ProtectController().ProtectCurDocumentWithXrefs();
	}

	static void NextLabsRMXrmxqprotectwithxrefs()
	{
		LOG_DEBUG(L"Executing rmxqprotectwithxrefs command...");
		ProtectController().ProtectCurDocumentWithXrefs(false);
	}

	static void NextLabsRMXrmxqprotect()
	{
		LOG_DEBUG(L"Executing rmxqprotect command...");
		ProtectController().ProtectCurDocument(false);
	}

	static int ads_rmxisprotect() {
		LOG_DEBUG(L"Executing rmxisprotect command...");
		auto pCurDoc = acDocManager->mdiActiveDocument();
		
		if (pCurDoc && pCurDoc->fileName() && SessionCacheMgr().IsNxlFile(pCurDoc->fileName()))
			acedRetInt(1);
		else
			acedRetInt(0);
		
		return (RTNORM);
	}
};

//-----------------------------------------------------------------------------
IMPLEMENT_ARX_ENTRYPOINT(CAutoCADRMXApp)

ACED_ADSSYMBOL_ENTRY_AUTO(CAutoCADRMXApp, RMXADDRPMDIR, false)
ACED_ADSSYMBOL_ENTRY_AUTO(CAutoCADRMXApp, RMXREMOVERPMDIR, false)
ACED_ARXCOMMAND_ENTRY_AUTO(CAutoCADRMXApp, NextLabsRMX, rmxabout, rmxabout, ACRX_CMD_MODAL, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CAutoCADRMXApp, NextLabsRMX, rmxviewinfo, rmxviewinfo, ACRX_CMD_MODAL, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CAutoCADRMXApp, NextLabsRMX, rmxprotect, rmxprotect, ACRX_CMD_MODAL | ACRX_CMD_SESSION, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CAutoCADRMXApp, NextLabsRMX, rmxprotectwithxrefs, rmxprotectwithxrefs, ACRX_CMD_MODAL | ACRX_CMD_SESSION, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CAutoCADRMXApp, NextLabsRMX, rmxqprotect, rmx1protect, ACRX_CMD_MODAL | ACRX_CMD_SESSION, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CAutoCADRMXApp, NextLabsRMX, rmxqprotectwithxrefs, rmxqprotectwithxrefs, ACRX_CMD_MODAL | ACRX_CMD_SESSION, NULL)
ACED_ADSSYMBOL_ENTRY_AUTO(CAutoCADRMXApp, rmxisprotect, false)