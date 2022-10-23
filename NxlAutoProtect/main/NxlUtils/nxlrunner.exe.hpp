#pragma once
#include <sstream>
#include <vector>
#include <xstring>
#include <NxlUtils/NxlTool.hpp>
#include <NxlUtils/include/utils_windows.h>

class NxlRunner:public NxlTool {
public:
	static const NxlPath &GetExe()
	{
		static NxlPath nxlrunner;
		if (!nxlrunner.IsValid()
			|| !ASSERT_FILE_EXISTS(nxlrunner)) {
			static std::vector<NxlPath> searchPaths;
			if (searchPaths.empty()) {
				searchPaths.push_back(NxlPath(SysUtils::GetModuleDir()));
				searchPaths.push_back(NxlPath(SysUtils::GetEnvVariable(TEXT(ENV_PRODUCT_ROOT))).AppendPath(L"bin32"));
				searchPaths.push_back(NxlPath(SysUtils::GetRegistryValue(TEXT(REG_PRODUCT), TEXT("InstallDir"))).AppendPath(L"bin32"));
			}
			return nxlrunner = NxlTool::SearchExe(searchPaths, L"nxlrunner.exe");
		}
		return nxlrunner;
	}
	NxlRunner() {
		_toolPath = GetExe();
	}
	bool SetTrustApp(DWORD pid = 0) {
		if (!found()) return false;
		std::vector<std::wstring> args;
		args.push_back(L"-trust");
		std::wstringstream ss;
		if (pid == 0)
		{
			pid = GetCurrentProcessId();
		}
		ss << pid;
		args.push_back(ss.str());
		bool set = Execute(args);
		INFOLOG("Set Process-%lu as trust=%d", pid, set);
		return set;
	}
	bool DistrustApp(DWORD pid = 0) {
		if (!found()) return false;
		std::vector<std::wstring> args;
		args.push_back(L"-distrust");
		std::wstringstream ss;
		if (pid == 0)
		{
			pid = GetCurrentProcessId();
		}
		ss << pid;
		args.push_back(ss.str());
		bool set = Execute(args);
		INFOLOG("Set Process-%lu as distrusted=%d", pid, set);
		return set;
	}

};