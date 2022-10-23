#include "SwWndHooks.h"
#include "SwUtils.h"
#include "SwUserAuthorization.h"
#include "SwRMXDialog.h"

namespace CSwWndHook {
	#define ID_BUTTON_COPY_FILES  54268
	#define ID_SAVE_TO_ZIP_FILES 1274 
	#define ID_JTEXPORT_EXPORT 220

	HHOOK g_hook;
	HWND g_hFindRefDlg;
	HWND g_hPackAndGoDlg;
	HWND g_hJTExportDlg;
	bool g_enableExportBtnInProgress = false;
	bool g_JTQuickExportRunning = false;
	
	bool ApplyUsageControlOnJTExport()
	{
		if (g_hJTExportDlg == nullptr) return false;

		auto activeModelDoc = CSwUtils::GetInstance()->GetCurrentlyActiveModelDoc();
		if (!activeModelDoc) return false;

 		HWND hExportBtn = GetDlgItem(g_hJTExportDlg, ID_JTEXPORT_EXPORT);
		if (hExportBtn != NULL)
		{
			wstring activeDoc = CSwUtils::GetInstance()->GetModelNameForModelDoc(activeModelDoc);
			CSwAuthResult authResultObj(activeDoc);
			bool isAuthorized = CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj);
			bool enabled = (IsWindowEnabled(hExportBtn) == TRUE);
			if (isAuthorized != enabled) {
				LOG_DEBUG_FMT(L"JTExport - Enable Export button to %d", isAuthorized);
				g_enableExportBtnInProgress = true;
				EnableWindow(hExportBtn, isAuthorized);
				g_enableExportBtnInProgress = false;
			}
		}

