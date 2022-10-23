// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <internals/Utilities.h>
#include <NxlUtils/RMXUtils.hpp>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	wchar_t path[MAX_PATH];
	int len = GetModuleFileName(hModule, path, MAX_PATH);
	//auto utf8 = Utilities::wstring_to_utf8(path);
	DWORD ver = SDWLibGetVersion();
	DWORD AA = (ver & 0xFF000000) >> 24;
	DWORD BB = (ver & 0xFF0000) >> 16;
	DWORD CCCC = ver & 0xFFFF;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		RMXUtils::initializeLogger();
		INFOLOG(TARGETFILENAME ": Version=" FILE_VER " RPMLibVersion=%d.%d.%d FullPath='%s'", AA, BB, CCCC, path);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		TRACELOG("unloading lib-'%s'", path);
		log4cplus::Logger::shutdown();
		break;
	}
	return TRUE;
}

