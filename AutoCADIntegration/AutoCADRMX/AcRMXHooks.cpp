//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//

#include "StdAfx.h"
#include "AcRMXHooks.h"
#include <mapi.h>
#include "HookDefs.h"
#include <madCHook.h>
#include "AcRMXUtils.h"
#include "AcRMXAccessControl.h"
#include "AcRMXSessionCacheMgr.h"

#include <LibRMX.h>

using namespace std;

DEFINE_HOOK_CALLBACK(CreateFileW)
static HANDLE CreateFileW_hooked(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	auto callNextFunc = [&]() -> HANDLE {
		return CreateFileW_next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	};

	// Workaround: File locked after open, cannot rename newly protected file to original opened file.
	// Set the FILE_SHARE_DELETE flag to allow rename
	
	// 11/12/2021 Update: 
	// This workaround not working in Trista's VM. 
	// Change the workaround to CopyFile instead of MoveFile(rename). If RPM dir is added with RPMFOLDER_OVERWRITE flag, it allows to overwrite
	/*if (dwCreationDisposition == OPEN_EXISTING)
	{
		if (HAS_BIT(dwDesiredAccess, GENERIC_WRITE) || HAS_BIT(dwDesiredAccess, GENERIC_WRITE | GENERIC_READ))
		{
			if (AcRMXUtils::IsSupportedFileType(lpFileName))
			{
				dwShareMode |= FILE_SHARE_DELETE;
				LOG_DEBUG(L"CreateFileW_hooked. FILE_SHARE_DELETE flag set for " << lpFileName);
			}
		}
	}*/
	if ((dwCreationDisposition == CREATE_ALWAYS) && HAS_BIT(dwDesiredAccess, GENERIC_WRITE))
	{
		if (wcsstr(lpFileName, L"_recover") != NULL)
		{
			SessionCacheMgr().AddRecoverFile(lpFileName);
		}
	}
	return callNextFunc();
}

