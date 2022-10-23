#include "stdafx.h"
#include "CatHooks.h"
#include <madCHook.h>
#include "..\common\HookDefs.h"
#include "CatSession.h"
#include "CatLib.h"
#include "CatEventMgr.h"
#include "RMXUsageControl.h"
#include "LibRMX.h"

//
// void l_CATInteractiveApplication::OnIdle(int)
// App loaded event
// 
typedef void(*pfCATInteractiveApplication_OnIdle)(CPP_PTR p, int a);
DEFINE_HOOK_CALLBACK2(CATInteractiveApplication_OnIdle, "?OnIdle@l_CATInteractiveApplication@@QEAAXH@Z")
void CATInteractiveApplication_OnIdle_hooked(CPP_PTR p, int a)
{
	CATInteractiveApplication_OnIdle_next(p, a);

	if (!curCatSession.IsConnected() )
	{
		curCatSession.Start();
	}
}

#pragma region Notification

typedef CPP_PTR(*pfCATCommandHeader_StartCommand) (CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATCommandHeader_StartCommand, "?StartCommand@CATCommandHeader@@UEAAPEAVCATCommand@@XZ")
static CPP_PTR CATCommandHeader_StartCommand_hooked(CPP_PTR pThis)
{
	static set<wstring, ICaseKeyLess> cmdsToNotify = {
			CatRMX::CMD_CATSAVEALLAS,
			CatRMX::CMD_CATSAVEALL,
			CatRMX::CMD_CAPTURE, CatRMX::CMD_VIDEO,
			CatRMX::CMD_PCS2CGR, CatRMX::CMD_PCS2PPR,
			CatRMX::CMD_QUICKPRINT, CatRMX::CMD_PRINT,
			CatRMX::CMD_COPY, CatRMX::CMD_CUT,
			CatRMX::CMD_PRODUCTTOPART,
			CatRMX::CMD_SAVECMD, CatRMX::CMD_SAVEASCMD,
			CatRMX::CMD_DRWCUT, CatRMX::CMD_DRWPRINT,
			CatRMX::CMD_COLLABSHAREDOC, CatRMX::CMD_COLLABSHARESNAPSHOT
	};

	wstring wsCmdName;
	try
	{
		CPP_PTR pName = CATCommandHeader_GetClass(pThis);
		const char* szName = CATString_ConvertToChar(pName);
		if (szName != NULL)
		{
			wsCmdName = StringHlp::s2ws(szName);
			LOG_DEBUG_FMT(L"Entering CATCommandHeader_StartCommand_hooked(%s)", wsCmdName.c_str());
			if (cmdsToNotify.find(wsCmdName) != cmdsToNotify.end())
			{
				CatCommandNotify notifParam;
				notifParam.wsCmdName = wsCmdName;
				LOG_DEBUG_FMT(L"StartCommand(%s)", wsCmdName.c_str());

				curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));
				// Return to cancel the command 
				if (notifParam.bCancel)
				{
					LOG_ERROR_FMT(L"Command Cancelled (%s)", wsCmdName.c_str());
					return NULL;
				}
			}
		}
	}
	catch (...)
	{
		LOG_ERROR("Exception occurs in CATCommandHeader_StartCommand_hooked");
	}

	return CATCommandHeader_StartCommand_next(pThis);
}

//
// void CATMMediaVideoDialog::cb_Record(class CATCommand *,class CATNotification *,void *)
//
typedef void(*pfCATMMediaVideoDialog_cb_Record) (CPP_PTR, CPP_PTR, CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATMMediaVideoDialog_cb_Record, "?cb_Record@CATMMediaVideoDialog@@AEAAXPEAVCATCommand@@PEAVCATNotification@@PEAX@Z")
static void CATMMediaVideoDialog_cb_Record_hooked(CPP_PTR pThis, CPP_PTR pCmd, CPP_PTR pNotify, CPP_PTR p)
{
	LOG_ENTER_CALL(L"");
	CatCommandNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_VIDEO;

	curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));
	// Return to cancel the command 
	if (notifParam.bCancel)
	{
		LOG_ERROR(L"CATMMediaVideoDialog_cb_Record Cancelled");
		return;
	}

	CATMMediaVideoDialog_cb_Record_next(pThis, pCmd, pNotify, p);
}

