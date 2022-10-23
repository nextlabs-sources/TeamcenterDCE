#pragma once
#include <stdafx.h>
#include <shellapi.h>
#include <RunnerConfig.h>
#include <ProcessContext.h>

#define SHELL_OPEN L"open"
class ApplicationContext
{
public:
	static ApplicationContext ApplicationContext::FindAssocApp(const FileContext &fileContext)
	{
		wchar_t execPath[MAX_PATH] = TEXT("");
		const std::wstring &tarFile = reg_use_long_name(L"")
			? fileContext.target_long()
			: fileContext.target_path();
		HINSTANCE hResult = FindExecutable(tarFile.c_str(), NULL, execPath);
		if ((int)hResult >= 32)
		{
			DBGLOG("FindExecutable('%s')='%s'", tarFile.c_str(), execPath);
			if (!RunnerConfig::IsNxlApplication(execPath)) {
				return ApplicationContext(execPath, fileContext);
			}
		}
		else
		{
			DBGLOG("ERROR:FindExecutable(%s) return %u(LastError=%#x)", tarFile.c_str(), hResult, GetLastError());
		}
		DWORD size = MAX_PATH;
		auto realExt = fileContext.orig_ext().c_str();
		auto verbs = RunnerConfig::ExtensionOpenVerbs(realExt);
		for (size_t i = 0; i < verbs.size(); i++)
		{
			const wchar_t* verb = verbs[i].length() == 0 ? NULL : verbs[i].c_str();
			HRESULT r = AssocQueryString(0, ASSOCSTR_EXECUTABLE, realExt, verb, execPath, &size);
			if (S_OK != r)
			{
				DBGLOG("[%d]AssocQueryString(ASSOCSTR_EXECUTABLE, '%s', '%s') returns %#x(LastError=%#x)",
					i, realExt, verb, r, GetLastError());
				continue;
			}
			DBGLOG("[%d]AssocQueryString(ASSOCSTR_EXECUTABLE, '%s', '%s')='%s'",
				i, realExt, verb, execPath);
			if (!RunnerConfig::IsNxlApplication(execPath))
			{
				//found
				return ApplicationContext(execPath, fileContext, verbs[i]);
			}
		}
		//not found
		return ApplicationContext(RunnerConfig::ExtensionAssocApplication(fileContext.orig_ext()), fileContext);
	}
	static DWORD GetRegDWORDValue(HKEY rootKey, const std::wstring &keyPath, const std::wstring &valName, bool is64bit, DWORD defaultValue = 0)
	{
		DWORD dwValue = defaultValue;
		HKEY hKey;
		LONG returnStatus;
		REGSAM sam = KEY_QUERY_VALUE | (is64bit ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
		if ((returnStatus = RegOpenKeyExW(rootKey, keyPath.c_str(), 0, sam, &hKey)) == ERROR_SUCCESS)
		{
			DWORD dwSize = sizeof(DWORD);
			DWORD dwType = REG_DWORD;
			if ((returnStatus = RegQueryValueExW(hKey, valName.c_str(), NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) == ERROR_SUCCESS)
			{
				DBGLOG("%s:%s=%lu", keyPath.c_str(), valName.c_str(), dwValue);
			}
		}
		RegCloseKey(hKey);
		return dwValue;
	}
	static bool ApplicationContext::IsOfficeAddinActive(const std::wstring &appname)
	{
		//workaround fix for excel.exe
		if (wcsicmp(appname.c_str(), L"excel.exe") != 0)
			return true;
		/*
		//CurrentUser
		CurrentUserSubKeys.Add(@"Software\Microsoft\Office\Excel\Addins\NxlRmAddin");

		//LocalMachine
		LocalMachineSubKeys.Add(@"SOFTWARE\MICROSOFT\Office\Excel\Addins\NxlRmAddin");
		LocalMachineSubKeys.Add(@"SOFTWARE\Wow6432Node\Microsoft\Office\Excel\Addins\NxlRmAddin");

		//LocalMachineclickToRun
		LocalMachineSubKeys.Add(@"SOFTWARE\MICROSOFT\Office\ClickToRun\REGISTRY\MACHINE\SOFTWARE\Microsoft\Office\Excel\Addins\NxlRmAddin");
		LocalMachineSubKeys.Add(@"SOFTWARE\MICROSOFT\Office\ClickToRun\REGISTRY\MACHINE\SOFTWARE\Wow6432Node\Microsoft\Office\Excel\Addins\NxlRmAddin");
		*/
		if (GetRegDWORDValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Excel\\Addins\\NxlRmAddin", L"LoadBehavior", true, 3) != 3
			|| GetRegDWORDValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Office\\Excel\\Addins\\NxlRmAddin", L"LoadBehavior", true, 3) != 3
			|| GetRegDWORDValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Office\\Excel\\Addins\\NxlRmAddin", L"LoadBehavior", false, 3) != 3
			|| GetRegDWORDValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Office\\ClickToRun\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Office\\Excel\\Addins\\NxlRmAddin", L"LoadBehavior", true, 3) != 3
			|| GetRegDWORDValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Office\\ClickToRun\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Office\\Excel\\Addins\\NxlRmAddin", L"LoadBehavior", false, 3) != 3
			)
		{
			//if any of these values are not 3, don't set the process as trust
			return false;
		}
		return true;
	}
	void ApplicationContext::InitializeCommand(const wchar_t *ext) {
		auto cmd = reg_cmd(ext);
		if (!cmd.empty()) {
			auto args = reg_cmd_args(ext);
			if (args.empty()) {
				args = L"\"%1\"";
			}
			_cmd = cmd;
			_cmdArgs = FormatCmdArgs(args, _file);
			DBGLOG("==>CommandPath:%s", _cmd.c_str());
			DBGLOG("==>CommandArgs:%s", _cmdArgs.c_str());
		}

	}
	ApplicationContext::ApplicationContext(const std::wstring &app, const FileContext &fileContext, const std::wstring &defaultVerb = L"") :_appPath(app), _defaultVerb(defaultVerb)
	{
		if (_appPath.empty())
		{
			//no associate application is found for this file
			if (fileContext.is_nxl_file()) {
				_file = fileContext.nxl_file();
				InitializeCommand(L".nxl");
			}
			else
			{
				_file = fileContext.target_path();
			}
			return;
		}
		_appName = PathFindFileName(_appPath.c_str());

		if(fileContext.is_nxl_file())
		{
			//check if application is registered with RMX
			bool registered;
			if (!g_rpmHelper->IsAppRegistered(app, registered)
				|| !registered)
			{
				DBGLOG("no RMX is registered with '%s'", app.c_str());
				_file = fileContext.nxl_file();
				InitializeCommand(L".nxl");
				return;
			}
			else if (!IsOfficeAddinActive(_appName))
			{
				DBGLOG("NxlRmAddin for Office is not active");
				_file = fileContext.nxl_file();
				InitializeCommand(L".nxl");
				return;
			}
			else {
				g_rpmHelper->AddRPMFolder(fileContext.folder());
				fileContext.ListFiles();
				//TODO:check rights before start native appliaction??
				//session.SetRMXStatus(true);
				//session.GetFileRights(fileContext.target_path().c_str());
				//session.SetRMXStatus(false);
			}
		}

		//build target file path
		if (!fileContext.is_nxl_file())
		{
			//file is NOT protected, use original target file path
			_file = fileContext.target_path();
		}
		else if (reg_use_nxl_name(_appName))
		{
			//use nxl name
			_file = fileContext.nxl_file();
		}
		else if (reg_use_long_name(L"")
			|| reg_use_long_name(_appName))
		{
			//use long name without nxl
			_file = fileContext.target_long();
		}
		else
		{
			//use short name without nxl
			_file = fileContext.target_path();
		}

		//build command and args
		//check if specified command is defined for this application
		_cmd = reg_cmd(_appName);
		const std::wstring cmdArgsFmt = reg_cmd_args(_appName);
		if(!cmdArgsFmt.empty()) {
			_cmdArgs = FormatCmdArgs(cmdArgsFmt, _file);
			if (_cmd.empty()) {
				_cmd = _appPath;
			}
		}
		/*workaround for PhotoViewer.dll, deprecated after introduce new registry value - Command
		if(NULL == processContext.CommandPath
		&& _wcsicmp(PathFindExtension(processContext.ApplicationPath), L".dll")==0)
		{
		//to solve the image related issue
		//in some environment, image is opened by dllhost.exe which will not create new process for opening image
		//at here we specify rundll32.exe as the openning process
		wchar_t cmd[1024];
		DWORD size = 1024;
		HRESULT r = AssocQueryString(ASSOCF_NOTRUNCATE, ASSOCSTR_COMMAND, pFileContext->TargetFileExtension, NULL, cmd, &size);
		if(S_OK != r)
		{
		DBGLOG("ERROR:AssocQueryString(ASSOCSTR_COMMAND, '%s') Failed: %#x(LastError=%#x)",
		pFileContext->TargetFileExtension, r, GetLastError());
		}
		else
		{
		DBGLOG("AssociatedCommandLine='%s'", cmd);
		PWSTR pszApp;
		PWSTR pszCmdLine;
		PWSTR pszParameters;
		SHEvaluateSystemCommandTemplate(cmd, &pszApp, &pszCmdLine, &pszParameters);
		//DBGLOG("==>App='%s'", pszApp);
		//DBGLOG("==>Cmd='%s'", pszCmdLine);
		//DBGLOG("==>Parameters='%s'", pszParameters);
		processContext.CommandPath = _wcsdup(pszApp);
		processContext.CommandArgs = format_cmd_args(pszParameters, &processContext, pFileContext);
		//free
		CoTaskMemFree(pszApp);
		CoTaskMemFree(pszCmdLine);
		CoTaskMemFree(pszParameters);
		}
		}*/
		DBGLOG("==>ApplicationName:%s", _appName.c_str());
		DBGLOG("==>TargetFile:%s", _file.c_str());
		DBGLOG("==>CommandPath:%s", _cmd.c_str());
		DBGLOG("==>CommandArgs:%s", _cmdArgs.c_str());
	}
	std::wstring FormatCmdArgs(const std::wstring &fmt, const std::wstring &file)
	{
		if( fmt.empty())
			return std::wstring();

		//check if the cmd args is defined in registry
		wchar_t buffer[1024];
		DWORD size = 1024;
		//%1:target file full path
		//%2:application full path
		const wchar_t *args[] = { file.c_str(),  _appPath.c_str() };
		if (!FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			fmt.c_str(),
			0,
			0,
			buffer,
			size,
			(va_list*)args))
		{
			DBGLOG("Format message failed with %#x", GetLastError());
			return fmt;
		}
		else
		{
			DBGLOG("Formatted:%s", buffer);
			return buffer;
		}
	}
	bool StartByCreateProcess(ProcessContext *procContext) const
	{
		if (_cmd.empty() || _cmdArgs.empty())
		{
			DBGLOG("CommandArgs is not defined for application-'%s'", _appName.c_str());
			return false;
		}
		wchar_t cmdline[1000];
		wsprintf(cmdline, L"\"%s\" %s", _cmd.c_str(), _cmdArgs.c_str());
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		DBGLOG("CreateProcess('%s')...", cmdline);
		if (!CreateProcess(
			NULL,
			cmdline,
			NULL,
			NULL,
			false,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&si,
			&pi))
		{
			DWORD error = GetLastError();
			DBGLOG("CreateProcess() Failed!(Error=%lu(%#X))", error, error);
			return false;
		}
		DBGLOG("CreateProcess():pid=%lu tid=%lu", pi.dwProcessId, pi.dwThreadId);
		*procContext = ProcessContext(pi.dwProcessId, _cmd, pi.hProcess);
		procContext->SetAsTrusted(true);
		if (ResumeThread(pi.hThread) == -1)
		{
			DBGLOG("ResumeThread(hThread=%d) failed with error code %d", pi.hThread, GetLastError());
		}
		return true;
	}
	bool StartByShell(ProcessContext *procContext) const
	{
		//TODO: user CreateProcess with hook lib to start notepad.exe. ???
		SHELLEXECUTEINFO rSEI = { 0 };
		rSEI.cbSize = sizeof(rSEI);
		rSEI.lpVerb = NULL;
		rSEI.nShow = SW_NORMAL;
		rSEI.fMask = SEE_MASK_NOCLOSEPROCESS;

		if (_cmd.empty())
		{
			if (!_defaultVerb.empty())
				rSEI.lpVerb = _defaultVerb.c_str();
			rSEI.lpFile = _file.c_str();
			rSEI.lpParameters = NULL;
		}
		else
		{
			rSEI.lpFile = _cmd.c_str();
			rSEI.lpParameters = _cmdArgs.c_str();
		}
		DBGLOG("SHELLEXECUTEINFO.lpVerb:%s", rSEI.lpVerb);
		DBGLOG("SHELLEXECUTEINFO.lpFile:%s", rSEI.lpFile);
		DBGLOG("SHELLEXECUTEINFO.lpParams:%s", rSEI.lpParameters);

		if (!ShellExecuteEx(&rSEI)) // you should check for an error here
		{
			DBGLOG("ShellExecuteEx failed");
			return false;
		}
		if (rSEI.hProcess)
		{
			//wait for the process is initialized
			DWORD dwCode = WaitForInputIdle(rSEI.hProcess, 3000);
			if (dwCode != 0)
			{
				DBGLOG("WaitForInputIdle failed(%#x)", dwCode);
			}
			*procContext = ProcessContext(rSEI.hProcess);
			DBGLOG("Process-%d(%s) is going to open file-'%s'(IsSTA=%d)", procContext->pid(), procContext->pname().c_str(), _file.c_str(), procContext->IsSTA());
		}
		else
		{
			DBGLOG("No new process is created to open FIle-%s", _file.c_str());
		}
		return true;
	}
	ProcessContext Start() const
	{
		ProcessContext pContext;
		bool bStart = false;
		if (RunnerConfig::RunAsTrustApp(_appName))
		{
			bStart = StartByCreateProcess(&pContext);
		}
		//if not started by create process failed, try starting by shell
		if(!bStart)
		{
			bStart = StartByShell(&pContext);
		}
		if (!bStart)
		{
			return pContext;
		}
		if(pContext.pname().empty())
		{
			//failed to load process name from process handle, try to get it from other places
			if (!_cmd.empty())
			{
				//process is started by command line
				pContext.SetName(PathFindFileName(_cmd.c_str()));
				DBGLOG("Use CommandName-'%s' as processName", pContext.pname().c_str());
			}
			else if (!_appName.empty())
			{
				//process is started by shell
				pContext.SetName(_appName);
				DBGLOG("Use ApplicationName-'%s' as processName", pContext.pname().c_str());
			}
			else
			{
				DBGLOG("ERROR:CANNOT INFER PROCESS NAME");
			}
		}
		return pContext;
	}
	inline const std::wstring &app_name() const {
		return _appName;
	}
	inline const std::wstring &file() const {
		return _file;
	}
private:
	//info of the application to open the file
	std::wstring _appPath;
	std::wstring _appName;
	//info of the command to open the file
	std::wstring _defaultVerb;
	std::wstring _cmd;
	std::wstring _cmdArgs;
	//
	std::wstring _file;
};
