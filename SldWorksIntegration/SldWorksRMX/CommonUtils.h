#pragma once

#if ! defined (COMMONUTILS_H)
#define COMMONUTILS_H

#include <string>

namespace CommonUtils
{
	
	bool ExecuteCmd(const std::wstring& cmd);
	vector<DWORD> getchpid(DWORD ppid);
	std::wstring ltrim(const wstring& str);
	std::wstring rtrim(const wstring& str);
	bool IsLineEmpty(const wstring& str);
	bool IsLineComment(const wstring& str);
	bool ParsePropertyLine(const wstring& str, pair<wstring, wstring>& kvp);
	void GetAllKeyValueFromPropFile(const wstring& propertyFile, map<wstring, wstring>& kvpMap);

}; // namespace CommonUtils

#endif // COMMONUTILS_H