#pragma endregion Notification

#pragma region UI Update
//
// void CATCloseBox::CloseAndRemove(void)
// 
typedef void(*pfCATCloseBox_CloseAndRemove)(CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATCloseBox_CloseAndRemove, "?CloseAndRemove@CATCloseBox@@IEAAXXZ")
void CATCloseBox_CloseAndRemove_hooked(CPP_PTR pThis)
{
	LOG_ENTER_CALL(L"");
	RMX_SCOPE_GUARD_ENTER(&CATCloseBox_CloseAndRemoveInHook);

	CATCloseBox_CloseAndRemove_next(pThis);

	CatFrmNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_FRMCLOSEBOX;
	notifParam.pFrmLayout = NULL;
	curEventMgr.Notify(eCatNotify_FrmLayoutChanged, (LPCatNotify)(&notifParam));
}

//
// void CATFrmLayout::ActivateWindow(class CATFrmWindow*)
// CATApplicationFrame.dll
typedef void(*pfCATFrmLayout_ActivateWindow)(CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_ActivateWindow, "?ActivateWindow@CATFrmLayout@@AEAAXPEAVCATFrmWindow@@@Z")
void CATFrmLayout_ActivateWindow_hooked(CPP_PTR pThis, CPP_PTR pWnd)
{
	LOG_ENTER_CALL(L"");

	CATFrmLayout_ActivateWindow_next(pThis, pWnd);

	// Avoid nested check happens when closing a doc
	if (CATCloseBox_CloseAndRemoveInHook) return;

	CatFrmNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_ACTIVATEWINDOW;
	notifParam.pFrmLayout = pThis;
	curEventMgr.Notify(eCatNotify_FrmLayoutChanged, (LPCatNotify)(&notifParam));

	// Workaround to close capture dialog if it's showing
	HWND hCaptureDlg = NULL;
	const auto& vecTopWnds = RMXUtils::FindCurrentProcessMainWindow();
	for (const auto& hWnd : vecTopWnds)
	{
		// TODO: How to support other language  
		wchar_t szText[MAX_PATH];
		GetWindowText(hWnd, szText, MAX_PATH);
		if (_wcsicmp(szText, L"Capture") == 0)
		{
			hCaptureDlg = hWnd;
			break;
		}
	}

	if (hCaptureDlg != NULL)
	{
		auto pFrmWnd = CATFrmLayout_GetCurrentWindow(pThis);
		if (pFrmWnd)
		{
			auto pEditor = CATFrmWindow_GetEditor(pFrmWnd);
			if (pEditor)
			{
				auto pDoc = CATFrmEditor_GetDocument(pEditor);
				if (pDoc != NULL)
				{
					char* szStorageName = NULL;
					szStorageName = CATDocument_storageName(pDoc);
					if (szStorageName)
					{
						CatCommandNotify notifParam;
						notifParam.wsCmdName = CatRMX::CMD_CAPTUREONACTIVATEWINDOW;
						//notifParam.wsFileFullName = StringHlp::s2ws(szStorageName);
						curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));
						// Return to cancel the command 
						if (notifParam.bCancel)
						{
							LOG_ERROR(L"Closing capture dialog...");
							PostMessage(hCaptureDlg, WM_CLOSE, NULL, NULL);
						}
					}
				}
			}
		}
		
	}
}

static void notifyFrmLayoutChanged(CPP_PTR pFrmLayout)
{
	CatFrmNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_FRMMDICHANGE;
	notifParam.pFrmLayout = pFrmLayout;
	curEventMgr.Notify(eCatNotify_FrmLayoutChanged, (LPCatNotify)(&notifParam));
}

//
//void CATFrmLayout::FrmMDICascade(int)
//
typedef void(*pfCATFrmLayout_FrmMDICascade)(CPP_PTR, int);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDICascade, "?FrmMDICascade@CATFrmLayout@@QEAAXH@Z")
void CATFrmLayout_FrmMDICascade_hooked(CPP_PTR pThis, int n)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDICascade_next(pThis, n);

	notifyFrmLayoutChanged(pThis);
}

