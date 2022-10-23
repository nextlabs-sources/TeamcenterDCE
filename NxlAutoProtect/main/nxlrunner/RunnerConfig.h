#pragma once
#include <stdafx.h>


//reg utils
std::wstring reg_get_chars_value(const std::wstring &appName, const std::wstring &valName);
DWORD reg_get_dword_value(const std::wstring &appName, const std::wstring &valName, DWORD defaultValue);
bool reg_get_multi_chars(const std::wstring &appName, const std::wstring &valName, std::vector<std::wstring> &values);

#define reg_cmd(appName) reg_get_chars_value(appName, L"Command")
#define reg_cmd_args(appName) reg_get_chars_value(appName, L"CommandArgs")
#define reg_max_try_times(appName) reg_get_dword_value(appName, L"MaxTryOfHandleSearch", 1)
#define reg_use_nxl_name(appName) (reg_get_dword_value(appName, L"UseNxlName", 0)>0)
#define reg_use_long_name(appName) (reg_get_dword_value(appName, L"UseLongName", 0)>0)

#define NXL_RUNNER_EXE L"nxlrunner.exe"
#define RMC_HANDLER L"nxrmshell.dll"
class RunnerConfig
{
public:
	static bool IsNxlApplication(const std::wstring &exePath) {
		if (!exePath.empty())
		{
			std::vector<std::wstring> nxlapps;
			if (reg_get_multi_chars(L"", L"NxlApps", nxlapps))
			{
				for (auto app : nxlapps)
				{
					if (app.length() > 0
						&& app.length() <= exePath.length())
					{
						if (wcsicmp(exePath.substr(exePath.length() - app.length(), app.length()).c_str(), app.c_str()) == 0)
						{
							DBGLOG("!!!SELF REFERNCING-'%s'", app.c_str());
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	static std::wstring ExtensionAssocApplication(const std::wstring &ext) {
		return reg_get_chars_value(ext, L"AssocApplication");
	}
	static std::vector<std::wstring> ExtensionOpenVerbs(const std::wstring& ext) {
		std::vector<std::wstring> verbs;
		reg_get_multi_chars(ext, L"OpenVerbs", verbs);
		if(std::find(verbs.begin(), verbs.end(), L"open")==verbs.end())
			verbs.push_back(L"open");
		if (std::find(verbs.begin(), verbs.end(), L"") == verbs.end())
			verbs.push_back(L"");
		return verbs;
	}
	static bool RunAsTrustApp(const std::wstring &appname) {
		return reg_get_dword_value(appname, L"RunAsTrustApp", 0) > 0;
	}
	//Note:when application is STA, the handle return from shell is not reliable
	//1, we need wait for file handle
	//2, when searching the file handle, should search by the process name instead of the process handle
	static bool AppIsSTA(const std::wstring &appName) {
		return reg_get_dword_value(appName, L"IsSTAApp", 0);
	}
	static bool WaitForFileHandle(const std::wstring &appName) {
		return reg_get_dword_value(appName, L"WaitForFileHandle", 0) > 0;
	}
	//When application is creating a new child process to open the file
	//1, we need search the process by children process name
	//2, wait the file handle by searching in children process
	static std::wstring WaitForChildProcess(const std::wstring &appName) {
		return reg_get_chars_value(appName, L"WaitForChildProcess");
	}

	static DWORD RetryIfProcesIsAlive(const std::wstring &appName)
	{
		return reg_get_dword_value(appName, L"RetryIfProcesIsAlive", 0);
	}

};

