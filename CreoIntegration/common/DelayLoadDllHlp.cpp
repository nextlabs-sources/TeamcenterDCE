
#include "DelayLoadDllHlp.h"
#include <Shlwapi.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>

#include "..\common\CreoRMXLogger.h"

#pragma comment(lib,"shlwapi.lib")

using namespace std;
std::vector<std::wstring> g_delayLoadDlls;

static std::vector<std::string> g_loadedDllNames;
FARPROC DelayLoadHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliStartProcessing:
		break;
	case dliNotePreLoadLibrary:
	{
		LOG_INFO_FMT(L"DelayLoadHook: '%s'", RMXUtils::s2ws(pdli->szDll).c_str());
		// Helper to load dll with the given path.
		for (const auto& dllPath : g_delayLoadDlls)
		{
			LPTSTR pszFileName = ::PathFindFileName(dllPath.c_str());
			string fileNameA = RMXUtils::ws2s(pszFileName);
			if (_stricmp(pdli->szDll, fileNameA.c_str()) == 0)
			{
				LOG_INFO_FMT(L"DelayLoadHook: Load library from : '%s'", dllPath.c_str());
				auto loadResult = ::LoadLibraryEx(dllPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
				if (loadResult != NULL)
					g_loadedDllNames.push_back(fileNameA);

				return (FARPROC)loadResult;
			}
		}
		break;
	}
	case dliNotePreGetProcAddress:
	case dliFailLoadLib:
	case dliFailGetProc:
	case dliNoteEndProcessing:
		break;
	}

	return NULL;
}

void UnloadDelayLoadedDlls()
{
	for (const auto& dllName : g_loadedDllNames)
	{
		if (!__FUnloadDelayLoadedDLL2(dllName.c_str()))
		{
			LOG_ERROR_FMT(L"DelayLoadHook: Cannot unload %s", RMXUtils::s2ws(dllName).c_str());
		}
	}
	g_loadedDllNames.clear();
}
