#include "stdafx.h"

#include "CatSession.h"
#include "RMXUsageControl.h"
#include "RMXTypes.h"
#include <LibRMX.h>

#define BEGIN_CMD_MAP m_cmdMap = {
#define END_CMD_MAP };

#define CMD_ENTRY(cmdName, right, checkRefs, funcName) \
{cmdName, UCCmd(cmdName, right, checkRefs, &CheckRight_ ##funcName) }

#define CMD_ENTRY_DEFAULT(cmdName, right, checkRefs) \
CMD_ENTRY(cmdName, right, checkRefs, Default)

bool CheckRight_Default(const UCCmd& ucCmd)
{
	auto ret = curUCControl.CheckRight(ucCmd.bCheckReferences, [&](CAT::DocumentPtr pDoc, bool bRefDoc) -> bool {
		wstring wsFileFullName;
		if (!CCatSession::GetDocFileName(pDoc, wsFileFullName) ||
			!curCatSession.IsNxlFile(wsFileFullName))
			return true; // Ignore if it's not valid nxl.

		// Check if file permission is granted
		if (!curCatSession.CheckRight(wsFileFullName, ucCmd.dwAccessRight))
		{
			wchar_t szRightName[50];
			RMX_GetRightName(ucCmd.dwAccessRight, szRightName);
			UINT nMsgID = bRefDoc ? IDS_ERROR_DENY_OP_DEP : IDS_ERROR_DENY_OP;
			ALERT_ERROR(nMsgID, szRightName, wsFileFullName.c_str());

			return false;
		}

		return true;
	});

	return (ret == CatRMX::eRightGrant);
}

bool CheckRight_DisableOnNxlOpened(const UCCmd& ucCmd)
{
	// R13.3: Disable if any protected file is openning
	wstring msg;
	bool ret = curCatSession.WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
		wstring wsFileFullName;
		if (curCatSession.GetDocFileName(pDoc, wsFileFullName) && curCatSession.IsNxlFile(wsFileFullName))
		{
			msg += L"\n\t" + wsFileFullName;
		}

		return false;
	});

	if (!msg.empty())
	{
		ALERT_ERROR(IDS_ERROR_DENY_NXLOPENED, msg.c_str());
		return false;
	}
	
	return true;
}

//bool CheckRight_Open(const UCCmd& ucCmd)
//{
//	// Currently the only case is to check right by given file path. The context is not active doc
//	// as during load phase, the doc is not activated in session yet. 
//	if (ucCmd.eCtx == eContxt_File)
//	{
//		// Skip the install files
//		const wstring& installDir = curCatSession.GetInstallDir();
//		if (StringHlp::IFindSubString<wchar_t>(ucCmd.wsCtxFileFullName, installDir) != wstring::npos)
//			return true;
//
//		if (curCatSession.IsNxlFile(ucCmd.wsCtxFileFullName))
//		{
//			if (!curCatSession.CheckRight(ucCmd.wsCtxFileFullName, ucCmd.dwAccessRight))
//			{
//				// Mark readonly if nxl does not have edit right
//				DWORD attr = GetFileAttributesW(ucCmd.wsCtxFileFullName.c_str());
//				if ((INVALID_FILE_ATTRIBUTES != attr) && !(HAS_BIT(attr, FILE_ATTRIBUTE_READONLY)))
//				{
//					attr |= FILE_ATTRIBUTE_READONLY;
//					SetFileAttributes(ucCmd.wsCtxFileFullName.c_str(), attr);
//				}
//
//				curCatSession.ShowStatusBar(IDS_WARN_DENY_SAVECHANGE, ucCmd.wsCtxFileFullName.c_str());
//			}
//		}
//	}
//	
//	return true;
//}

bool CheckRight_Disable(const UCCmd& ucCmd)
{
	auto ret = curUCControl.CheckRight(ucCmd.bCheckReferences, [&](CAT::DocumentPtr pDoc, bool bRefDoc) -> bool {
		wstring wsFileFullName;
		if (!CCatSession::GetDocFileName(pDoc, wsFileFullName) ||
			!curCatSession.IsNxlFile(wsFileFullName))
			return true; // Ignore if it's not valid nxl.

		// Always block action on the protected file. No right check
		UINT nMsgID = bRefDoc ? IDS_ERROR_DENY_ALWAYS_DEP : IDS_ERROR_DENY_ALWAYS;
		ALERT_ERROR(nMsgID, wsFileFullName.c_str());
	
		return false;
	});

	return (ret == CatRMX::eRightGrant);
}

