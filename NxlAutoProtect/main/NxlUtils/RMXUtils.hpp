#pragma once
#include "utils_log4cpp.hpp"
#include "NxlString.hpp"
#include "SysUtils.hpp"
#include <Shlwapi.h>

//TODO:make this as singleton
class RMXUtils
{
public:
	static const nxl::tstring& getRmxRoot()
	{
		static nxl::tstring rmxRoot;
		if (rmxRoot.empty()) {
			rmxRoot = SysUtils::GetEnvVariable(TEXT("NX_RMX_ROOT"));//TODO:make configurable
			if (rmxRoot.empty())
			{
				rmxRoot = SysUtils::GetRegistryValue(TEXT("Software\\NextLabs\\NXRMX"), TEXT("InstallDir"));//TODO:make configurable
			}
			//TODO:TBD:what if still empty?
		}
		return rmxRoot;
	}
	static const nxl::tstring& getSessionId() {
#define ENV_RMX_SESSION_ID "RMX_SESSION_ID"
		static nxl::tstring idString;
		if (idString.empty()) {
			TCHAR buffer[MAX_PATH];
			DWORD pid = GetCurrentProcessId();
			_stprintf(buffer, TEXT("%lu"), pid);
			idString = GetOrSetEnvironmentVariable(TEXT(ENV_RMX_SESSION_ID), buffer);
		}
		return idString;
	}
	static const nxl::tstring& getSessionName() {
#define ENV_RMX_SESSION_NAME "RMX_SESSION_NAME"
		static nxl::tstring nameString;
		if (nameString.empty()) {
			TCHAR buffer[MAX_PATH];
			const TCHAR* pname = TEXT("");
			int len = GetModuleFileName(NULL, buffer, MAX_PATH);
			if (len > 0)
			{
				pname = PathFindFileName(buffer);
				nameString = GetOrSetEnvironmentVariable(TEXT(ENV_RMX_SESSION_NAME), pname);
			}
			else
			{
				nameString = SysUtils::GetEnvVariable(TEXT(ENV_RMX_SESSION_NAME));
			}

		}
		return nameString;
	}
	static nxl::tstring GetOrSetEnvironmentVariable(const TCHAR* envName, const TCHAR* defaultValue) {
		TCHAR buffer[MAX_PATH] = TEXT("");
		auto value = SysUtils::GetEnvVariable(envName);
		if (value.empty()) {
			//not defined yet
			if (SetEnvironmentVariable(envName, defaultValue))
			{
				INFOLOG("Set %s=%s", envName, defaultValue);
				value = defaultValue;
			}
			else
			{
				ERRLOG("Set %s=%s failed(error=%#X)", envName, defaultValue, GetLastError());
			}
		}
		else
		{
			INFOLOG("%s=%s DefaultValue=%s", envName, value.c_str(), defaultValue);
		}
		return value;
	}
	static bool initializeLogger() {
#define LOG4CPLUS_PROPERTIES "log4cplus.properties"
		log4cplus::initialize();
		{
			auto propFileName = WTEXT(LOG4CPLUS_PROPERTIES);
			NxlPath searchDirs[] = { NxlPath(SysUtils::GetModuleDir()), NxlPath(RMXUtils::getRmxRoot()) };
			for (int i = 0; i < sizeof(searchDirs) / sizeof(NxlPath); i++) {
				auto propFile = searchDirs[i].AppendPath(propFileName);
				if (propFile.CheckFileExists())
				{
					//log4cplus::PropertyConfigurator::doConfigure(propFile.tstr());
					//TRACELOG("Loaded log4cplus properties files from-%s", propFile.tstr());
					log4cplus::helpers::Properties props(propFile.tstr());
					props.setProperty(TEXT(ENV_RMX_SESSION_ID), RMXUtils::getSessionId());
					props.setProperty(TEXT(ENV_RMX_SESSION_NAME), RMXUtils::getSessionName());
					log4cplus::PropertyConfigurator conf(props, log4cplus::Logger::getDefaultHierarchy(), log4cplus::PropertyConfigurator::PCFlags::fShadowEnvironment);
					conf.configure();
					//output info
					TRACELOG("Loaded '%s' sid='%s' sname='%s'"
						, propFile.tstr(), RMXUtils::getSessionId().c_str(), RMXUtils::getSessionName().c_str());
					break;
				}
				else
				{
					WARLOG("Failed locate-%s in module file directory-'%s'", TEXT(LOG4CPLUS_PROPERTIES), searchDirs[i].tstr());
				}
			}
			//TODO:set default logger
		}
		return true;
	}
};
