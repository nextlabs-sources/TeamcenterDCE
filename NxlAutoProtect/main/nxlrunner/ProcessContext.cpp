#include <ProcessContext.h>
#include <RunnerConfig.h>
#include <Shellapi.h>
#include <ntdll.h>

bool ProcessContext::Wait(const std::wstring & file)
{
	if (_name.empty()) return false;
	//process name is obtained successfully
	//get wait process name from registry(for example aclauncher.exe need to wait process acad.exe)
	const std::wstring childProcessName = RunnerConfig::WaitForChildProcess(_name);
	if (!childProcessName.empty())
	{
		ProcessContext childProcessContext(childProcessName);
		return childProcessContext.WaitFileClosed(file);
	}
	else if (IsSTA() || RunnerConfig::WaitForFileHandle(_name)) {
		//the application is STA, the process handle returned is not reliable, use wait file closed
		return WaitFileClosed(file);
	}
	else if (!IsHandleValid(_handle))
	{
		//no new process is created in shellexecute
		//wait file handle by process name
		return WaitFileClosed(file);
	}
	else
	{
		return WaitProcessExit();
	}

}

bool ProcessContext::WaitFileClosed(const std::wstring & file) const
{
	if (_name.empty()) return false;
	DBGLOG("Waiting Process('%s') close File-'%s'", full_name().c_str(), file.c_str());
	Sleep(3000);
	//FULL_SEARCH:
	std::vector<DWORD> pids;
	std::vector<HANDLE> phandles;
	DWORD maxTries = reg_max_try_times(_name);
	DWORD ntried = 0;
	std::vector<ProcessContext> inProcesses;
	if (_pid > 0 && !IsSTA()) {
		//if no pid defined AND process is NOT sta application
		inProcesses.push_back(*this);
	}
	else
	{
		//do a full search for which process is opening this file
		inProcesses = SearchProcessesByName(_name);
		if (inProcesses.empty())
		{
			//no process is found by specified name;
			return false;
		}
	}
	while (ntried < maxTries
		&& !inProcesses.empty())
	{
		DBGLOG("Trying(%lu/%lu) to search file handles in target %d process(Name='%s')", ntried, maxTries, inProcesses.size(), full_name().c_str());
		//process is found or process name is not defined
		std::vector<FileHandle> handles = SearchFileHandlersInProcesses(file, inProcesses);
		if (!handles.empty())
		{
			inProcesses.clear();
			//wait for all file handles closed
			for (auto fileHandle = handles.begin(); fileHandle != handles.end(); fileHandle++)
			{
				ProcessContext pContext(fileHandle->pid());
				DBGLOG("Handle-%#x(%s) is being opened in Process-%lu(%s)...", fileHandle->hnd(), fileHandle->name().c_str(), pContext.pid(), pContext.full_name().c_str());
				do
				{
					Sleep(3000);
					int nfailed = 0;
					DWORD error;
					while ((error = fileHandle->UpdateHandleName()) > 0
						&& (++nfailed) <= 3)
					{
						DBGLOG("[%d]UpdateHandleName(%#X) Failed: Error=%#X...", nfailed, fileHandle->hnd(), error);
						Sleep(100);
					}
				} while (fileHandle->IsHandleMatchedFile(file));
				DBGLOG("Handle-%#x(%s) is closed in Process-%lu('%s').", fileHandle->hnd(), fileHandle->name().c_str(), pContext.pid(), pContext.full_name().c_str());
				if (pContext.IsAlive())
				{
					DBGLOG("Process-%lu('%s') still running", pContext.pid(), pContext.full_name().c_str());
					if (std::find(inProcesses.begin(), inProcesses.end(), pContext.pid()) == inProcesses.end())
					{
						//the process still alive, we may do another search
						inProcesses.push_back(pContext);
					}
				}
			}
			//WORKAROUND:some application will open&close the file handle when user click save, e.g. powerpnt.exe
			//if the process is still alive, we re-start the search again
			maxTries = RunnerConfig::RetryIfProcesIsAlive(_name);
			if (!inProcesses.empty()
				&& maxTries > 0)
			{
				//reset retry times
				ntried = 0;
				Sleep(1000);
				continue;
			}
			else
			{
				return true;
			}
		}
		ntried++;
		Sleep(1000);
		//check if the processes are still alive
		for (auto it = inProcesses.begin(); it != inProcesses.end();) {
			if (it->IsAlive())
			{
				DBGLOG("Process-%lu('%s') still running", it->pid(), it->full_name().c_str());
				it++;
			}
			else
			{
				DBGLOG("Process-%lu('%s') is NOT running anymore", it->pid(), it->full_name().c_str());
				it = inProcesses.erase(it);
			}
		}
	}
	DBGLOG("File-%s is not used by any process", file.c_str());
	return false;
}

bool ProcessContext::WaitProcessExit()
{
	if (!IsHandleValid(_handle))
	{
		DBGLOG("Failed to get the process handler of the opening application....");
		return false;
	}
	DBGLOG("Waiting Process[%lu]%s exit...", _pid, full_name().c_str());

	while (TRUE) {

		DWORD nStatus = MsgWaitForMultipleObjects(
			1, &_handle, FALSE,
			INFINITE, QS_ALLINPUT   // drop through on user activity 
		);

		if (nStatus == WAIT_OBJECT_0) {  // done: the program has ended
			break;
		}
		DBGLOG("MsgWaitForMultipleObjects return %#X(Error=%#X)", nStatus, GetLastError());
		if (nStatus == WAIT_FAILED) {
			break;
		}
		Sleep(3000);
	}
	GetExitCodeProcess(_handle, &_exitCode);  // ERRORLEVEL value
	DBGLOG("Process[%lu]%s Exit(%lu)!", _pid, full_name().c_str(), _exitCode);
	return true;
}

bool ProcessContext::SetAsTrusted(bool external) const {
	//for safe 
	if (_name.empty() || _pid == 0) return false;
	DBGLOG("SetProcess([%d]'%s') as trusted...!", _pid, full_name().c_str());
	/* not only ugraf.exe need this, all other application in NX need this, e.g. cgm2pdf stp203 and son
	if (external
		&&_name != L"ugraf.exe") {
		DBGLOG("call from external only works for ugraf.exe");
#ifndef DEBUG
		return false;
#endif
	}
	*/
	bool success = g_rpmHelper->SetTrustedProcess(_pid, true);
	DBGLOG("AddTrustedProcess([%d]'%s') %s!", _pid, full_name().c_str(), success?L"success":L"failed");
	return success;
}

bool ProcessContext::RemoveTrust() const {
	//for safe 
	if (_name.empty() || _pid == 0) return false;
	DBGLOG("SetProcess([%d]'%s') as DIStrusted...!", _pid, full_name().c_str());
	bool success = g_rpmHelper->SetTrustedProcess(_pid, false);
	DBGLOG("RemoveTrustedProcess([%d]'%s') %s!", _pid, full_name().c_str(), success ? L"success" : L"failed");
	return success;
}

bool ProcessContext::IsAlive() const
{
	if (_pid <= 0)
		return false;
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, _pid);
	if (process == INVALID_HANDLE_VALUE) return false;
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}
