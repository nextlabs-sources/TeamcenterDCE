#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <xstring>
#include <NxlUtils/NxlPath.hpp>
#include <NxlUtils/RMXUtils.hpp>

#define __WTEXT(quote) L##quote
#define WTEXT(quote) __WTEXT(quote)

class NxlDllLoader {
public:
	virtual bool Load(const std::wstring &dllName, bool is32bit=false) {
		HMODULE module = GetModuleHandleW(dllName.c_str());
		if (TryLoadFromSameFolder(dllName, module)
			|| TryLoadFromRMXRoot(dllName, is32bit?L"bin32":L"bin64", module)
			|| TryLoadFromRMXRoot(dllName, L"bin", module)) {
			wchar_t dllpath[MAX_PATH];
			if (GetModuleFileNameW(module, dllpath, MAX_PATH) == 0)
			{
				OutputMessage(std::wstring(L"GetModuleFileNameW(") + dllName + L") failed", GetLastError());
			}
			else
			{
				OutputMessage(std::wstring(L"Loaded '") + dllName + L"' as '" + dllpath + L"'");
			}
			return true;
		}
		return false;
	}
	virtual void OutputMessage(const std::wstring &msg, DWORD code = 0) {
		if (code > 0) {
			std::wstringstream ss;
			ss << L"Error-" << code << ":" << msg;
			OutputDebugStringW(ss.str().c_str());
		}
		else
		{
			OutputDebugStringW(msg.c_str());
		}
	}
	bool TryLoadFromSameFolder(const std::wstring &dllName, HMODULE &module) {
		if (module != NULL) return true;
		HMODULE thisModule = GetModuleHandleW(WTEXT(TARGETFILENAME));
		if (thisModule == NULL)
		{
			OutputMessage(L"GetModuleHandle(" WTEXT(TARGETFILENAME) L") failed", GetLastError());
			return false;
		}
		wchar_t dllpath[MAX_PATH];
		if (GetModuleFileNameW(thisModule, dllpath, MAX_PATH) == 0)
		{
			OutputMessage(L"GetModuleFileNameW(" WTEXT(TARGETFILENAME) L") failed", GetLastError());
			return false;
		}
		PathRemoveFileSpecW(dllpath);
		SetDllDirectoryW(dllpath);
		module = LoadLibraryW(dllName.c_str());
		SetDllDirectoryW(NULL);
		if (module == NULL) {
			OutputMessage(std::wstring(L"LoadLibraryW(") + dllName + L") from '" + dllpath + L"' failed", GetLastError());
			return false;
		}
		return true;
	}
	bool TryLoadFromRMXRoot(const std::wstring &dllName, const std::wstring &subfolder, HMODULE &module) {
		if (module != NULL) return true;
		NxlPath dllDir = NxlPath(RMXUtils::getRmxRoot());
		if (!subfolder.empty()) {
			dllDir = dllDir.AppendPath(subfolder);
		}
		SetDllDirectoryW(dllDir.wchars());
		module = LoadLibraryW(dllName.c_str());
		DWORD error = GetLastError();
		SetDllDirectoryW(NULL);
		if (module == NULL) {
			OutputMessage(std::wstring(L"LoadLibraryW(")+ dllName + L") from '" + dllDir.wstring() + L"' failed", error);
			return false;
		}
		return true;
	}
};