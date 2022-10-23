#include "stdafx.h"
#include "WatermarkControl.h"
#include "CatSession.h"
#include <LibRMX.h>
#include "RMXUsageControl.h"

void CWatermarkControl::Start()
{
	LOG_INFO(L"Starting watermark control...");

	m_hMainWnd = curCatSession.GetMainWnd();
	m_hWndParent = NULL;

	// Register event listeners
	curEventMgr.AddListener((CCatEventLisener*)this, eCatNotify_FrmLayoutChanged | eCatNotify_AfterCommand | eCatNotify_FullScreenChanged);
}

void CWatermarkControl::Stop()
{
	LOG_INFO(L"Stopping watermark control...");

	RemoveWatermark();
}

void CWatermarkControl::UpdateWatermarkForLoad()
{
	LOG_ENTER_CALL(L"");
	
	CAT::DocumentPtr pDoc = curCatSession.GetActiveDocument();
	if (pDoc == NULL)
	{
		RemoveWatermark();
		return;
	}
	
	wstring wsFileFullName;
	if (!CCatSession::GetDocFileName(pDoc, wsFileFullName))
	{
		RemoveWatermark();
		return;
	}

	HWND hWndTarget = curCatSession.GetFullScreenTopWindow();
	if (hWndTarget == NULL)
		hWndTarget = m_hMainWnd;

	if (SetWatermark(hWndTarget, wsFileFullName))
	{
		return;
	}

	if(!curCatSession.TraverseLinks(pDoc, [&](CAT::DocumentPtr pLinkDoc) -> bool {
		wstring wsLinkFullName;
		if (CCatSession::GetDocFileName(pLinkDoc, wsLinkFullName) && SetWatermark(hWndTarget, wsLinkFullName))
		{
			return true; // Terminate
		}
		
		return false;
	}))
	{
		// Remove old one if no one found.(component delete?)
		RemoveWatermark();
	}	
}

// Called to update watermark after any nxl component was inserted to new doc
void CWatermarkControl::UpdateWatermarkForNew()
{
	LOG_ENTER_CALL(L"");
	HWND hWndTarget = curCatSession.GetFullScreenTopWindow();
	if (hWndTarget == NULL)
		hWndTarget = m_hMainWnd;

	// TODO: Cache watermark for file and ignore 2nd time evaluation if watermark is already created?
	
	CAT::DocumentPtr pDoc = curCatSession.GetActiveDocument();
	wstring wsFileFullName;
	// Evaluate the linked docs if the root doc is not existing in disk.
	// Note: CPathEx::IsFile(L"Product1.CATProduct") doesn't work well. So check path seperator to determine if this given file name
	// is a path or not.
	if (CCatSession::GetDocFileName(pDoc, wsFileFullName) && (wcsstr(wsFileFullName.c_str(), L"\\") == NULL))
	{	
		if (!curCatSession.TraverseLinks(pDoc, [&](CAT::DocumentPtr pLinkDoc) -> bool {
			wstring wsLinkFullName;
			if (CCatSession::GetDocFileName(pLinkDoc, wsLinkFullName))
			{
				if (SetWatermark(hWndTarget, wsLinkFullName))
				{
					return true; // Terminate
				}
			}

			return false;
			}))
		{
			RemoveWatermark();
		}
	}
}

void CWatermarkControl::RemoveWatermark()
{
	if (m_hWndParent != NULL)
	{
		RMX_RPMClearViewOverlay(m_hWndParent);
		m_wsCurDoc.clear();
		m_hWndParent = NULL;
	}
}

bool CWatermarkControl::SetWatermark(HWND hWndTarget, const wstring& wsNxlFullName)
{
	LOG_ENTER_CALL(wsNxlFullName.c_str());
	if (!curCatSession.IsNxlFile(wsNxlFullName))
		return false;

	// No change
	// Note: In case the file in disk is diff from the one in session, 
	// we always assume no change after file gets opened, regarding to permission and watermark

	if (hWndTarget == m_hWndParent && StringHlp::ICompare(m_wsCurDoc, wsNxlFullName))
		return true;
	//
	// Fill tags including dependencies
	// Design decision: If object or any of its dependency is protected, display
	// overlay. All tags of dependencies will also take int account. The first found
	// watermark obligation will be used.
	RMX_VIEW_OVERLAY_PARAMS params;
	vector<const wchar_t*> cVecFiles;
	cVecFiles.push_back(wsNxlFullName.c_str());
	params.files.count = cVecFiles.size();
	params.files.fileArr = cVecFiles.data();

	//
	// Fill target window
	// Note: The top window is diff between full screen and normal mode
	params.hTargetWnd = hWndTarget;

	//
	// Fill display offset
	params.displayOffset[0] = 0;
	params.displayOffset[1] = 0;
	params.displayOffset[2] = 0;
	params.displayOffset[3] = 0;
	//
	// Fill context attributes

	CPathEx filePath(wsNxlFullName.c_str());
	RMX_EVAL_ATTRIBUTE ctxAttrs[] = {
		{ L"rmxname", L"catiarmx" },
		{ L"rmxversion", _W(VER_FILEVERSION_STR) },
		{ L"filename", filePath.GetFileName().c_str() },
		{ L"filepath", wsNxlFullName.c_str() } };
	params.ctxAttrs.attrs = ctxAttrs;
	params.ctxAttrs.count = sizeof(ctxAttrs) / sizeof(RMX_EVAL_ATTRIBUTE);

	LOG_INFO(L"Attempting to set view overlay...");
	LOG_INFO_FMT(L"\t-hWnd: %p", params.hTargetWnd);
	LOG_INFO_FMT(L"\t-srcFile: %s", wsNxlFullName.c_str());

	if (RMX_RPMSetViewOverlay(params))
	{
		m_wsCurDoc = wsNxlFullName;
		m_hWndParent = hWndTarget;
		return true;
	}

	return false;
}

void CWatermarkControl::OnAfterCommand(LPCatCommandNotify pParam)
{
	// For "Insert existing component" command, need to evaluate watermark from
	// linked docs if root doc is new doc
	// TODO: Update for existing doc? Performance issue as this command will be sent many times such as file open, 
	// file insert, delete component, etc.
	if (StringHlp::ICompare(pParam->wsCmdName.c_str(), CatRMX::CMD_UPDATENODECHILDREN))
	{
		UpdateWatermarkForNew();
	}
}

void CWatermarkControl::OnFrmLayoutChanged(LPCatFrmNotify pParam)
{
	static set<wstring> cmdsToProcess = {
			CatRMX::CMD_ACTIVATEWINDOW,
			CatRMX::CMD_FRMCLOSEBOX
	};
	if (cmdsToProcess.find(pParam->wsCmdName) != cmdsToProcess.end())
	{
		UpdateWatermarkForLoad();
	}
}


void CWatermarkControl::OnFullScreenChanged(int nFullScreen)
{
	LOG_ENTER_CALL(L"");
	UpdateWatermarkForLoad();
}
