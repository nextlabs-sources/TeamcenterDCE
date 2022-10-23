#pragma once
#include "stdafx.h"

class CSwViewOverlay {
private:
	static HWND _topWindow;
	HWND _childWindow;
	wstring _fileName;

public :
	CSwViewOverlay();
	CSwViewOverlay(wstring fileName);
	~CSwViewOverlay();
	void SetViewOverlay();
	void ClearViewOverlay();
	void ResetViewOverlay();
	static void RefreshActiveWindow();
	inline static void CleanUp() {
		_topWindow = nullptr;
	}

	bool DisableScreenCapture(const wstring& fileName);
};