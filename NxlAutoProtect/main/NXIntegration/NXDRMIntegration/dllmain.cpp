// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <NxlUtils/NxlPath.hpp>
#include <NxlUtils/NxlDllLoader.hpp>


class NxDllLoader :public NxlDllLoader {
public:
	void OutputMessage(const std::wstring& msg, DWORD code = 0) override {
		if (code == 0) {
			DBGLOG("%s", msg.c_str());
		}
		else
		{
			ERRLOG("%s(Error=%lu(%#X))", msg.c_str(), code, code);
		}
	}
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	wchar_t path[MAX_PATH];
	int len = GetModuleFileNameW(hModule, path, MAX_PATH);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		RMXUtils::initializeLogger();
		{
			NxDllLoader loader;
			if (!loader.Load(L"NxlUtils64.dll"))
				return FALSE;
			if (!loader.Load(L"RPMUtils64.dll"))
				return FALSE;
		}
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

