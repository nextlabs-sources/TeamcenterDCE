#include "stdafx.h"
#include "SwimUtils.h"
#include "FileUtils.h"
#include "CommonUtils.h"
#include <fstream>
#include "SwRMXMgr.h"
#include "SwimXMLParser.h"

namespace SwimUtils {

	static wstring S2_CacheFolderProperyValue = L"";

	bool  FindPropertyFile(const wstring& fileloc,wstring &swimPropertyFilePath) {
		wstring propertyFilePath = fileloc + L"\\" + L"swim.properties";
		if (!FileUtils::IsFileExists(propertyFilePath)) {
			propertyFilePath = fileloc + L"\\" + L".swimrc";
			if (!FileUtils::IsFileExists(propertyFilePath)) {
				return false;
			}
		}
		swimPropertyFilePath = propertyFilePath;
		return true;
	}

	
	void ReadSwimProperty(const wstring&fileloc, const wstring& property, wstring&value) {
		wstring swimPropertyFilePath;
		if (!FindPropertyFile(fileloc,swimPropertyFilePath))
			return;
		
		map<wstring, wstring> kvpMap;
		CommonUtils::GetAllKeyValueFromPropFile(swimPropertyFilePath, kvpMap);
		//Check if property exists in propList
		if (kvpMap.find(property) != kvpMap.end())
			value = kvpMap[property];

	}


	bool GetSwimPropertyValue(const wstring& property, wstring& value) {
		//This function reads property from swim.properties or .swimrc
		wstring env_val = RMXUtils::getEnv(L"SWIM_START_DIR");
		if (env_val.empty()) {
			env_val = RMXUtils::getEnv(L"USERPROFILE");
		}
		ReadSwimProperty(env_val, property, value);
		if (!value.empty()) {
			LOG_INFO_FMT("swim.property(or .swimrc) was read from the location = %s and workspaceDir is = %s", env_val.c_str(), value.c_str());
			return true;
		}

		env_val = RMXUtils::getEnv(L"USERPROFILE");
		ReadSwimProperty(env_val, property, value);
		if (!value.empty()) {
			LOG_INFO_FMT("swim.property(or .swimrc) was read from the location = %s and workspaceDir is = %s", env_val.c_str(), value.c_str());
			return true;
		}

		env_val = RMXUtils::getEnv(L"SWIM_DIR");
		if (env_val.empty()) {
			env_val = L"C:\\swim";
		}
		ReadSwimProperty(env_val, property, value);
		if (!value.empty()) {
			LOG_INFO_FMT("swim.property(or .swimrc) was read from the location = %s and workspaceDir is = %s", env_val.c_str(), value.c_str());
			return true;
		}
		return false;
	}


	DWORD GetSwimPID() {
		DWORD appPID = GetCurrentProcessId();

		//Get all the child process ids of SldWorks.exe
		vector<DWORD> appchpids = CommonUtils::getchpid(appPID);
		DWORD swimPPID = 0;
		for (auto pid : appchpids) {
			if (wcsstr(RMXUtils::GetProcessCommandLine(pid).c_str(), L"startswim.bat") != NULL){
				swimPPID = pid;
				break;
			}
		}

		vector<DWORD> chpids = CommonUtils::getchpid(swimPPID);
		for (auto pid : chpids) {
			if (wcsstr(RMXUtils::GetProcessCommandLine(pid).c_str(), L"MainSwim") != NULL) {
				return pid;
			}
		}
		return 0;
	}

	bool SetSwimPIDAsTrusted() {
		bool status = false;
		//Make java.exe corresponding to swim as trusted process
		DWORD swim_pid = GetSwimPID();
		//Set the swim_pid as trusted
		if (swim_pid != 0) {
			LOG_INFO("Setting java process associated with SWIM as trusted. Process Id = " << swim_pid);
			status = CSwRMXMgr::GetInstance()->SetProcessAsTrusted(swim_pid);
		}
		else {
			LOG_WARN("Process Id of java.exe corresponding to SWIM couldn't be found. Unable to set the process as trusted.");
		}

		return status;
	}

	void InitSwim() {
		LOG_INFO("Reading SW2_CacheFolder preference value...");
		wstring workspaceDir;
		bool found = SwimUtils::GetSwimPropertyValue(L"SW2_CacheFolder", workspaceDir);
		if (!found) {
			workspaceDir = RMXUtils::getEnv(L"USERPROFILE") + L"\\Siemens\\swcache";
			LOG_WARN("SW2_CacheFolder preference not found. Set it to default value: " << workspaceDir.c_str());
		}
		//Might have been specified in the form as : ${USERPROFILE}\\Siemens\\swcache
		workspaceDir = FileUtils::GetEnvVariablePath(workspaceDir);
		LOG_INFO("WorkspaceDir found:  " << workspaceDir.c_str());
		if (!FileUtils::IsFileExists(workspaceDir)) {
			LOG_WARN("WorkspaceDir not exists. Attempt to create: " << workspaceDir.c_str());	
			if (!FileUtils::CreateDirIfNotExists(workspaceDir)) {
				workspaceDir = RMXUtils::getEnv(L"USERPROFILE") + L"\\Siemens\\swcache";
				LOG_WARN("Failed to create WorkspaceDir. Set to default value = " << workspaceDir.c_str());
			}
		}
		
		workspaceDir = FileUtils::GetAbsolutePath(workspaceDir);
		S2_CacheFolderProperyValue = workspaceDir;
		if (!CSwRMXMgr::GetInstance()->RMX_IsRPMFolder(workspaceDir)) {
			LOG_INFO("Setting WorkspaceDir as RPM folder: " << workspaceDir.c_str());
			CSwRMXMgr::GetInstance()->RMX_AddRPMFolder(workspaceDir);
		}
		else {
			LOG_INFO_FMT("WorkspaceDir already set as an RPM folder: ", workspaceDir.c_str());
		}
	}

	bool RefreshSwimRegistry(const wstring& regFile) {
		SwimXMLParser parser;
		bool status = parser.UpdateSwimRegistry(RMXUtils::ws2s(regFile));
		return status;
	}

	wstring GetSW2CacheFolder() {
		return S2_CacheFolderProperyValue;
	}

}