//
// void CATFrmLayout::FrmMDIClose(class CATFrmWindow*)
//
typedef void(*pfCATFrmLayout_FrmMDIClose)(CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDIClose, "?FrmMDIClose@CATFrmLayout@@QEAAXPEAVCATFrmWindow@@@Z")
void CATFrmLayout_FrmMDIClose_hooked(CPP_PTR pThis, CPP_PTR pWnd)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDIClose_next(pThis, pWnd);
	
	notifyFrmLayoutChanged(pThis);
}

//
//void CATFrmLayout::FrmMDIIconArrange(void)
//
typedef void(*pfCATFrmLayout_FrmMDIIconArrange)(CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDIIconArrange, "?FrmMDIIconArrange@CATFrmLayout@@QEAAXXZ")
void CATFrmLayout_FrmMDIIconArrange_hooked(CPP_PTR pThis)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDIIconArrange_next(pThis);

	notifyFrmLayoutChanged(pThis);
}

//
//void CATFrmLayout::FrmMDIMaximize(class CATFrmWindow*)
//
typedef void(*pfCATFrmLayout_FrmMDIMaximize)(CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDIMaximize, "?FrmMDIMaximize@CATFrmLayout@@QEAAXPEAVCATFrmWindow@@@Z")
void CATFrmLayout_FrmMDIMaximize_hooked(CPP_PTR pThis, CPP_PTR pWnd)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDIMaximize_next(pThis, pWnd);

	notifyFrmLayoutChanged(pThis);
}

//
//void CATFrmLayout::FrmMDIMinimize(class CATFrmWindow*)
//
typedef void(*pfCATFrmLayout_FrmMDIMinimize)(CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDIMinimize, "?FrmMDIMinimize@CATFrmLayout@@QEAAXPEAVCATFrmWindow@@@Z")
void CATFrmLayout_FrmMDIMinimize_hooked(CPP_PTR pThis, CPP_PTR pWnd)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDIMinimize_next(pThis, pWnd);

	notifyFrmLayoutChanged(pThis);
}

//
//void CATFrmLayout::FrmMDIRestore(class CATFrmWindow*)
//
typedef void(*pfCATFrmLayout_FrmMDIRestore)(CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDIRestore, "?FrmMDIRestore@CATFrmLayout@@QEAAXPEAVCATFrmWindow@@@Z")
void CATFrmLayout_FrmMDIRestore_hooked(CPP_PTR pThis, CPP_PTR pWnd)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDIRestore_next(pThis, pWnd);

	notifyFrmLayoutChanged(pThis);
}

//
//void CATFrmLayout::FrmMDITile(int)
//
typedef void(*pfCATFrmLayout_FrmMDITile)(CPP_PTR, int);
DEFINE_HOOK_CALLBACK2(CATFrmLayout_FrmMDITile, "?FrmMDITile@CATFrmLayout@@QEAAXH@Z")
void CATFrmLayout_FrmMDITile_hooked(CPP_PTR pThis, int n)
{
	LOG_ENTER_CALL(L"");
	CATFrmLayout_FrmMDITile_next(pThis, n);

	notifyFrmLayoutChanged(pThis);
}

//
// void CATViewer::SetFullScreen(int)
// 
typedef void(*pfCATViewer_SetFullScreen)(CPP_PTR, int);
DEFINE_HOOK_CALLBACK2(CATViewer_SetFullScreen, "?SetFullScreen@CATViewer@@UEAAXH@Z")
void CATViewer_SetFullScreen_hooked(CPP_PTR pThis, int n)
{
	LOG_ENTER_CALL(to_wstring(n).c_str());

	CATViewer_SetFullScreen_next(pThis, n);

	curEventMgr.Notify(eCatNotify_FullScreenChanged, (LPCatNotify)(&n));
}