bool CheckRight_SaveAll(const UCCmd& ucCmd)
{
	wstring msg;
	bool ret = curCatSession.WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
		wstring wsFileFullName;
		if (CCatSession::GetDocFileName(pDoc, wsFileFullName) &&
			curCatSession.IsNxlFile(wsFileFullName))
		{
			// Only check right on modified doc
			// The file never modfiied, we don't deny action, becuase save will be fired.
			VARIANT_BOOL vSaved = VARIANT_TRUE;
			if (SUCCEEDED(pDoc->get_Saved(&vSaved)) && (vSaved == VARIANT_FALSE))
			{
				LOG_DEBUG(L"Nxl file modified in session" << wsFileFullName.c_str());
				if (!curCatSession.CheckRight(wsFileFullName, ucCmd.dwAccessRight))
				{
					msg += L"\n\t" + wsFileFullName;
				}
			}
		}
		return false;
	});
	
	if (msg.empty())
		return true;

	ALERT_ERROR(IDS_ERROR_DENY_DIRTY, msg.c_str());

	return false;
}

bool CheckRight_Print(const UCCmd& ucCmd)
{
	auto ret = curUCControl.CheckRight(ucCmd.bCheckReferences, [&](CAT::DocumentPtr pDoc, bool bRefDoc) -> bool {
		wstring wsFileFullName;
		if (!CCatSession::GetDocFileName(pDoc, wsFileFullName) ||
			!curCatSession.IsNxlFile(wsFileFullName))
			return true; // Ignore if it's not valid nxl.

		// Check if file permission is granted
		if (!curCatSession.CheckRight(wsFileFullName, ucCmd.dwAccessRight))
		{
			wchar_t szRightName[50];
			RMX_GetRightName(ucCmd.dwAccessRight, szRightName);
			UINT nMsgID = bRefDoc ? IDS_ERROR_DENY_OP_DEP : IDS_ERROR_DENY_OP;
			ALERT_ERROR(nMsgID, szRightName, wsFileFullName.c_str());

			return false;
		}

		// Addtional edit right check on modified doc
		VARIANT_BOOL vSaved = VARIANT_TRUE;
		if (SUCCEEDED(pDoc->get_Saved(&vSaved) && !vSaved) && !curCatSession.CheckRight(wsFileFullName, RMX_RIGHT_EDIT))
		{
			ALERT_ERROR(IDS_ERROR_DENY_DIRTY, wsFileFullName.c_str());
			return false;
		}

		return true;
	});

	return (ret == CatRMX::eRightGrant);
}

bool CheckRight_ScreenCapture(const UCCmd& ucCmd)
{
	if (ucCmd.eCtx == eContext_File)
	{
		if (!curCatSession.IsNxlFile(ucCmd.wsCtxFileFullName))
			return true; // Ignore if it's not valid nxl.

		// Check if file permission is granted
		if (!curCatSession.CheckRight(ucCmd.wsCtxFileFullName, ucCmd.dwAccessRight, false))
		{
			LOG_ERROR_FMT(L"Don't allow to capture screen. No 'Extract' permission granted on the file: %s", ucCmd.wsCtxFileFullName.c_str());
			return false;
		}
	}
	else
	{
		auto ret = curUCControl.CheckRight(ucCmd.bCheckReferences, [&](CAT::DocumentPtr pDoc, bool bRefDoc) -> bool {
			wstring wsFileFullName;
			if (!CCatSession::GetDocFileName(pDoc, wsFileFullName) ||
				!curCatSession.IsNxlFile(wsFileFullName))
				return true; // Ignore if it's not valid nxl.

			// Check if file permission is granted
			if (!curCatSession.CheckRight(wsFileFullName, ucCmd.dwAccessRight, false))
			{
				LOG_ERROR_FMT(L"Don't allow to capture screen. No 'Extract' permission granted on the document: %s", wsFileFullName.c_str());

				return false;
			}

			return true;
		});

		return (ret == CatRMX::eRightGrant);
	}

	return true;
}

