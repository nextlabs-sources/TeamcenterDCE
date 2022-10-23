
#pragma once

#include <RMXLogger.h>
#include <string>

namespace RMXLogHlp
{
	const static wchar_t* LOG_CONFIGFILE = L"log4cplus.properties";

	static inline std::wstring GetConfigFile(HMODULE hRMXLib)
	{
		std::wstring configFile;
		wchar_t szLibPath[MAX_PATH];
		if (GetModuleFileName(hRMXLib, &szLibPath[0], MAX_PATH) != 0)
		{
			configFile = szLibPath;
			size_t pos = configFile.find_last_of(L"/\\");
			if (pos != std::wstring::npos)
			{
				configFile = configFile.substr(0, pos + 1);
				configFile += LOG_CONFIGFILE;
			}
			else
				configFile.clear();
		}

		return configFile;
	}

	class CallLogGuard
	{
	public:
		CallLogGuard(const std::wstring& funcName) : m_funcName(funcName)
		{
			LOG_TRACE(m_funcName.c_str() << L" Entering");
		}
		~CallLogGuard()
		{
			LOG_TRACE(m_funcName.c_str() << L" Leaving");
		}
	private:
		std::wstring m_funcName;
	};
} // namespace RMXLogHlp

#define RMXL_LOG_INITIALIZE(hRMXLib) LOG_INITIALIZE(RMXLogHlp::GetConfigFile(hRMXLib))
#define RMXL_LOG_ENTER RMXLogHlp::CallLogGuard apiCallGuard(LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()))


