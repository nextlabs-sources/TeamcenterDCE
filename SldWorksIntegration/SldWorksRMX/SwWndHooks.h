#pragma once
#include "stdafx.h"

namespace CSwWndHook
{
	LRESULT WINAPI WndProc(int nCode, WPARAM wParam, LPARAM lParam);
	void Start();
	void Stop();

	void HookJTExport();
	void UnhookJTExport();

	// Temporary fix. To be refactorying
	bool IsJTQuickExportRunning();
	
}