bool CheckRight_DisableOnRMXLoaded(const UCCmd& ucCmd)
{
	ALERT_ERROR(IDS_ERROR_DENY_WHEN_RMX_LOAED, L"");
	
	return false;
}

// Only happen when saving a readonly file, a save as dialog will show to ask
// you to save new copy instead of replacing read-only one.
//bool CheckRight_SaveAsReadonly(const UCCmd& ucCmd)
//{
//	auto hWnd = GetActiveWindow();
//	auto hParent = GetParent(hWnd);
//	if (hParent == curCatSession.GetMainWnd())
//	{
//		// TODO: How to support other languages without "Save As" words
//		wchar_t szText[MAX_PATH];
//		GetWindowText(hWnd, szText, MAX_PATH);
//		if (_wcsicmp(szText, L"Save As") == 0)
//		{
//			LOG_DEBUG_FMT(L"Save As Dialog found @%p", hWnd);
//			CAT::DocumentPtr pDoc = curCatSession.GetActiveDocument();
//			if (pDoc == NULL)
//			{
//				return true;
//			}
//
//			wstring wsFileFullName;
//			if (!CCatSession::GetDocFileName(pDoc, wsFileFullName)
//				|| !curCatSession.IsNxlFile(wsFileFullName))
//			{
//				return true;
//			}
//
//			if (!curCatSession.CheckRight(wsFileFullName, RMX_RIGHT_EDIT, false))
//			{
//				ALERT_ERROR(IDS_ERROR_DENY_ALWAYS, wsFileFullName.c_str());
//				return false;
//			}
//		}
//	}
//
//	return true;
//}

// This is called for below 2 cases.  
// 1. Close event: Ask to save change, but cannot catch pre-save and cancel in time. We do post-action to delete new saved file. 
// 2. Save parent doc: Modified dependent doc will be saved together. We should allow to save oepration processed, 
// but deny dependent doc separately. We do post-action to delete new saved one.
bool CheckRight_SaveChangesToFile(const UCCmd& ucCmd)
{
	if (ucCmd.eCtx == eContext_File)
	{
		if (!RMX_IsProtectedFile(ucCmd.wsCtxFileFullName.c_str()))
			return true; // Ignore if it's not valid nxl.

		// Check if file permission is granted
		if (!curCatSession.CheckRight(ucCmd.wsCtxFileFullName, ucCmd.dwAccessRight, false))
		{
			CString csMsg;
			if (csMsg.LoadString((HINSTANCE)&__ImageBase, IDS_WARN_DISCARDCHANGES))
			{
				// Show message in rmd tray instead of alert, in case multiple files processed in sequence
				// and alert is very anoying 
				RMX_NotifyMessage(ucCmd.wsCtxFileFullName.c_str(), (LPCWSTR)csMsg);
				LOG_ERROR((LPCWSTR)csMsg);
			}
			return false;
		}
	}

	return true;
}

bool CheckRight_CopyOneFileToDir(const UCCmd& ucCmd)
{
	if (ucCmd.eCtx == eContext_File)
	{
		if (RMX_IsProtectedFile(ucCmd.wsCtxFileFullName.c_str()))
		{
			CString csMsg;
			if (csMsg.LoadString((HINSTANCE)&__ImageBase, IDS_ERROR_DENY_COPYONEFILETODIR))
			{
				// Show message in rmd tray instead of alert, in case multiple files processed in sequence
				// and alert is very anoying 
				RMX_NotifyMessage(ucCmd.wsCtxFileFullName.c_str(), (LPCWSTR)csMsg);
				LOG_ERROR((LPCWSTR)csMsg);
			}
			return false;
		}
	}

	return true;
}

