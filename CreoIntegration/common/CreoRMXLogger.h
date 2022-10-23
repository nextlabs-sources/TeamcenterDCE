
#pragma once

#if !defined(LOG4CPLUS_ENABLE) 
#define LOG4CPLUS_ENABLE
#endif

#include <RMXLogger.h>
#include <RMXUtils.h>
#include <string>

namespace RMXLogHlp
{
	const static wchar_t* ENV_CREO_RMX_ROOT = L"CREO_RMX_ROOT";
	const static wchar_t* LOG_CONFIGFILE = L"log4cplus.properties";

	static inline std::wstring GetConfigFile()
	{
		std::wstring configFile = RMXUtils::getEnv(ENV_CREO_RMX_ROOT);
		if (!configFile.empty())
		{
			if (configFile.find_last_of(L"/\\") != configFile.length() - 1)
			{
				configFile += L"\\";
			}
			configFile += LOG_CONFIGFILE;
		}

		return configFile;
	}

	class CallLogGuard
	{
	public:
		CallLogGuard(const std::wstring& domain, const std::wstring& funcName, const std::wstring& msg = std::wstring())
			: m_domain(domain), m_funcName(funcName), m_msg(msg)
		{
			LOG_DEBUG_FMT(L"Entering %s - %s %s", m_domain.c_str(), m_msg.c_str(), m_funcName.c_str());
		}
		~CallLogGuard()
		{
			LOG_DEBUG_FMT(L"Exiting %s - %s %s", m_domain.c_str(), m_msg.c_str(), m_funcName.c_str());
		}
	private:
		std::wstring m_domain;
		std::wstring m_funcName;
		std::wstring m_msg;
	};
} // namespace RMXLogHlp

#define CREOLOG_INITIALIZE() LOG_INITIALIZE(RMXLogHlp::GetConfigFile())

// Scope exit guard logs
#define OTKCALL_LOG_ENTER RMXLogHlp::CallLogGuard otkLogGuard(L"CREO call", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()));
#define IPEMRMX_LOG_ENTER RMXLogHlp::CallLogGuard ipemLogGuard(L"IPEMRMX call", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()));
#define UC_LOG_ENTER(msg) RMXLogHlp::CallLogGuard ucLogGuard(L"USAGECONTROL", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()), msg)
#define PC_LOG_ENTER RMXLogHlp::CallLogGuard pcLogGuard(L"PROTECTCONTROL", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()));
#define RPMDIR_LOG_ENTER(msg) RMXLogHlp::CallLogGuard rpmdirLogGuard(L"RPMDIRCONTROL", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()), msg)
