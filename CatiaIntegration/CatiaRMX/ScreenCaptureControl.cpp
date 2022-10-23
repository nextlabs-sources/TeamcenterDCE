#include "stdafx.h"
#include "ScreenCaptureControl.h"
#include "RMXUsageControl.h"
#include "..\common\HookDefs.h"
#include "CatLib.h"
#include "CatSession.h"

void CScreenCaptureControl::Start()
{
	LOG_INFO(L"Starting screen capture control...");

	// Register event listeners
	curEventMgr.AddListener((CCatEventLisener*)this,
		eCatNotify_FrmLayoutChanged | eCatNotify_FullScreenChanged | eCatNotify_AfterCommand);
}

void CScreenCaptureControl::Stop()
{
	LOG_INFO(L"Stopping screen capture control...");
}

void CScreenCaptureControl::OnAfterCommand(LPCatCommandNotify pParam)
{
	// For "Insert existing component" command, need to refresh screen capture permission 
	// if any protected doc is loaded
	// TODO: Performance issue as this command will be sent many times such as file open, 
	// file insert, delete component, etc.
	if (StringHlp::ICompare(pParam->wsCmdName.c_str(), CatRMX::CMD_UPDATENODECHILDREN))
	{
		bool bFullScreen = curCatSession.IsFullScreen();;
		// Call automation APIs for SDI mode
		if (ApplyToCurrent(bFullScreen))
			return;

		// Check all opening docs for MDI mode
		ApplyToOpenDocs();
	}
}

void CScreenCaptureControl::OnFrmLayoutChanged(LPCatFrmNotify pParam)
{
	if (pParam->pFrmLayout != NULL)
	{
		// Call internal hook functions for SDI mode
		if (ApplyToLayout(pParam->pFrmLayout))
			return;
	}
	else
	{
		// Call automation APIs for SDI mode
		if (ApplyToCurrent(false))
			return;
	}

	// Check all opening docs for MDI mode
	ApplyToOpenDocs();
}

void CScreenCaptureControl::OnFullScreenChanged(int nFullScreen)
{
	LOG_ENTER_CALL(L"");
	// Call automation APIs for SDI mode
	bool bFullScreen = (nFullScreen == 1) ? true : false;
	if (ApplyToCurrent(bFullScreen))
		return;

	// Check all opening docs for MDI mode
	ApplyToOpenDocs();
}

void CScreenCaptureControl::ApplyToOpenDocs()
{
	LOG_ENTER_CALL(L"");
	bool bRightGranted = true;
	// Check all openning docs but skip the files which are not displayed in window(minized window exclusive)
	if (curCatSession.WalkOpenningDocs([&](CAT::DocumentPtr pDoc) -> bool {
		wstring wsFileName;
		if (!CCatSession::GetDocFileName(pDoc, wsFileName))
			return false;

		LOG_DEBUG_FMT(L"Check screen capture right on openning file: %s", wsFileName.c_str());
		if (!curUCControl.CheckCommandRightByFile(CatRMX::CMD_SCREENCAPTURE, wsFileName))
		{
			return true; // Terminate loop of open docs
		}

		return false;
	}))
	{
		bRightGranted = false;
	}

	RMXUtils::DisableScreenCapture(curCatSession.GetMainWnd(), !bRightGranted);
}

// The rules:
// If the most recent activated window is maxmized, only check associated doc and its deps
// otherwise, check all openning docs which are displayed in window(excluding minized window).
bool CScreenCaptureControl::ApplyToLayout(const void* pFrmLayout)
{
	LOG_ENTER_CALL(L"");
	if (pFrmLayout == NULL) return false;

	bool bRightGranted = true;

	bool bCurrentMaximized = false;
	try
	{
		// Note: Windows state returned by Automation API is not accurate for FrmLayout hook event, 
		// so call internal methods as below:
		auto pFrmWnd = CATFrmLayout_GetCurrentWindow(pFrmLayout);
		if (pFrmWnd)
		{
			int iWndState = CATFrmWindow_GetWindowMDIState(pFrmWnd);
			if (iWndState == 2/*Maximized*/)
			{
				bCurrentMaximized = true;
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
							const auto& wsFileName = StringHlp::s2ws(szStorageName);
							LOG_DEBUG_FMT(L"Check screen capture right on active window(maxmized): %s", wsFileName.c_str());
							if (!curUCControl.CheckCommandRightByFile(CatRMX::CMD_SCREENCAPTURE, wsFileName))
							{
								bRightGranted = false;
							}
							else
							{
								// Continue to scan all child dependencies
								auto pDoc = curCatSession.FindDocument(wsFileName);
								if (pDoc)
								{
									if (curCatSession.TraverseLinks(pDoc, [&](CAT::DocumentPtr pLinkDoc) -> bool {
										wstring wsLinkFileName;
										if (!CCatSession::GetDocFileName(pLinkDoc, wsLinkFileName))
											return false;

										if (!curUCControl.CheckCommandRightByFile(CatRMX::CMD_SCREENCAPTURE, wsLinkFileName))
										{
											return true; // Terminate loop
										}

										return false;
										}))
									{
										bRightGranted = false;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		LOG_ERROR("Exception occurs in ApplyAntiScreenCapture");
	}

	if (bCurrentMaximized)
	{
		RMXUtils::DisableScreenCapture(curCatSession.GetMainWnd(), !bRightGranted);
		return true;
	}

	return false;
}

bool CScreenCaptureControl::ApplyToCurrent(bool bFullScreen)
{
	LOG_ENTER_CALL(L"");
	bool bRightGranted = true;

	if (bFullScreen)
	{
		// Only check the current active doc
		bool bRightGranted = true;
		if (!curUCControl.CheckCommandRight(CatRMX::CMD_SCREENCAPTURE))
			bRightGranted = false;

		HWND hWnd = curCatSession.GetFullScreenTopWindow();
		if (hWnd != NULL)
		{
			LOG_DEBUG_FMT(L"Apply anti screen capture on VisuFullScreen %p", hWnd);
			RMXUtils::DisableScreenCapture(hWnd, !bRightGranted);
		}
	}
	else
	{
		// This is more suitable for command hook event.
		auto pWnd = curCatSession.GetActiveWindow();
		if (pWnd)
		{
			CAT::CatWindowState state;
			pWnd->get_WindowState(&state);
			if (state == 0 /*Maximize*/)
			{
				if (!curUCControl.CheckCommandRight(CatRMX::CMD_SCREENCAPTURE))
					bRightGranted = false;

				RMXUtils::DisableScreenCapture(curCatSession.GetMainWnd(), !bRightGranted);
				return true;
			}
		}

	}
	
	return false;
}