bool CheckRight_NewFrom(const UCCmd& ucCmd)
{
	auto checkFunc = [&](const wstring& f) -> bool {
		if (!curCatSession.IsNxlFile(f))
			return true;

		// Check if file permission is granted
		if (!curCatSession.CheckRight(f, ucCmd.dwAccessRight, true))
		{
			return false;
		}

		return true;
	};

	if (ucCmd.eCtx == eContext_Files) // Multiple files
	{
		wstring msg;
		for (const auto& file : ucCmd.ctxFiles)
		{
			if (!checkFunc(file))
			{
				msg += L"\n" + file;
			}
		}
		if (!msg.empty())
		{
			wchar_t szRightName[50];
			RMX_GetRightName(ucCmd.dwAccessRight, szRightName);
			ALERT_ERROR(IDS_ERROR_DENY_NEW_FROM, szRightName, msg.c_str());
			return false;
		}
	}
	else if (ucCmd.eCtx == eContext_File) // Single file
	{
		if (!checkFunc(ucCmd.wsCtxFileFullName))
		{
			wchar_t szRightName[50];
			RMX_GetRightName(ucCmd.dwAccessRight, szRightName);
			ALERT_ERROR(IDS_ERROR_DENY_NEW_FROM, szRightName, ucCmd.wsCtxFileFullName.c_str());
			return false;
		}
	}

	return true;
}

bool CheckRight_CaptureOnActivateWindow(const UCCmd& ucCmd)
{
	auto ret = curUCControl.CheckRight(ucCmd.bCheckReferences, [&](CAT::DocumentPtr pDoc, bool bRefDoc) -> bool {
		wstring wsFileFullName;
		if (!CCatSession::GetDocFileName(pDoc, wsFileFullName) ||
			!curCatSession.IsNxlFile(wsFileFullName))
			return true; // Ignore if it's not valid nxl.

		// Check if file permission is granted
		if (!curCatSession.CheckRight(wsFileFullName, ucCmd.dwAccessRight))
		{
			wchar_t szRightName[50];
			RMX_GetRightName(ucCmd.dwAccessRight, szRightName);
			UINT nMsgID = bRefDoc ? IDS_ERROR_DENY_OP_DEP : IDS_ERROR_DENY_OP;

			wsFileFullName += L"\nCapture dialog will be closed.";
			ALERT_ERROR(nMsgID, szRightName, wsFileFullName.c_str());

			return false;
		}

		return true;
	});

	return (ret == CatRMX::eRightGrant);
}

void CRMXUsageControl::Start()
{
	LOG_INFO(L"Starting usage control...");
	
	BEGIN_CMD_MAP
		// Save
		CMD_ENTRY_DEFAULT(CatRMX::CMD_SAVECMD, RMX_RIGHT_EDIT, false),
		CMD_ENTRY(CatRMX::CMD_CATSAVEALL, RMX_RIGHT_EDIT, false, SaveAll),
		CMD_ENTRY(CatRMX::CMD_SAVEASCMD, 0, true, Disable),
		CMD_ENTRY(CatRMX::CMD_CATSAVEALLAS, 0, false, DisableOnNxlOpened),
		CMD_ENTRY(CatRMX::CMD_PRINT, RMX_RIGHT_PRINT, true, Print),
		CMD_ENTRY(CatRMX::CMD_DRWPRINT, RMX_RIGHT_PRINT, true, Print),
		CMD_ENTRY(CatRMX::CMD_QUICKPRINT, RMX_RIGHT_PRINT, true, Print),
		CMD_ENTRY_DEFAULT(CatRMX::CMD_CUT, RMX_RIGHT_DECRYPT, true),
		CMD_ENTRY_DEFAULT(CatRMX::CMD_DRWCUT, RMX_RIGHT_DECRYPT, true),
		CMD_ENTRY_DEFAULT(CatRMX::CMD_COPY, RMX_RIGHT_DECRYPT, true),
		CMD_ENTRY(CatRMX::CMD_SCREENCAPTURE, RMX_RIGHT_DECRYPT, true, ScreenCapture),
		CMD_ENTRY_DEFAULT(CatRMX::CMD_PRODUCTTOPART, RMX_RIGHT_DECRYPT, true),
		CMD_ENTRY_DEFAULT(CatRMX::CMD_CAPTURE, RMX_RIGHT_DECRYPT, true),
		CMD_ENTRY_DEFAULT(CatRMX::CMD_VIDEO, RMX_RIGHT_DECRYPT, true),
		CMD_ENTRY(CatRMX::CMD_PCS2CGR, 0, true, Disable),
		CMD_ENTRY(CatRMX::CMD_PCS2PPR, 0, true, Disable),
		CMD_ENTRY(CatRMX::CMD_SAVECHANGESTOFILE, RMX_RIGHT_EDIT, false, SaveChangesToFile),
		CMD_ENTRY(CatRMX::CMD_COLLABSHARESNAPSHOT, 0, true, Disable),
		CMD_ENTRY(CatRMX::CMD_COLLABSHAREDOC, 0, false, DisableOnRMXLoaded),
		CMD_ENTRY(CatRMX::CMD_COPYONEFILETODIR, 0, false, CopyOneFileToDir),
		CMD_ENTRY(CatRMX::CMD_NEWFROM, RMX_RIGHT_DECRYPT, false, NewFrom),
		CMD_ENTRY(CatRMX::CMD_CAPTUREONACTIVATEWINDOW, RMX_RIGHT_DECRYPT, true, CaptureOnActivateWindow)
	END_CMD_MAP

	// Supress the deny alert in UI
	m_cmdMap[CatRMX::CMD_SCREENCAPTURE].bAlert = false;

	// Register event listeners
	curEventMgr.AddListener((CCatEventLisener*)this, eCatNotify_BeforeCommand);
}

