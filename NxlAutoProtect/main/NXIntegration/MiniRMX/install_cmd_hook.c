#include <Windows.h>
#include <hook/hook_injector.h>
#include <utils_log.h>

BOOL cmd_install()
{
	HMODULE module = GetModuleHandle(TARGETFILENAME);
	if(module != NULL)
	{
		char dllPath[MAX_PATH];
		if(GetModuleFileName(module, dllPath, MAX_PATH) > 0)
		{
			return inject_on_CreateProcess(dllPath);
		}
		else
		{
			DBGLOG("Failed to GetModuleFileName(" TARGETFILENAME ")!(ErrorCode:%#X)", GetLastError());
		}
	}
	else
	{
		DBGLOG("Failed to GetModuleHandle(" TARGETFILENAME ")!(ErrorCode:%#X)", GetLastError());
	}
	return FALSE;
}
void cmd_uninstall()
{
	inject_on_CreateProcess(NULL);
}