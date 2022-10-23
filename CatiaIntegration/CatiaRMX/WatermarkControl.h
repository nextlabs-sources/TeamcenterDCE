#pragma once
#include "CatEventMgr.h"

class CWatermarkControl : public RMXControllerBase<CWatermarkControl>, public CCatEventLisener
{
public:
	void Start();
	void Stop();

	void UpdateWatermarkForLoad();
	void UpdateWatermarkForNew();
	void RemoveWatermark();
	bool SetWatermark(HWND hWndTarget, const wstring& wsNxlFullName);

	// Event handler
	void OnAfterCommand(LPCatCommandNotify pParam);
	void OnFrmLayoutChanged(LPCatFrmNotify pParam);
	void OnFullScreenChanged(int nFullScreen);

private:
	wstring m_wsCurDoc;
	HWND m_hMainWnd;
	HWND m_hWndParent;
};

#define curWatermarkControl CWatermarkControl::GetInstance()