//
// int CATNodeExtension::UpdateChildren(void)
// 
typedef int(*pfCATNodeExtension_UpdateChildren) (CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATNodeExtension_UpdateChildren, "?UpdateChildren@CATNodeExtension@@QEAAHXZ")
int CATNodeExtension_UpdateChildren_hooked(CPP_PTR pThis)
{
	auto ret = CATNodeExtension_UpdateChildren_next(pThis);

	// Skip nested call
	if (CATNodeExtension_UpdateChildrenInHook)
		return ret;

	RMX_SCOPE_GUARD_ENTER(&CATNodeExtension_UpdateChildrenInHook);

	// Notify to update watermark/screen capture
	CatCommandNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_UPDATENODECHILDREN;
	curEventMgr.Notify(eCatNotify_AfterCommand, (LPCatNotify)(&notifParam));

	return ret;
}

#pragma endregion UI Update

#pragma region File 
//
// long CATStorageManager::EndOfLoadOperation(void)
// File open event
// 
typedef long(*pfCATStorageManager_EndOfLoadOperation) (CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATStorageManager_EndOfLoadOperation, "?EndOfLoadOperation@CATStorageManager@@QEAAJXZ")
long CATStorageManager_EndOfLoadOperation_hooked(CPP_PTR pThis)
{
	long ret = CATStorageManager_EndOfLoadOperation_next(pThis);

	try
	{
		// Add file to cache
		CPP_PTR pDoc = CATStorageManager_GetDocument(pThis);
		if (pDoc != NULL)
		{
			// Add newly loaded nxl file into cache
			char* szStorageName = NULL;
			szStorageName = CATDocument_storageName(pDoc);
			if (szStorageName != NULL)
			{
				CatCommandNotify notifParam;
				notifParam.wsCmdName = CatRMX::CMD_FILELOAD;
				notifParam.wsFileFullName = StringHlp::s2ws(szStorageName);

				curEventMgr.Notify(eCatNotify_AfterCommand, (LPCatNotify)(&notifParam));
			}
		}
	}
	catch (...)
	{
		LOG_ERROR("Exception occurs in EndOfLoadOperation_hook");
	}

	return ret;
}

//
// void CATPrintImageViewerDialog::cb_ImmediatePrint(class CATCommand*, class CATNotification*, void*)
// Print in Preview dialog
//
typedef void(*pfCATPrintImageViewerDialog_cb_ImmediatePrint)(CPP_PTR, CPP_PTR, CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATPrintImageViewerDialog_cb_ImmediatePrint, "?cb_ImmediatePrint@CATPrintImageViewerDialog@@AEAAXPEAVCATCommand@@PEAVCATNotification@@PEAX@Z")
void CATPrintImageViewerDialog_cb_ImmediatePrint_hooked(CPP_PTR pThis, CPP_PTR pCmd, CPP_PTR pNotif, CPP_PTR p)
{
	LOG_ENTER_CALL(L"");

	CatCommandNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_PRINT;
	curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));

	// Return to cancel the command 
	if (notifParam.bCancel)
	{
		LOG_ERROR(L"Abort. Print not allowed.");
		return;
	}

	CATPrintImageViewerDialog_cb_ImmediatePrint_next(pThis, pCmd, pNotif, p);
}

//
// long CATStorageManager::EndOfSaveOperation(int)
// File save event
//
typedef long(*pfCATStorageManager_EndOfSaveOperation) (CPP_PTR, int);
DEFINE_HOOK_CALLBACK2(CATStorageManager_EndOfSaveOperation, "?EndOfSaveOperation@CATStorageManager@@QEAAJH@Z")
long CATStorageManager_EndOfSaveOperation_hooked(CPP_PTR pThis, int n)
{
	LOG_ENTER_CALL(L"");

	wstring wsFileOrigin;
	try
	{
		CPP_PTR pDoc = CATStorageManager_GetDocument(pThis);
		if (pDoc != NULL)
		{
			char* szStorageName = NULL;
			szStorageName = CATDocument_storageName(pDoc);
			wsFileOrigin = StringHlp::s2ws(szStorageName);
		}
	}
	catch (...)
	{
		LOG_ERROR("Exception occurs in EndOfSaveOperation_hook");
	}

	/*CatSaveNotify notifParam;
	notifParam.wsFileFullName = wsFileOrigin;
	curEventMgr.Notify(eCatNotify_AfterSave, (LPCatNotify)(&notifParam));*/

	long ret = CATStorageManager_EndOfSaveOperation_next(pThis, n);

	if (!wsFileOrigin.empty())
	{
		CatSaveNotify notifParam;
		notifParam.wsFileFullName = wsFileOrigin;
		curEventMgr.Notify(eCatNotify_AfterSave, (LPCatNotify)(&notifParam));

		if (notifParam.bPostSaveDone)
		{
			try
			{
				// Sync up save time
				CPP_PTR pDoc = CATStorageManager_GetDocument(pThis);
				if (pDoc != NULL)
				{

					LOG_INFO(L"CATDocument_SetSaveTime called");
					CATDocument_SetSaveTime(pDoc);
				}
			}
			catch (...)
			{
				LOG_ERROR("Exception occurs in CATDocument_SetSaveTime");
			}
		}
	}
	
	return ret;
}

