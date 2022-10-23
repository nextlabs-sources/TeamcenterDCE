#include "CustomizeUsageControl.h"

#include "OtkXUIStrings.h"
#include "OtkXUtils.h"

#include "OtkXSessionCache.h"
#include "ProtectController.h"
#include "UsageController.h"

#include <LibRMX.h>

using namespace OtkXUtils;

namespace CustomizeUsageControl
{
#define ID_BUTTON_SEARCH 1024
#define ID_BUTTON_EXPORT 1000

	HHOOK g_hook;
	HWND g_hExportDlg;
	std::string g_promptedError;
	std::wstring g_prevProcessedMdl;

	BOOL CheckRightForModel(const COtkXModel& xMdl, std::string& errorMsg)
	{
		auto rightStatus = CUsageController::CheckRight(xMdl, RMX_RIGHT_EXPORT, true);
		if (rightStatus == Usage_Right_Deny_FileNotExists)
		{
			errorMsg = IDS_ERR_DENY_OP_LOSEFILE;
			LOG_ERROR(RMXUtils::s2ws(errorMsg).c_str());
			return FALSE;
		}
		// JT Export always make model modified. So we check both save as and edit permssion
		// for this operation, regardless if the model is modified
		if (rightStatus == Usage_Right_Deny)
		{
			errorMsg = RMXUtils::FormatString(IDS_ERR_DENY_EX_JTCURRENT_FMT, "Save As");
			LOG_ERROR(RMXUtils::s2ws(errorMsg).c_str());
			return FALSE;
		}

		if (rightStatus == Usage_Right_DenyEdit)
		{
			errorMsg = RMXUtils::FormatString(IDS_ERR_DENY_EX_JTCURRENT_FMT, "Edit");
			LOG_ERROR(RMXUtils::s2ws(errorMsg).c_str());
			return FALSE;
		}

		std::set<std::wstring> denyFiles;
		auto buildErrorMsg = [&]() -> void {
			errorMsg += IDS_ERR_DENY_EX_JTCURRENT_SURFFIX;
			std::string fileInfo;
			for (const auto& file : denyFiles)
			{
				fileInfo = RMXUtils::FormatString("\n- %s", RMXUtils::ws2s(file).c_str());
				errorMsg += fileInfo;
			}
			LOG_ERROR(RMXUtils::s2ws(errorMsg).c_str());
		};
		rightStatus = CUsageController::CheckDepRight(xMdl, RMX_RIGHT_EXPORT, denyFiles, true);
		if (rightStatus == Usage_Right_Deny_FileNotExists)
		{
			errorMsg = (IDS_ERR_DENY_OP_DEP_LOSEFILE);
			buildErrorMsg();
			return FALSE;
		}
		if (rightStatus == Usage_Right_Deny)
		{
			errorMsg = RMXUtils::FormatString(IDS_ERR_DENY_EX_DEP_FMT, "Save As");
			buildErrorMsg();
			return FALSE;
		}
		if (rightStatus == Usage_Right_DenyEdit)
		{
			errorMsg = RMXUtils::FormatString(IDS_ERR_DENY_EX_DEP_FMT, "Edit").c_str();
			buildErrorMsg();
			return FALSE;
		}

		return TRUE;
	}

	void UpdateUsageControlOnJTCurrent(pfcModel_ptr pMdl)
	{
		if (pMdl == nullptr || g_hExportDlg == nullptr) return;

		HWND exportBtn = GetDlgItem(g_hExportDlg, ID_BUTTON_EXPORT);
		if (exportBtn != NULL)
		{
			BOOL enabled = IsWindowEnabled(exportBtn);
			std::string errMsg;
			COtkXModel xMdl(pMdl);
			BOOL hasRight = CheckRightForModel(xMdl, errMsg);
			if (hasRight != enabled)
			{
				// Update the enable status
				EnableWindow(exportBtn, hasRight);
			}
			// Fix to avoid duplicated prompt
			// If previous checked model is same to current one and error is same, ignore prompt
			if (!errMsg.empty() &&
				g_promptedError.compare(errMsg) != 0 &&
				wcsicmp(g_prevProcessedMdl.c_str(), xMdl.GetOrigin().c_str()) != 0)
			{
				OtkX_ShowMessageDialog(errMsg.c_str(), pfcMESSAGE_ERROR);
			}
			g_prevProcessedMdl = xMdl.GetOrigin();
			g_promptedError = errMsg;
		}
	}

	LRESULT CALLBACK WndProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode < 0)  // do not process message 
			return CallNextHookEx(g_hook, nCode, wParam,
				lParam);

		// Call an application-defined function that converts a message 
		// constant to a string and copies it to a buffer. 
		if (lParam != NULL)
		{
			CWPSTRUCT* cwpStruct = (CWPSTRUCT*)lParam;
			long style = GetWindowLong(cwpStruct->hwnd, GWL_STYLE);

			if (!(style & WS_CAPTION))
				return CallNextHookEx(g_hook, nCode, wParam, lParam);

			wchar_t szWindowName[255];
			GetWindowText(cwpStruct->hwnd, szWindowName, 255);
			//LOG_DEBUG(L"Create  Window" << szWindowName);
			if (wcsstr(szWindowName, L"PLM Components JT Translator for Creo Parametric") != NULL) {
				if (cwpStruct->message == WM_INITDIALOG)
				{
					LOG_DEBUG(L"[JTCurrent]Export Dialog opened");
					g_hExportDlg = cwpStruct->hwnd;
					auto pMdl = OtkX_GetCurrentModel();
					UpdateUsageControlOnJTCurrent(pMdl);
				}
				else if (cwpStruct->message == WM_NCDESTROY)
				{
					LOG_DEBUG(L"[JTCurrent]Export Dialog closed");
					g_hExportDlg = nullptr;
				}
			}
		}
		return CallNextHookEx(g_hook, nCode, wParam, lParam);
	}

	void HandleOnAfterModelDisplay(pfcModel_ptr pMdl)
	{
		if (CProtectController::GetInstance().IsWatcherRunning(PROTECT_WATCHER_EXPORT_JTCURRENT))
		{
			UpdateUsageControlOnJTCurrent(pMdl);
		}
	}

	void Start()
	{
#if defined(CREO_VER) && CREO_VER == 7
		// Don't support to load file in export dialog on Creo7 + JT Translator v16 
#else
		if (GetModuleHandle(L"JtTk103.dll") != NULL)
		{
			g_hExportDlg = nullptr;
			g_hook = SetWindowsHookEx(WH_CALLWNDPROC, WndProc, (HINSTANCE)NULL, GetCurrentThreadId());
			if (g_hook == NULL)
			{
				LOG_ERROR(L"Failed to install windows hook for customize usage control");
			}
			else
			{
				LOG_INFO(L"Customize usage control started.");
			}
		}
#endif
	}
	
	void Stop()
	{
#if defined(CREO_VER) && CREO_VER == 7
#else
		if (g_hook != NULL)
		{
			UnhookWindowsHookEx(g_hook);
			g_hook = NULL;
		}

		g_hExportDlg = nullptr;
		g_promptedError.clear();
		g_prevProcessedMdl.clear();

		LOG_INFO(L"Customize usage control stopped.");
#endif
	}

} // namespace CustomizeUsageControl

