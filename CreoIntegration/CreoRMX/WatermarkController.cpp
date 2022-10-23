#include "WatermarkController.h"

#include "..\common\CreoRMXLogger.h"

#include "OtkXTypes.h"
#include "OtkXUtils.h"
#include "OtkXModel.h"

#include <pfcGlobal.h>

#include <LibRMX.h>
#include "CustomizeUsageControl.h"
#include "ProtectController.h"
#include "UsageController.h"
#include "OtkXSessionCache.h"
#include "OtkXUIStrings.h"

using namespace std;
using namespace OtkXUtils;

namespace OverlayHlp
{
	struct OtkXWindow
	{
		xint id;
		wstring mdlId;
		HWND hOverlayTargetWnd; /*Indicates view overlay is set to this window if it has value*/
	};

	static map<xint, OtkXWindow> g_xWindows;
	
	struct EnumWindowParams
	{
		DWORD in_dwProcessId;
		wstring in_mdlName;
		HWND out_hWnd;
		int out_displayOffset[4];
	};
	
	struct EnumChildWindowParams
	{
		wstring in_wndClassName;
		HWND out_hWnd;
	};

	BOOL CALLBACK EnumChildWindowCB(HWND hWnd, LPARAM lParam)
	{	
		if (!::IsWindowVisible(hWnd))
			return TRUE;

		auto pParams = (EnumChildWindowParams*)(lParam);
		wchar_t szClassName[MAX_PATH];
		GetClassName(hWnd, szClassName, MAX_PATH);
		if (wcsicmp(szClassName, pParams->in_wndClassName.c_str()) == 0) {
			pParams->out_hWnd = hWnd;
			SetLastError(ERROR_SUCCESS);
			return FALSE;
		}
		
		return TRUE;
	}

	bool FindModelWindow(const wstring& mdlName, EnumWindowParams& params)
	{
		wstring mdlNameLCase(mdlName);
		RMXUtils::ToLower<wchar_t>(mdlNameLCase);
		params.in_dwProcessId = ::GetCurrentProcessId();
		params.in_mdlName = mdlNameLCase;
		params.out_hWnd = nullptr;
	
		BOOL bResult = EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL
		{
			if (!::IsWindowVisible(hWnd))
				return TRUE;

			auto pParams = (EnumWindowParams*)(lParam);
			DWORD dwProcessId;
			GetWindowThreadProcessId(hWnd, &dwProcessId);
			if (dwProcessId == pParams->in_dwProcessId)
			{
				int len = GetWindowTextLength(hWnd) + 1;
				vector<wchar_t> textBuf(len);
				GetWindowText(hWnd, &textBuf[0], len);
				wstring wndText(&textBuf[0]);
				RMXUtils::ToLower<wchar_t>(wndText);
				if (wcsstr(wndText.c_str(), pParams->in_mdlName.c_str()) != NULL)
				{
					pParams->out_hWnd = hWnd;
					// When the top level window is found for the given model, try to find out
					// the child window for model display.
					EnumChildWindowParams childParams = { L"PGL_STD_WN_CLASS", nullptr };
					if (!EnumChildWindows(hWnd, EnumChildWindowCB, (LPARAM)&childParams) &&
						GetLastError() == ERROR_SUCCESS && childParams.out_hWnd)
					{
						LOG_DEBUG_FMT(L"Model window(PGL_STD_WN_CLASS) found @%p.(MainWnd: %p, Caption: %s)", childParams.out_hWnd,
							hWnd, wndText.c_str());
						RECT topWndRC, childWndRC;
						::GetWindowRect(hWnd, &topWndRC);
						::GetWindowRect(childParams.out_hWnd, &childWndRC);
						pParams->out_displayOffset[0] = 0;
						pParams->out_displayOffset[1] = childWndRC.top - topWndRC.top;
						pParams->out_displayOffset[2] = topWndRC.right - childWndRC.right;
						pParams->out_displayOffset[3] = topWndRC.bottom - childWndRC.bottom;
						return FALSE;
					}									
				}			
			}

			return TRUE;

		}, (LPARAM)&params);

		if (!bResult && GetLastError() == ERROR_SUCCESS && params.out_hWnd)
		{
			return true;
		}