DEFINE_HOOK_CALLBACK(MAPISendMail)
ULONG MAPISendMail_hooked(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
)
{
	LOG_DEBUG_FMT(L"MAPISendMail_hooked(%d)", lpMessage->nFileCount);
	for (ULONG i = 0; i < lpMessage->nFileCount; i++)
	{
		auto szFilePath = lpMessage->lpFiles[i].lpszPathName;
		const wstring& mapiFilePath = RMXUtils::s2ws(const_cast<char*>(szFilePath));
		if (!AccessController().CheckCommandRight(/*MSGO*/L"RMXMAPISendMail", mapiFilePath.c_str()))
		{
			return MAPI_E_FAILURE;
		}
	}
	return MAPISendMail_next(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

//
// Hook CNavDialog::OnOK for other usage control when file i selected. e.g.: attach, import
//
typedef void* AcNavDlg_ptr;
#define AcNavDlg_OnOK_FULLNAME "?OnOK@CNavDialog@@MEAAXXZ"
#define AcNavDlg_OnOK_ORDINAL 732
typedef void(*pfAcNavDlg_OnOK)(AcNavDlg_ptr);
static pfAcNavDlg_OnOK AcNavDlg_OnOK_original;
static pfAcNavDlg_OnOK AcNavDlg_OnOK_next;
static bool g_bCreatingNewDoc = false;
static void AcNavDlg_OnOK_hooked(AcNavDlg_ptr p)
{
	CNavDialog* pDlg = (CNavDialog*)(p);
	bool bCmdDenied = false;
	if (pDlg)
	{	
		CString csFullPath;
		pDlg->GetSingleDataFullPath(csFullPath);
		LOG_DEBUG_FMT(L"AcNavDlg_OnOK_hooked. Dlg=%s, SelectedFile=%s", (LPCWSTR)pDlg->GetDialogCaption(),
			(LPCWSTR)csFullPath);

		CPathEx fPathEx((LPCWSTR)csFullPath);
		bool bCmdToCheckRight = false;
		CString csCmdToCheck;
		// New from template
		// Note: Don't check dialog caption which is English text, but cannot work well for other language.
		// g_bCreatingNewDoc is updated by documentCreateStarted callback
		if (g_bCreatingNewDoc)
		{
			CString csCmd;
			AcRMXUtils::GetSysVar(L"CMDNAMES", csCmd);
			LOG_DEBUG_FMT(L"\t-CMDNAMES: %s", (LPCWSTR)csCmd);
			// Check if this dialog is template select dialog based on filters
			// TODO: Didn't find a better way to determine if this is template select dialog, below check is 
			// really relying on the filter list in build-in dialog. High risk.
			// Need to investigate better solution.
			CNavFilterArray& aFilters = pDlg->GetFilterArray();
			if ((aFilters.GetCount() == 3) && (aFilters.Find(L"*.dwt") == 0) &&
				(aFilters.Find(L"*.dwg") == 1) && (aFilters.Find(L"*.dws") == 2))
			{
				bCmdToCheckRight = true;
				csCmdToCheck = L"RMXNEW";
			}
		}
		else
		{
			// Disable import and some attach commands
			CString csCmd;
			AcRMXUtils::GetSysVar(L"CMDNAMES", csCmd);
			LOG_DEBUG_FMT(L"\t-CMDNAMES: %s", (LPCWSTR)csCmd);
			AccessController().EnsureCommandName(csCmd);
			if (AccessController().IsImportCmd(csCmd))
			{
				bCmdToCheckRight = true;
				csCmdToCheck = L"RMXIMPORT";
			}
			else if (csCmd.Find(L"ATTACH") != -1)
			{
				bCmdToCheckRight = true;
				csCmdToCheck = L"RMXATTACH";
			}
		}

		if (bCmdToCheckRight)
		{
			bool addToCache = false;
			if (!SessionCacheMgr().IsNxlFile(fPathEx.c_str()) && SessionCacheMgr().AddNxlFile(fPathEx.c_str()))
				addToCache = true;

			if (!AccessController().CheckCommandRight(csCmdToCheck, fPathEx.c_str()))
			{
				bCmdDenied = true;
			}

			if (addToCache)
				SessionCacheMgr().RemoveNxlFile(fPathEx.c_str());
		}
	}
	
	if (!bCmdDenied)
		AcNavDlg_OnOK_next(p);
}

// Convert
#define ConvertDwg_FULLNAME "Convert"
#define ConvertDwg_ORDINAL 23
typedef void(*pfConvertDwg)();
static pfConvertDwg ConvertDwg_original;
static pfConvertDwg ConvertDwg_next;
static void ConvertDwg_hooked()
{
	LOG_INFO(L"ConvertDwg_hooked entering...");
	if (!AccessController().CheckCommandRight(L"DWGCONVERT", L"Dummy.dwg"))
		return;

	ConvertDwg_next();
}

//
// bool DragInsertBlock(wchar_t const *,wchar_t const *,bool,double,double,double,bool,double,double,double,bool,double,bool,bool,bool,bool,bool,bool &,unsigned int &)
//
#define DragInsertBlock_FULLNAME "?DragInsertBlock@@YA_NPEB_W0_NNNN1NNN1N11111AEA_NAEAI@Z"
#define DragInsertBlock_ORDINAL 217
typedef bool(*pfDragInsertBlock)(wchar_t const*, wchar_t const*, bool, double, double, double, bool, double, double, double, bool, double, bool, bool, bool, bool, bool, bool&, unsigned int&);
static pfDragInsertBlock DragInsertBlock_original;
static pfDragInsertBlock DragInsertBlock_next;
static bool DragInsertBlock_hooked(wchar_t const* p1, wchar_t const* p2, bool p3, double p4, double p5, double  p6, 
	bool p7, double p8, double p9, double p10, bool p11, double p12, bool p13, bool p14, bool p15, bool p16, bool p17, bool& p18, unsigned int& p19)
{
	LOG_DEBUG(L"DragInsertBlock_hooked: " << p1 << L"," << p2);

	// Check if this dialog is template select dialog based on filters
	bool addToCache = false;
	bool bDenied = false;
	if (!SessionCacheMgr().IsNxlFile(p2) && SessionCacheMgr().AddNxlFile(p2))
	{
		addToCache = true;
	}
	
	if (!AccessController().CheckCommandRight(L"RMXINSERTBLOCK", p2))
	{
		bDenied = true;
	}

	if (addToCache)
		SessionCacheMgr().RemoveNxlFile(p2);

	// Skip call if right not granted.
	if (bDenied)
		return false;

	return DragInsertBlock_next(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19);
}

void AcRMXHooks::InstallGlobalHooks()
{
	InitializeMadCHook();
	LOG_INFO(L"MadCHook initialized");
	SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);
	
	HOOK_API("MAPI32", MAPISendMail);
	HOOK_API("ACETRANSMITUI.ARX", ConvertDwg);
	HOOK_API("ACBLOCK.CRX", DragInsertBlock);
}

void AcRMXHooks::UnhookGlobalHooks()
{
	UNHOOK(MAPISendMail);
	UNHOOK(ConvertDwg);
	UNHOOK(DragInsertBlock);

	FinalizeMadCHook();
	LOG_INFO(L"MadCHook Finalized");
}

void AcRMXHooks::Hook_NavDlgOK(bool creatingDoc)
{
	HOOK_API("anav.dll", AcNavDlg_OnOK);
	g_bCreatingNewDoc = creatingDoc;
}

void AcRMXHooks::Unhook_NavDlgOK()
{
	UNHOOK(AcNavDlg_OnOK);
	g_bCreatingNewDoc = false;
}

void AcRMXHooks::Hook_CreateFile()
{
	HOOK_API("KernelBase.dll", CreateFileW);
}

void AcRMXHooks::Unhook_CreateFile()
{
	UNHOOK(CreateFileW);
	SessionCacheMgr().ClearupReoverFiles();
}
