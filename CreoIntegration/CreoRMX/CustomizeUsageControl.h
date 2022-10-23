#pragma once
#include <Windows.h>

#include <pfcModel.h>

namespace CustomizeUsageControl
{
	LRESULT CALLBACK WndProc(int nCode, WPARAM wParam, LPARAM lParam);
	void HandleOnAfterModelDisplay(pfcModel_ptr pMdl);

	void Start();
	void Stop();

}  // namespace CustomizeUsageControl