		return false;
	}

	void DisableScreenCapture(std::vector<const wchar_t*>& files, HWND hWnd)
	{
		for (auto& f : files)
		{
			auto rightStatus = CUsageController::CheckRight(f, RMX_RIGHT_DECRYPT, false);
			if (rightStatus != Usage_Right_Grant)
			{
				// If any file does not have extract right, disallow print screen.
				RMXUtils::DisableScreenCapture(hWnd, true);
				return;
			}
		}
	}

	void ClearAllOverlay()
	{
		for (const auto& xWnd : g_xWindows)
		{
			if (xWnd.second.hOverlayTargetWnd)
			{
				RMX_RPMClearViewOverlay((void*)(xWnd.second.hOverlayTargetWnd));
				RMXUtils::DisableScreenCapture(xWnd.second.hOverlayTargetWnd, false);
			}
		}
		g_xWindows.clear();
	}

	void RemoveOverlay(const OtkXWindow& xWnd)
	{
		LOG_INFO_FMT(L"RemoveOverlay(wndId: %d, hWnd: %p, mdlId: %s", xWnd.id, xWnd.hOverlayTargetWnd, xWnd.mdlId.c_str());
		if (xWnd.hOverlayTargetWnd)
		{
			RMX_RPMClearViewOverlay((void*)xWnd.hOverlayTargetWnd);
			RMXUtils::DisableScreenCapture(xWnd.hOverlayTargetWnd, false);
		}
		g_xWindows.erase(xWnd.id);
	}

	void RemoveOverlay(pfcWindow_ptr pWnd)
	{
		if (pWnd == nullptr)
			return;

		auto iterOverlay = g_xWindows.find(pWnd->GetId());
		if (iterOverlay != g_xWindows.end())
		{
			RemoveOverlay(iterOverlay->second);
		}
	}

	void UpdateWindowOverlay(pfcWindow_ptr pWnd)
	{
		if (pWnd == nullptr)
			return;
	
		auto pMdl = pWnd->GetModel();
		bool isProtected = false;
		if (pMdl)
		{	
			COtkXModel xModel(pMdl);
			//LOG_DEBUG_FMT(L"UpdateWindowOverlay(wndId: %d, mdlOrigin: %s", pWnd->GetId(), xModel.GetOrigin().c_str());
			
			// Fix bug 60324: OnAfterModelDisplay triggered but no file attached to pWnd.
			// The old model is still displayed in the window. It seems the certain operation
			// trigger modeldisplay event with a fake window object.
			//if (xModel.IsValid())
			//{
				auto xWnd = g_xWindows.find(pWnd->GetId());
				if (xWnd != g_xWindows.end())
				{
					if (wcsicmp(xWnd->second.mdlId.c_str(), xModel.GetId().c_str()) == 0)
						return;

					// Remove old overlay in current window
					RemoveOverlay(xWnd->second);
				}

				OtkXWindow newXWnd = { pWnd->GetId(), xModel.GetId(), nullptr };

				isProtected = xModel.IsProtected();
			//}
			
			std::set<std::wstring> nxlFiles;
			xModel.GetDepNxlFiles(nxlFiles);
			// Ignore if no any nxl exists
			if (!isProtected && nxlFiles.size() == 0)
				return;

			const wstring mdlName(CXS2W(pMdl->GetInstanceName()));		
			OverlayHlp::EnumWindowParams wndParams;
			if (!FindModelWindow(mdlName, wndParams))
				return;

			
			//
			// Fill tags including dependencies
			// Design decision: If object or any of its dependency is protected, display
			// overlay. All tags of dependencies will also take int account. The first found
			// watermark obligation will be used.
			RMX_VIEW_OVERLAY_PARAMS params;
			vector<const wchar_t*> cVecFiles;
			wstring nxlFileLog;

			// Always put the current doc in windows in the first position
			// in order to evaluate watermark firstly.
			const auto& fileOrigin = xModel.GetOrigin();
			if (isProtected)
			{
				cVecFiles.push_back(fileOrigin.c_str());
				nxlFileLog += fileOrigin;
			}
				
			for (const auto& f : nxlFiles)
			{
				cVecFiles.push_back(f.c_str());
				if (nxlFileLog.empty())
					nxlFileLog += f;
				else
					nxlFileLog += L";" + f;
			}

			params.files.count = cVecFiles.size();
			params.files.fileArr = cVecFiles.data();

			//
			// Overlay API only supports to attach top main window
			params.hTargetWnd = wndParams.out_hWnd;

			//
			// Fill display offset to exclude the title bar and status bar.
			params.displayOffset[0] = wndParams.out_displayOffset[0];
			params.displayOffset[1] = wndParams.out_displayOffset[1];
			params.displayOffset[2] = wndParams.out_displayOffset[2];
			params.displayOffset[3] = wndParams.out_displayOffset[3];
			//
			// Fill context attributes
			const wstring filename = CXS2W(pMdl->GetFileName());
			RMX_EVAL_ATTRIBUTE ctxAttrs[] ={
				{ L"rmxname", L"CreoRMX" },
				{ L"rmxversion", _W(VER_FILEVERSION_STR) },
				{ L"filename", filename.c_str()},
				{ L"filepath", fileOrigin.c_str() } };
			params.ctxAttrs.attrs = ctxAttrs;
			params.ctxAttrs.count = sizeof(ctxAttrs) /sizeof(RMX_EVAL_ATTRIBUTE);
			
			LOG_INFO(L"Attempting to set view overlay...");
			LOG_INFO_FMT(L"\t-wndId: %d", pWnd->GetId());
			LOG_INFO_FMT(L"\t-hWnd: %p", wndParams.out_hWnd);
			LOG_INFO_FMT(L"\t-srcModel: %s", xModel.GetId().c_str());
			LOG_INFO_FMT(L"\t-srcFile: %s", fileOrigin.c_str());
			LOG_INFO_FMT(L"\t-nxlFiles: %s", nxlFileLog.c_str());
			
			if (RMX_RPMSetViewOverlay(params))
			{
				newXWnd.hOverlayTargetWnd = wndParams.out_hWnd;
				g_xWindows[pWnd->GetId()] = newXWnd;
				LOG_INFO_FMT(L"AddOverlay successfully(wndId: %d, hWnd: %p, mdlId: %s", newXWnd.id, newXWnd.hOverlayTargetWnd, newXWnd.mdlId.c_str());
			}

			// Anti screen capture
			DisableScreenCapture(cVecFiles, wndParams.out_hWnd);
		}
	}
}