class NewFromLisener : public  CCatEventLisener
{
public:
	NewFromLisener() {
		LOG_DEBUG(L"NewFromLisener starting");

		// Take a snapshot of current new docs in session
		std::set<wstring, ICaseKeyLess> oldNewDocs;
		curCatSession.WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
			wstring wsFileFullName;
			// Note: CPathEx::IsFile(L"Product1.CATProduct") doesn't work well. So check path seperator to determine if this given file name
			// is a path or not.
			if (CCatSession::GetDocFileName(pDoc, wsFileFullName) && (wcsstr(wsFileFullName.c_str(), L"\\") == NULL))
			{
				m_oldNewDocs.insert(wsFileFullName);
			}

			return false;
		});
		// Lisening the file load notification, and monitor the newly loaded file
		// which will be used to determine the permission.
		curEventMgr.AddListener((CCatEventLisener*)this, eCatNotify_AfterCommand);
	}

	~NewFromLisener() {
		curEventMgr.RemoveListener((CCatEventLisener*)this, eCatNotify_AfterCommand);
		LOG_DEBUG(L"NewFromLisener stopping");
	}
	
	// RMX Event handler
	void OnAfterCommand(LPCatCommandNotify pParam) {
		if (StringHlp::ICompare(pParam->wsCmdName.c_str(), CatRMX::CMD_FILELOAD))
		{
			// Cache the newly loaded file
			LOG_DEBUG_FMT(L"NewFromLisener: %s loaded ", pParam->wsFileFullName.c_str());
			m_loadedFiles.insert(pParam->wsFileFullName);
		}
	}

	bool PostAction()
	{
		if (m_loadedFiles.empty())
			return false;

		CatCommandNotify notifParam;
		notifParam.wsCmdName = CatRMX::CMD_NEWFROM;
		notifParam.ctxFiles = m_loadedFiles;
		curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));
		// Return to cancel the command 
		if (notifParam.bCancel)
		{
			// Grab the new from documents, based on the old snapshot of new documents in session.
			std::vector<CAT::DocumentPtr> newDocs;
			curCatSession.WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
				wstring wsFileFullName;
				if (CCatSession::GetDocFileName(pDoc, wsFileFullName) &&
					(wcsstr(wsFileFullName.c_str(), L"\\") == NULL) && (m_oldNewDocs.find(wsFileFullName) == m_oldNewDocs.end()))
				{
					LOG_WARN_FMT(L"NewFromLisener: %s will be closed", wsFileFullName.c_str());
					newDocs.push_back(pDoc);
				}

				return false;
			});

			// Close all new docs as we deny this new from command
			// Note: If multiple files are selected, will trigger this instead of newfrom2 below. Returnning -1 doesn't terminate the call sucessfully.
			// So we do post action here to close new docs w/o saving below.
			for (auto d : newDocs)
				d->Close();

			LOG_ERROR(L"NewFrom cancelled");
			
			return true;
		}

		return false;
	}

private:
	std::set<wstring, ICaseKeyLess> m_loadedFiles;
	std::set<wstring, ICaseKeyLess> m_oldNewDocs;
};