		return false;
	}

	BOOL CALLBACK EnumChildWindowCB(HWND hWnd, LPARAM lParam)
	{
		wchar_t szText[MAX_PATH];
		GetWindowText(hWnd, szText, MAX_PATH);
		LOG_DEBUG_FMT(L"EnumChildWindowCB: %s, %d " , szText, GetDlgCtrlID(hWnd));
		
		return TRUE;
	}

	LRESULT WINAPI WndProc(int nCode, WPARAM wParam, LPARAM lParam) {
		if (nCode < 0 || lParam == NULL)  // do not process message 
			return CallNextHookEx(g_hook, nCode, wParam, lParam);

		CWPSTRUCT* cwpStruct = (CWPSTRUCT*)lParam;

		// In case the export button is enabled by JT Translator, need to re-apply usage control afterwards.
		if (g_hJTExportDlg && !g_enableExportBtnInProgress &&
			(GetParent(cwpStruct->hwnd) == g_hJTExportDlg) &&
			(cwpStruct->message == WM_ENABLE)) {
			if (GetDlgCtrlID(cwpStruct->hwnd) == ID_JTEXPORT_EXPORT) {
				LOG_DEBUG_FMT(L"JTExport - Export button enabled to %d. Applying usage control...", cwpStruct->wParam);
				ApplyUsageControlOnJTExport();
			}

			return CallNextHookEx(g_hook, nCode, wParam, lParam);
		}
		long style = GetWindowLong(cwpStruct->hwnd, GWL_STYLE);

		if (!(style & WS_CAPTION))
			return CallNextHookEx(g_hook, nCode, wParam, lParam);

		wchar_t szWindowName[255];
		GetWindowText(cwpStruct->hwnd, szWindowName, 255);
		if (wcsstr(szWindowName, L"Find References") != NULL) {
			if (cwpStruct->message == WM_SHOWWINDOW)
			{
				LOG_DEBUG(L"Find References Dialog opened");
				g_hFindRefDlg = cwpStruct->hwnd;
				HWND copyFileBtn = GetDlgItem(g_hFindRefDlg, ID_BUTTON_COPY_FILES);
				if (copyFileBtn != NULL)
				{
					//Get the current active document and check for RMX_EXPORT
					wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
					CSwAuthResult authResultObj(activeDoc);
					bool isAuthorized = CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj);
					BOOL enabled = IsWindowEnabled(copyFileBtn);
					if (enabled && !isAuthorized) {
						LOG_INFO_FMT("Current active file = %s doesn't possess RMX_EXPORT permission. Disabling Copy Files button in Find Reference Dialog", activeDoc.c_str());
						EnableWindow(copyFileBtn, !enabled);
					}
				}
			}
			else if (cwpStruct->message == WM_NCDESTROY)
			{
				LOG_DEBUG(L"Find References Dialog closed");
				g_hFindRefDlg = nullptr;
			}
		}
		else if (wcsstr(szWindowName, L"Pack and Go") != NULL) {
			if (cwpStruct->message == WM_SHOWWINDOW)
			{
				LOG_DEBUG(L"Pack And Go Dialog opened");
				g_hPackAndGoDlg = cwpStruct->hwnd;
				HWND saveToZipFileBtn = GetDlgItem(g_hPackAndGoDlg, ID_SAVE_TO_ZIP_FILES);
				if (saveToZipFileBtn != NULL)
				{
					//Get the current active document and check if it contains any protected file.
					wstring activeDoc = CSwUtils::GetInstance()->GetCurrentActiveFile();
					bool hasNXLFile = CSwUtils::GetInstance()->IsFileHasNXLDoc(activeDoc);
					BOOL enabled = IsWindowEnabled(saveToZipFileBtn);
					if (enabled && hasNXLFile) {
						LOG_INFO_FMT("Current active file = %s has protected file in it. Currently RMX doesn't support zip support in Pack And Go.", activeDoc.c_str());
						EnableWindow(saveToZipFileBtn, !enabled);
					}
				}
			}
			else if (cwpStruct->message == WM_NCDESTROY)
			{
				LOG_DEBUG(L"Pack And Go Dialog closed");
				g_hPackAndGoDlg = nullptr;
			}
		}
		else if (wcsstr(szWindowName, L"PLM Components JT Translator for Solidworks") != NULL) {
			if (cwpStruct->message == WM_SHOWWINDOW)
			{
				LOG_DEBUG(L"JTExport Dialog shown. Applying usage control");
				g_hJTExportDlg = cwpStruct->hwnd;
				ApplyUsageControlOnJTExport();
			}
			else if (cwpStruct->message == WM_NCDESTROY)
			{
				LOG_DEBUG(L"JTExport Dialog closed. Unhook");
				g_hJTExportDlg = nullptr;
				auto ret = CallNextHookEx(g_hook, nCode, wParam, lParam);
				UnhookJTExport();

				return ret;
			}
			else if (cwpStruct->message == WM_COMMAND)
			{
				// In case the load file changed in editable field(ID:201), enable/disable export
				// button according to the new active doc.
				if (LOWORD(cwpStruct->wParam) == 201 &&
					HIWORD(cwpStruct->wParam) == EN_CHANGE) {

					LOG_DEBUG(L"JTExport - Load File changed. Applying usage control...");
					ApplyUsageControlOnJTExport();
				}
			}
		}
		else if (wcsstr(szWindowName, L"Translation Status") != NULL)
		{
			if (cwpStruct->message == WM_SHOWWINDOW)
			{
				LOG_DEBUG(L"JTExport Translation progress dialog opened");
				g_JTQuickExportRunning = true;
			}
			else if (cwpStruct->message == WM_NCDESTROY)
			{
				LOG_DEBUG(L"JTExport Translation progress dialog closed");
				UnhookJTExport();
			}
		}
		return CallNextHookEx(g_hook, nCode, wParam, lParam);
	}

	void CSwWndHook::Start() {
		g_hFindRefDlg = nullptr;
		g_hPackAndGoDlg = nullptr;
		g_hook = SetWindowsHookEx(WH_CALLWNDPROC, WndProc, (HINSTANCE)NULL, GetCurrentThreadId());
		if (g_hook == NULL)
		{
			LOG_ERROR(L"Failed to install windows hook.");
		}
	}

	void CSwWndHook::Stop() {
		UnhookWindowsHookEx(g_hook);
		g_hook = NULL;
		g_hFindRefDlg = nullptr;
		g_hPackAndGoDlg = nullptr;
	}

	void HookJTExport()
	{
		UnhookJTExport();
		g_hook = SetWindowsHookEx(WH_CALLWNDPROC, WndProc, (HINSTANCE)NULL, GetCurrentThreadId());
		if (g_hook == NULL)
		{
			LOG_ERROR(L"Failed to install windows hook.");
		}
	}

	void UnhookJTExport()
	{
		LOG_DEBUG(L"Unhook windows hook for JT Export");
		if (g_hook)
			UnhookWindowsHookEx(g_hook);
		g_hook = NULL;
		g_hJTExportDlg = nullptr;
		g_JTQuickExportRunning = false;
	}

	bool IsJTQuickExportRunning()
	{
		return g_JTQuickExportRunning;
	}

}