void COverlaySessionActionListener::OnAfterModelDisplay()
{
	try
	{
		auto pSess = pfcGetProESession();
		if (pSess)
		{
			auto pWnd = pSess->GetCurrentWindow();
			if (pWnd != nullptr)
			{
				COtkXModel xMdl(pWnd->GetModel());
				if (OtkXSessionCacheHolder().IsDirtyToReload(xMdl.GetId()))
				{
					LOG_ERROR(L"Must retrieve model from disk after protection: " << xMdl.GetId().c_str());
					
					if (!OtkX_RunInGraphicMode())
					{
						LOG_WARN(L"No graphics mode. Ignore to display info for protected model");
						return;
					}

					xstringsequence_ptr msgs = xstringsequence::create();
					xstring line(CW2XS(xMdl.GetId().c_str()));
					msgs->append(line);
					pSess->UIDisplayMessage(OTKX_MSGFILE, "RMXFileReload", msgs);
				}
				else
				{
					OverlayHlp::UpdateWindowOverlay(pWnd);
					CustomizeUsageControl::HandleOnAfterModelDisplay(pWnd->GetModel());
				}
			}
		}
	}
	OTKX_EXCEPTION_HANDLER();
}

void CWatermarkController::Start()
{
	LOG_INFO(L"Starting watermark control...");
	// Register session level action listeners
	try
	{
		pfcSession_ptr pSession = pfcGetProESession();
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}
		auto pSessLis = new COverlaySessionActionListener();
		pSession->AddActionListener(pSessLis);
		m_listeners.push_back(pSessLis);
		LOG_INFO(L"\t-COverlaySessionActionListener added");

		auto pMdlLis = new COverlayModelEventActionListener();
		pSession->AddActionListener(pMdlLis);
		m_listeners.push_back(pMdlLis);
		LOG_INFO(L"\t-COverlayModelEventActionListener added");

		static const string closeCmd(OTKX_COMMAND_WINCLOSE);
		pfcUICommand_ptr pCmd = pSession->UIGetCommand(CA2XS(closeCmd.c_str()));
		if (pCmd)
		{
			COverlayCommandBracketListener* pListener = new COverlayCommandBracketListener(closeCmd);
			pCmd->AddActionListener(pListener);
			m_cmdListeners[closeCmd] = pListener;
			LOG_INFO(L"\t-Watermark control applied: " << closeCmd.c_str());
		}
	}
	OTKX_EXCEPTION_HANDLER();
}

void CWatermarkController::Stop()
{
	LOG_INFO(L"Stopping watermark control...");
	try
	{
		pfcSession_ptr pSession = pfcGetProESession();
		if (pSession == nullptr)
		{
			LOG_ERROR(L"Cannot retrieve the ProE Session");
			return;
		}

		for(auto &lis : m_listeners)
		{
			pSession->RemoveActionListener(lis);
		}	

		for (const auto& lis : m_cmdListeners)
		{
			pfcUICommand_ptr pCmd = pSession->UIGetCommand(CA2XS(lis.first.c_str()));
			if (pCmd)
			{
				pCmd->RemoveActionListener(lis.second);
			}
		}		
	}
	OTKX_EXCEPTION_HANDLER();
	m_listeners.clear();
	m_cmdListeners.clear();
	OverlayHlp::ClearAllOverlay();
}

COverlayCommandBracketListener::COverlayCommandBracketListener(const std::string & cmdName)
	: m_cmdName(cmdName)
{
	
}


void COverlayCommandBracketListener::OnBeforeCommand()
{
	OTKCALL_LOG_ENTER
	try
	{
		auto pSess = pfcGetProESession();
		if (pSess)
		{
			OverlayHlp::RemoveOverlay(pSess->GetCurrentWindow());
		}
	}
	OTKX_EXCEPTION_HANDLER();
}

void COverlayModelEventActionListener::OnAfterModelErase(pfcModelDescriptor_ptr pMdlDescr)
{
	OTKCALL_LOG_ENTER
	if (pMdlDescr == nullptr)
		return;

	try
	{
		auto pSess = pfcGetProESession();
		if (pSess == nullptr)
			return;

		auto pMdl = pSess->GetModelFromDescr(pMdlDescr);
		if (pMdl == nullptr)
			return;

		OverlayHlp::RemoveOverlay(pSess->GetModelWindow(pMdl));		
	}
	OTKX_EXCEPTION_HANDLER();
	
}
