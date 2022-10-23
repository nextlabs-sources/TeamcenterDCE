#pragma once

// Enable automation test framework in build.
// TODO: Turn off in RC builds.
#ifndef NDEBUG
#define ENABLE_AUTOTEST
#endif // !NDEBUG

#if defined(ENABLE_AUTOTEST)
#include <string>
#include <Windows.h>
#include <map>

#if defined(CREO_VER) && CREO_VER == 3
#define RMXTESTFWK_NAME L"CreoRMXTestFwk3_x64.dll"
#elif defined(CREO_VER) && CREO_VER == 4
#define RMXTESTFWK_NAME L"CreoRMXTestFwk4_x64.dll"
#elif  defined(CREO_VER) && CREO_VER == 7
#define RMXTESTFWK_NAME L"CreoRMXTestFwk7_x64.dll"
#endif

namespace XTestAPI
{
	inline HMODULE GetXTestModule()
	{
		std::wstring testLibName(RMXTESTFWK_NAME);
		HMODULE hXTest = GetModuleHandle(testLibName.c_str());
		return hXTest;
	}

	inline void NotifyXTest(const std::wstring& testFile, std::wstring& nxlOrigin, std::string& tags)
	{
		typedef void(*pfNotifyTestFile)(const wchar_t*, const wchar_t*, const char*);
	
		HMODULE hXTest = GetXTestModule();
		if (hXTest != nullptr)
		{
			auto pNotify = (pfNotifyTestFile)GetProcAddress(hXTest, "NotifyTestFile");
			if (pNotify)
			{
				pNotify(testFile.c_str(), nxlOrigin.c_str(), tags.c_str());
			}
		}
	}

	inline void DumpCachedNxlModel(std::map<std::wstring, std::wstring> nxlMdls)
	{
		typedef void(*pfDumpCachedNxlModels)(const std::map<std::wstring, std::wstring>& nxlMdls);

		HMODULE hXTest = GetXTestModule();
		if (hXTest != nullptr)
		{
			auto pFunc = (pfDumpCachedNxlModels)GetProcAddress(hXTest, "DumpCachedNxlModels");
			if (pFunc)
			{
				pFunc(nxlMdls);
			}
		}
	}
}
#endif