//
// long CATDocumentServices::NewFrom(class CATListPtrCATDocument *,class CATListValCATUnicodeString const * &,class CATListValCATUnicodeString const *)
// 
typedef long(*pfCATDocumentServices_NewFrom)(CPP_PTR, CPP_PTR, CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CATDocumentServices_NewFrom, "?NewFrom@CATDocumentServices@@SAJPEAVCATListPtrCATDocument@@AEAPEBVCATListValCATUnicodeString@@PEBV3@@Z")
long CATDocumentServices_NewFrom_hooked(CPP_PTR pThis, CPP_PTR pDocs, CPP_PTR pStrs1, CPP_PTR pStrs2)
{
	LOG_ENTER_CALL(L"");
	// regster a temp lisener to track the newly loaded docs during this call
	// If any nxl file is loaded without extract permission, return -1 to terminate the call
	NewFromLisener lisner;

	auto ret = CATDocumentServices_NewFrom_next(pThis, pDocs, pStrs1, pStrs2);

	if (lisner.PostAction())
		return -1; // Cannot terminate the call by returning -1. Addtional clean up action is done in PostAction call.

	return ret;
}

//
// long CATDocumentServices::NewFrom(class CATListValCATUnicodeString *,class CATListPtrCATDocument *,short) 
// 
typedef long(*pfCATDocumentServices_NewFrom2)(CPP_PTR, CPP_PTR, CPP_PTR, short);
DEFINE_HOOK_CALLBACK2(CATDocumentServices_NewFrom2, "?NewFrom@CATDocumentServices@@SAJPEAVCATListValCATUnicodeString@@PEAVCATListPtrCATDocument@@F@Z")
long CATDocumentServices_NewFrom2_hooked(CPP_PTR pThis, CPP_PTR pStrs, CPP_PTR pDocs, short n)
{
	LOG_ENTER_CALL(to_wstring(n).c_str());
	// regster a temp lisener to track the newly loaded docs during this call
	// If any nxl file is loaded without extract permission, return -1 to terminate the call
	NewFromLisener lisner;
	
	auto ret = CATDocumentServices_NewFrom2_next(pThis, pStrs, pDocs, n);

	if (lisner.PostAction())
		return -1; // Cannot terminate the call by returning -1. Addtional clean up action is done in PostAction call.
	
	return ret;
}

//
// long CopyOneFileToDir(class CATUnicodeString const &,class CATUnicodeString const &,class CATUnicodeString const &,class CATUnicodeString const &)
// 
typedef long(*pfCopyOneFileToDir)(CPP_PTR, CPP_PTR, CPP_PTR, CPP_PTR);
DEFINE_HOOK_CALLBACK2(CopyOneFileToDir, "?CopyOneFileToDir@@YAJAEBVCATUnicodeString@@000@Z")
long CopyOneFileToDir_hooked(CPP_PTR dirFrom, CPP_PTR fileFrom, CPP_PTR dirTo, CPP_PTR fileTo)
{
	const char* sz1 = CATUnicodeString_ConvertToChar(dirFrom);
	const char* sz2 = CATUnicodeString_ConvertToChar(fileFrom);
	CPathEx from(StringHlp::s2ws(sz1).c_str());
	from /= StringHlp::s2ws(sz2).c_str();

	CatCommandNotify notifParam;
	notifParam.wsCmdName = CatRMX::CMD_COPYONEFILETODIR;
	notifParam.wsFileFullName = from.ToString();
	curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));
	// Return to cancel the command 
	if (notifParam.bCancel)
	{
		LOG_ERROR_FMT(L"CopyOneFileToDir Cancelled on the file %s", from.c_str());
		return -1;
	}

	return CopyOneFileToDir_next(dirFrom, fileFrom, dirTo, fileTo);
}

