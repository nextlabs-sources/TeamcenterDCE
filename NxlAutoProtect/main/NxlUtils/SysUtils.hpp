#pragma once
#include <Windows.h>
#include "NxlString.hpp"
#include "utils_log4cpp.hpp"

class SysUtils {
public:
	static nxl::tstring GetEnvVariable(const TCHAR* envName) {
		TCHAR szPath[MAX_PATH];
		DWORD code = GetEnvironmentVariable(envName, szPath, MAX_PATH);
		if (code != 0) {
			return nxl::tstring(szPath);
		}
		else
		{
			ERRLOG("GetEnvironmentVariable('%s') failed-%lu(%#X)", envName, GetLastError(), GetLastError());
		}
		return nxl::tstring();
	}
	static nxl::tstring GetRegistryValue(const TCHAR* keyPath, const TCHAR* valKey) {
		HKEY hKey;
		TCHAR lszValue[MAX_PATH] = TEXT("");
		LSTATUS retCode;
		if ((retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey)) == ERROR_SUCCESS)
		{
			DWORD dwType = REG_SZ;
			DWORD dwSize = MAX_PATH;
			if ((retCode = RegQueryValueEx(hKey, valKey, NULL, &dwType, (LPBYTE)&lszValue, &dwSize)) != ERROR_SUCCESS)
			{
				ERRLOG("RegQueryValueEx('HKLM\\%s','%s') failed(%ld)", keyPath, valKey, retCode);
				lszValue[0] = '\0';
			}
			RegCloseKey(hKey);
		}
		else
		{
			ERRLOG("RegOpenKeyEx('HKLM\\%s') failed(%ld)", keyPath, retCode);
		}
		DBGLOG("HKLM\\%s:%s='%s'", keyPath, valKey, lszValue);
		return nxl::tstring(lszValue);
	}
	static const nxl::tstring& GetModuleDir()
	{
		static nxl::tstring dir;
		if (dir.empty())
		{
			HMODULE module;
			//initialize current module path
			if ((module = GetModuleHandle(TEXT(TARGETFILENAME))) != NULL)
			{
				TCHAR dirBuf[MAX_PATH];
				int len = GetModuleFileName(module, dirBuf, MAX_PATH);
				if (len > 0)
				{
					//DBGLOG("GetModuleFileName=%s", dirBuf);
					PathRemoveFileSpec(dirBuf);
					dir = nxl::tstring(dirBuf);
					DBGLOG("ModuleDir='%s'", dir.c_str());
				}
				else
				{
					ERRLOG("Failed to GetModuleFileName!(ErrorCode:%#X)", GetLastError());
				}
			}
			else
			{
				ERRLOG("Failed to GetModuleHandle(%s)!(ErrorCode:%#X)", TEXT(TARGETFILENAME), GetLastError());
			}
		}
		return dir;
	}
};
