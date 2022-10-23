//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   June 2022
//////////////////////////////////////////////////////////////////////////////
//

#include "StdAfx.h"
#include "AcInc.h"
#include "AcRMXWatermarkControl.h"
#include "AcRMXSessionCacheMgr.h"
#include "AcRMXAccessControl.h"
#include <LibRMX.h>

#include "AcRMXUtils.h"
#include "AcRMXTypes.h"

using namespace std;

#define AC_LOG_ENTER(msg) AC_LOG_ENTER_BODY(L"WINDOWREACTOR", msg) // log guard helper

CAcRMXApWinowMgrReactor* CAcRMXApWinowMgrReactor::m_pInstance = NULL;

CAcRMXApWinowMgrReactor::CAcRMXApWinowMgrReactor()
{
	acDocWindowManager->addReactor(this);
}

CAcRMXApWinowMgrReactor::~CAcRMXApWinowMgrReactor()
{
	acDocWindowManager->removeReactor(this);
}

void CAcRMXApWinowMgrReactor::documentWindowFloated(AcApDocWindow* docWindow)
{
	AC_LOG_ENTER(L"");
	// Floating window needs to be evaluated separately when to detemince the capture
	// permission for float frame. 
	WatermarkController().ApplyAntiScreenCapture();
}

void CAcRMXApWinowMgrReactor::documentWindowDocked(AcApDocWindow* docWindow)
{
	AC_LOG_ENTER(L"");
	// Docked window should be taken into account when evaluating 
	// capture permission for main window.
	WatermarkController().ApplyAntiScreenCapture();
}

CAcRMXApWinowMgrReactor* CAcRMXApWinowMgrReactor::getInstance()
{
	if (m_pInstance)
		return m_pInstance;

	m_pInstance = new CAcRMXApWinowMgrReactor;
	LOG_DEBUG(L"Window Manager Reactor Added");
	return m_pInstance;
}

void CAcRMXApWinowMgrReactor::destroyInstance()
{
	if (m_pInstance) {
		delete m_pInstance;
		m_pInstance = NULL;
		LOG_DEBUG(L"Window Manager Reactor destroyed");
	}
	else {
		ASSERT(0);
	}
}

void CAcRMXWatermarkControl::Start()
{
	LOG_INFO(L"Starting watermark control...");
	m_bUpdateDisabled = false;
}

void CAcRMXWatermarkControl::Stop()
{
	LOG_INFO(L"Stopping watermark control...");
	RemoveViewOverlay();
	m_bUpdateDisabled = false;
}

bool CAcRMXWatermarkControl::SetViewOverlay(const CString& fileName)
{
	if (!SessionCacheMgr().IsNxlFile(fileName))
		return false;

	RMX_VIEW_OVERLAY_PARAMS params;

	vector<const wchar_t*> cVecFiles;
	cVecFiles.push_back(fileName);
	params.files.count = cVecFiles.size();
	params.files.fileArr = cVecFiles.data();

	//
	// Overlay API only supports to attach top main window
	params.hTargetWnd = adsw_acadMainWnd();

	//
	// Fill display offset
	params.displayOffset[0] = 0;
	params.displayOffset[1] = 0;
	params.displayOffset[2] = 0;
	params.displayOffset[3] = 0;

	//
	// Fill context attributes
	CPathEx filePath(fileName);
	RMX_EVAL_ATTRIBUTE ctxAttrs[] = {
		{ L"rmxname", L"AutoCADRMX" },
		{ L"rmxversion", _W(VER_FILEVERSION_STR) },
		{ L"filename", filePath.GetFileName().c_str()},
		{ L"filepath", filePath.c_str() } };
	params.ctxAttrs.attrs = ctxAttrs;
	params.ctxAttrs.count = sizeof(ctxAttrs) / sizeof(RMX_EVAL_ATTRIBUTE);

	LOG_INFO(L"Attempting to set view overlay...");
	LOG_INFO_FMT(L"\t-hWnd: %p", params.hTargetWnd);
	LOG_INFO_FMT(L"\t-srcFile: %s", fileName);

	return RMX_RPMSetViewOverlay(params);
}

void CAcRMXWatermarkControl::UpdateViewOverlay(AcApDocument* pActivatedDoc)
{
	if (m_bUpdateDisabled) return;

	// Remove old overlay if its exists
	// Note: RPM API not working well to update text, so eachtime we have to 
	// remove old one and create new one
	RemoveViewOverlay();

	if ((pActivatedDoc == NULL) || (pActivatedDoc->fileName() == NULL))
	{
		return;
	}
	
	ApplyAntiScreenCapture();

	// Evaluate watermark from main doc
	if (SetViewOverlay(pActivatedDoc->fileName()))
		return;

	// Evaluate from xrefs
	if(AcRMXUtils::TraverseXrefs([&](const TCHAR* szXrefFullPath) -> bool {
		if (szXrefFullPath && RMX_IsProtectedFile(szXrefFullPath))
		{
			if (SetViewOverlay(szXrefFullPath))
				return true; // Set sucessful, terminate recursive loop
		} 

		return false;
	}, pActivatedDoc->database()))
		return;

	// Evaluate from underlays
	AcRMXUtils::TraverseUnderlayRefs([&](const TCHAR* szRefFullPath) -> bool {
		if (szRefFullPath && RMX_IsProtectedFile(szRefFullPath))
		{
			if (SetViewOverlay(szRefFullPath))
				return true;  // Set sucessful, terminate recursive loop
		}
		return false;
	}, pActivatedDoc->database());
}