void CRMXUsageControl::Stop()
{
	LOG_INFO(L"Stopping usage control...");
	m_cmdMap.clear();
}

BOOL CRMXUsageControl::CheckCommandRight(const wstring& wsCmdName)
{
	return CheckCommandRightByFile(wsCmdName, L"");
}

BOOL CRMXUsageControl::CheckCommandRightByFile(const wstring& wsCmdName, const wstring& wsFileFullName)
{
	std::set<wstring, ICaseKeyLess> files = { wsFileFullName };
	return CheckCommandRightByFiles(wsCmdName, files);
}

BOOL CRMXUsageControl::CheckCommandRightByFiles(const wstring& wsCmdName, const std::set<wstring, ICaseKeyLess>& files)
{
	bool bRet = true;
	auto itCmd = m_cmdMap.find(wsCmdName);
	if (itCmd != m_cmdMap.end())
	{
		LOG_ENTER_CALL(wsCmdName.c_str());

		// Note: Don't access cmd reference but a new copy
		// to avoid context information resetting.
		auto cmd = itCmd->second;
		if (files.size() == 1)
		{
			cmd.eCtx = eContext_File;
			cmd.wsCtxFileFullName = (*files.begin());
		}
		else if (files.size() > 1)
		{
			cmd.eCtx = eContext_Files;
			cmd.ctxFiles = files;
		}
		else
			cmd.eCtx = eContext_Current;

		bRet = itCmd->second.pCB(cmd);

		// Special case the first command is vetoed, the nest command will be still triggered
		// Need to veto it as well. e.g.: Exportsetting, preview
		// Once the first command is called, the nest command will not be notified.
		if (!bRet)
		{
			LOG_ERROR(L"Permission not granted on " << wsCmdName.c_str());
		}
		else
		{
			LOG_INFO(L"Permission granted on " << wsCmdName.c_str());
		}
	}
	return bRet;
}

void CRMXUsageControl::OnBeforeCommand(LPCatCommandNotify pParam)
{
	if (!pParam->ctxFiles.empty())
	{
		if (!curUCControl.CheckCommandRightByFiles(pParam->wsCmdName, pParam->ctxFiles))
		{
			pParam->bCancel = true;
		}
	}
	else if (!curUCControl.CheckCommandRightByFile(pParam->wsCmdName, pParam->wsFileFullName))
	{
		pParam->bCancel = true;
	}
}

//void CRMXUsageControl::OnAfterCommand(LPCatCommandNotify pParam)
//{
//	if (StringHlp::ICompare(CatRMX::CMD_FILELOAD, pParam->wsCmdName.c_str()))
//	{
//		curUCControl.CheckCommandRightByFile(pParam->wsCmdName, pParam->wsFileFullName);
//	}
//}
