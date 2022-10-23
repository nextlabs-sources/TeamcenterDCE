#pragma once
#pragma once

#include "stdafx.h"
namespace RegistryHelper {
	bool GetLongValFromReg(HKEY keyParent, CString keyName, CString keyValName, DWORD &dwVal);
	bool GetStringValFromReg(HKEY keyParent, CString keyName, CString keyValName, wstring &strVal);
	bool SetLongValToReg(HKEY keyParent, CString keyName, CString keyValName, DWORD dwValue);
	bool SetStringValToReg(HKEY keyParent, CString keyName, CString keyValName, CString value);
	bool OpenRegKey(HKEY keyParent, CString keyName, REGSAM samDesired, CRegKey &key);
	bool CreateKey(HKEY hKeyParent, LPCTSTR lpszKeyName, LPTSTR lpszClass = REG_NONE, DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		REGSAM samDesired = KEY_READ | KEY_WRITE, LPSECURITY_ATTRIBUTES lpSecAttr = NULL, LPDWORD lpdwDisposition = NULL);
};