//// long CATIAProductsImpl::AddComponentsFromFiles(struct tagSAFEARRAY const &,unsigned short * const &)
//// ?AddComponentsFromFiles@CATIAProductsImpl@@UEAAJAEBUtagSAFEARRAY@@AEBQEAG@Z
//typedef long(*pfCATIAProductsImpl_AddComponentsFromFiles) (CPP_PTR, CPP_PTR, unsigned short* const&);
//DEFINE_HOOK_CALLBACK2(CATIAProductsImpl_AddComponentsFromFiles, "?AddComponentsFromFiles@CATIAProductsImpl@@UEAAJAEBUtagSAFEARRAY@@AEBQEAG@Z")
//long CATIAProductsImpl_AddComponentsFromFiles_hooked(CPP_PTR pThis, CPP_PTR safeArr, unsigned short* const& arrSize)
//{
//	LOG_ENTER_CALL(L"");
//	auto ret = CATIAProductsImpl_AddComponentsFromFiles_next(pThis, safeArr, arrSize);
//
//	CatCommandNotify notifParam;
//	notifParam.wsCmdName = CatRMX::CMD_ADDCOMPFROMFILES;
//	curEventMgr.Notify(eCatNotify_AfterCommand, (LPCatNotify)(&notifParam));
//
//	return ret;
//}

//
// MoveFileW
// File Save event
//
typedef
BOOL(WINAPI* pfMoveFileW)(
	__in     LPCWSTR lpExistingFileName,
	__in_opt LPCWSTR lpNewFileName
	);

DEFINE_HOOK_CALLBACK(MoveFileW)
BOOL MoveFileW_hooked(
	__in     LPCWSTR lpExistingFileName,
	__in_opt LPCWSTR lpNewFileName)
{
	if (MoveFileWInHook)
		return MoveFileW_next(lpExistingFileName, lpNewFileName);

	RMX_SCOPE_GUARD_ENTER(&MoveFileWInHook);

	LOG_DEBUG_FMT(L"MoveFileW_hooked triggered (lpExistingFileName=%s, lpNewFileName=%s):", lpExistingFileName, lpNewFileName);

	// Fix Bug 71292 : When saving changes to the file, the routine steps as following:
	// 1: The existing file a.CATPart will be renamed to .[guid].a.CATPart.bck file as backup. 
	// 2. Genenate new file with guide file name(e.g.:.e46cd029420000da017d6260010000), all changes will be written into it.
	// 3. After writing file,.e46cd029420000da017d6260010000 will be renamed back to original file name: a.catpart
	// 4. Delete .bck file
	// Note: Any exception occurs in step3, bck file will be left as backup file. 
	// Fix: 
	// In Step1, trusted process will copy encrypted file into bck file. (Data Leaking).  
	// It will cause the nxl file left in disk, may affect step3 with "ACCESS DENY" error.
	CPathEx newFile(lpNewFileName);
	CPathEx oldFile(lpExistingFileName);
	if ((_wcsicmp(newFile.GetExtension().c_str(), L".bck") == 0)
		&& curCatSession.IsNxlFile(wstring(lpExistingFileName))
		&& (_wcsicmp(newFile.GetParentPath().c_str(), oldFile.GetParentPath().c_str()) == 0))
	{
		BOOL ret = RMX_CopyNxlFile(lpExistingFileName, lpNewFileName, true);
		oldFile += L".nxl";
		if (CPathEx::IsFile(oldFile.c_str()))
		{
			LOG_ERROR(L"MoveFileW_hooked failed, nxl file still exists");
		}
		if (CPathEx::IsFile(lpExistingFileName))
		{
			LOG_ERROR(L"MoveFileW_hooked failed, plain file still exists");
		}
		
		return ret;
	}
	if (curCatSession.IsNxlFile(wstring(lpNewFileName)))
	{
		// This is called for below 2 cases. The general fix is to roll back bck file, but delete newly saved file
		// 1. Close event: Ask to save change, but cannot catch pre-save and cancel in time. We do post-action here.
		// 2. Save parent doc: Modified dependent doc will be saved together. We should allow to save oepration processed, 
		// but deny dependent doc separately. We do post-action here.
		// Backup file: .e46cd029420000da017d6260010000.E_Motor.CATPart.bck
		// Newly saved file: .e46cd029420000da017d6260010000
		wstring bckFile(lpExistingFileName);
		bckFile = bckFile + L"." + newFile.GetFileNameWithExt() + L".bck.nxl";
		// if target file is protected orginal
		if (CPathEx::IsFile(bckFile))
		{
			CatCommandNotify notifParam;
			notifParam.wsCmdName = CatRMX::CMD_SAVECHANGESTOFILE;
			notifParam.wsFileFullName = bckFile;
			curEventMgr.Notify(eCatNotify_BeforeCommand, (LPCatNotify)(&notifParam));

			// Return to cancel the command 
			if (notifParam.bCancel)
			{
				// Don't allow to move
				LOG_ERROR_FMT(L"MoveFileW_hooked: No Edit permission found. Restoring bck file %s", bckFile.c_str());
				RMX_CopyNxlFile(bckFile.c_str(), lpNewFileName, false);
				DeleteFile(lpExistingFileName);
				return TRUE;
			}
		}
	}
	BOOL ret = MoveFileW_next(lpExistingFileName, lpNewFileName);
	if (!ret)
	{
		LOG_ERROR_FMT(L"MoveFileW failed (error: %s)", (LPCTSTR)CSysErrorMessage(GetLastError()));
	}

	return ret;
}

