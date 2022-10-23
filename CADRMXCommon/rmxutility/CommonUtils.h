#pragma once

#include <iostream>
#include <string>
#include <tchar.h>
#include <windows.h>

#define LOG_INFO(fmt, ...) \
{ \
	wchar_t sMsgBuf[1024]; \
	swprintf_s(sMsgBuf, 1024, fmt, __VA_ARGS__); \
	wcout << sMsgBuf << endl; \
}

#define LOG_INFOA(fmt, ...) \
{ \
	char sMsgBuf[1024]; \
	sprintf_s(sMsgBuf, 1024, fmt, __VA_ARGS__); \
	cout << sMsgBuf << endl; \
}

namespace Utils
{
	inline std::wstring s2ws(const std::string & str)
	{
		if (str.empty())
			return std::wstring();

		int len = (int)str.length() + 1;
		int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), len, 0, 0);
		wchar_t* buf = new wchar_t[size];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), len, buf, size);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}
}; 