void CAcRMXWatermarkControl::RemoveViewOverlay()
{
	auto hMainWnd = adsw_acadMainWnd();
	if (hMainWnd != NULL)
	{
		RMX_RPMClearViewOverlay(hMainWnd);
		RMXUtils::DisableScreenCapture(hMainWnd, false);
	}
}

void CAcRMXWatermarkControl::ApplyAntiScreenCapture()
{
	if (m_bUpdateDisabled) return;

	//Skip if no protected is openning
	if (!SessionCacheMgr().HasNxlFileOpened())
	{
		RMXUtils::DisableScreenCapture(adsw_acadMainWnd(), false);
		return;
	}
		
	// Check active window firstly. 
	// If it's maxmized, only evaluate its document when other windows in MDI frame are hidden. 
	bool antiOnMainWnd = false;
	auto pWnd = acDocWindowManager->activeDocumentWindow();
	if (pWnd != nullptr && !pWnd->isFloating() && pWnd->document()
		&& pWnd->document()->isKindOf(AcApDocument::desc()))
	{
		LONG lStyles = GetWindowLong(pWnd->windowHandle(), GWL_STYLE);
		if (lStyles & WS_MAXIMIZE)
		{
			auto pDoc = AcApDocument::cast(pWnd->document());
			LOG_DEBUG_FMT(L"Checking active window when maxmized(doc: %s, floating: false, hwnd: %p)", (LPCWSTR)pDoc->fileName(), pWnd->windowHandle());
			if (!AccessController().CheckCommandRight(/*MSGO*/L"RMXSCREENCAPTURE", pDoc))
			{
				if (RMXUtils::DisableScreenCapture(adsw_acadMainWnd(), true))
					LOG_WARN(L"Screen capture disabled on main window. Extract permission not found on active window");
			}
			antiOnMainWnd = true;
		}
	}

	// Loop all existing windows.
	auto pItr1 = acDocWindowManager->newIterator();
	if (pItr1 == nullptr) return;

	for (; !pItr1->done(); pItr1->step())
	{
		auto pWnd = pItr1->current();
		if (pWnd == nullptr || pWnd->document() == nullptr ||
			!pWnd->document()->isKindOf(AcApDocument::desc()))
			continue;
	
		LONG lStyles = GetWindowLong(pWnd->windowHandle(), GWL_STYLE);
		bool isMaximize = false;
		if (lStyles & WS_MAXIMIZE)
			isMaximize = true;
	
		auto pDoc = AcApDocument::cast(pWnd->document());
		LOG_DEBUG_FMT(L"Checking window(doc: %s, isFloat: %d, hwnd: %p, isMaxmized=%d", (LPCWSTR)pDoc->fileName(), pWnd->isFloating(), pWnd->windowHandle(), isMaximize);
		// In the floating frame, disable capture on each floating window if no permission found in its doc.
		if (pWnd->isFloating())
		{
			LOG_DEBUG_FMT(L"Applying anti-screen capture on the floating window", (LPCWSTR)pDoc->fileName(), pWnd->windowHandle());
			bool hasRight = !AccessController().CheckCommandRight(/*MSGO*/L"RMXSCREENCAPTURE", AcApDocument::cast(pWnd->document()));
			auto hWndParent = GetParent(pWnd->windowHandle());
			if(RMXUtils::DisableScreenCapture(hWndParent, hasRight) && !hasRight)
				LOG_WARN_FMT(L"Screen capture disabled on float window %s", pWnd->title());
		}
		else // For MDI window, disable capture once any child window in MDI frame does not have permission.
		{
			// Ignore other windows in MDI frame when the anti one found already
			if (antiOnMainWnd)
				continue;

			LOG_DEBUG_FMT(L"Applying anti-screen capture on the window", (LPCWSTR)pDoc->fileName(), pWnd->windowHandle());
			if (!AccessController().CheckCommandRight(/*MSGO*/L"RMXSCREENCAPTURE", AcApDocument::cast(pWnd->document())))
			{
				if (RMXUtils::DisableScreenCapture(adsw_acadMainWnd(), true))
					LOG_WARN_FMT(L"Screen capture disabled on main window. Extract permission not found on %s window", pWnd->title());

				antiOnMainWnd = true; // If any child window in MDI frame found, disable MDI main winodow.
			}
		}
	} 

	// Revert screen capture if all has permission
	if (!antiOnMainWnd)
		RMXUtils::DisableScreenCapture(adsw_acadMainWnd(), false);
	
	delete pItr1;
}