#pragma endregion File

void CCatHooks::InstallHooks()
{
	HOOK_API("CATObjectModelerBase.dll", CATStorageManager_EndOfLoadOperation);
	HOOK_API("CATObjectModelerBase.dll", CATStorageManager_EndOfSaveOperation);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_ActivateWindow);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDICascade);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDIClose);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDIIconArrange);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDIMaximize);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDIMinimize);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDIRestore);
	HOOK_API("CATApplicationFrame.dll", CATFrmLayout_FrmMDITile);
	HOOK_API("CATApplicationFrame.dll", CATCommandHeader_StartCommand);
	HOOK_API("CATFileMenu.dll", CATCloseBox_CloseAndRemove);
	HOOK_API("CATFileMenu.dll", CopyOneFileToDir);
	HOOK_API("Kernel32.dll", MoveFileW);
	HOOK_API("CATPrint.dll", CATPrintImageViewerDialog_cb_ImmediatePrint);
	HOOK_API("CATVISUALIZATION.DLL", CATViewer_SetFullScreen);
	HOOK_API("CATObjectModelerNavigator.dll", CATNodeExtension_UpdateChildren);
	HOOK_API("CATObjectModelerBase.dll", CATDocumentServices_NewFrom);
	HOOK_API("CATObjectModelerBase.dll", CATDocumentServices_NewFrom2);
	HOOK_API("CATMMEDIAVIDEO.DLL", CATMMediaVideoDialog_cb_Record);

}

void CCatHooks::UninstallHooks()
{
	UNHOOK(CATInteractiveApplication_OnIdle);
	UNHOOK(CATStorageManager_EndOfLoadOperation);
	UNHOOK(CATStorageManager_EndOfSaveOperation);
	UNHOOK(CATFrmLayout_ActivateWindow);
	UNHOOK(CATFrmLayout_FrmMDICascade);
	UNHOOK(CATFrmLayout_FrmMDIClose);
	UNHOOK(CATFrmLayout_FrmMDIIconArrange);
	UNHOOK(CATFrmLayout_FrmMDIMaximize);
	UNHOOK(CATFrmLayout_FrmMDIMinimize);
	UNHOOK(CATFrmLayout_FrmMDIRestore);
	UNHOOK(CATFrmLayout_FrmMDITile);
	UNHOOK(CATPrintImageViewerDialog_cb_ImmediatePrint);
	UNHOOK(CATCloseBox_CloseAndRemove);
	UNHOOK(CopyOneFileToDir);
	UNHOOK(MoveFileW);
	UNHOOK(CATCommandHeader_StartCommand);
	UNHOOK(CATViewer_SetFullScreen);
	UNHOOK(CATNodeExtension_UpdateChildren);
	UNHOOK(CATDocumentServices_NewFrom);
	UNHOOK(CATDocumentServices_NewFrom2);
	UNHOOK(CATMMediaVideoDialog_cb_Record);
}

void CCatHooks::Start()
{
	InitializeMadCHook();
	SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);
	LOG_INFO(L"MadCHook initialized");

	HOOK_API("di0panv2.dll", CATInteractiveApplication_OnIdle);
}

void CCatHooks::Stop()
{
	FinalizeMadCHook();
	LOG_INFO(L"MadCHook Finalized");

	curCatSession.Stop();
}
