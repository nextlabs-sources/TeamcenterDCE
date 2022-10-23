#pragma once
#include <stdafx.h>
#include <RunnerConfig.h>

class FileHandle {
public:
	FileHandle() :FileHandle(INVALID_HANDLE_VALUE, 0)
	{

	}
	FileHandle(HANDLE hnd, DWORD pid) :
		_fileHandle(hnd), _ownerPid(pid)
	{
		UpdateHandleName();
	}
	bool IsHandleMatchedFile(const std::wstring &file) const;
	inline bool IsValid() const
	{
		return _fileHandle != 0
			&& _fileHandle != INVALID_HANDLE_VALUE
			&& _ownerPid > 0;
	}
	inline DWORD pid() const { return _ownerPid; }
	inline HANDLE hnd() const { return _fileHandle; }
	inline const std::wstring &name()const { return _handleName; }
	inline const std::wstring &type() const { return _handleType; }
	DWORD UpdateHandleName();
	inline bool IsFileHandle() const {
		return wcsicmp(_handleType.c_str(), L"File") == 0;
	}
private:
	HANDLE _fileHandle;
	DWORD _ownerPid;
	std::wstring _handleName;
	std::wstring _handleType;
};
inline bool IsHandleValid(HANDLE hnd) {
	return hnd != NULL && hnd != INVALID_HANDLE_VALUE;
}
class ProcessContext {
public:
	static std::wstring QueryProcessFullName(HANDLE hnd);
	static std::wstring QueryProcessFullName(DWORD pid);
	ProcessContext() :_handle(INVALID_HANDLE_VALUE) {}
	ProcessContext(DWORD pid):ProcessContext(pid, L"") {
		if(pid>0)
			SetFullName(QueryProcessFullName(pid));
	}
	ProcessContext(HANDLE hnd) :_handle(hnd) {
		if (!IsHandleValid(hnd)) return;
		_pid = GetProcessId(hnd);
		SetFullName(QueryProcessFullName(hnd));
	}
	ProcessContext(const std::wstring& name) :_name(name), _handle(INVALID_HANDLE_VALUE) {}
	ProcessContext(DWORD pid, const std::wstring &fullName, HANDLE handle = INVALID_HANDLE_VALUE) : _pid(pid), _handle(handle) {
		SetFullName(fullName);
	}
	inline bool IsValid() const {
		return _pid > 0 || IsHandleValid(_handle) || !_name.empty();
	}
	bool Wait(const std::wstring &file);
	bool WaitFileClosed(const std::wstring &file) const;
	bool WaitProcessExit();
	bool SetAsTrusted(bool external)const;
	bool RemoveTrust() const;
	bool IsAlive() const;
	inline DWORD pid() const {
		return _pid;
	}
	inline const std::wstring &pname() const {
		return _name;
	}
	inline void SetName(const std::wstring &pname) {
		_name = pname;
	}
	inline HANDLE hnd() const {
		return _handle;
	}
	inline DWORD exit_code()const {
		return _exitCode;
	}
	bool operator==(DWORD pid) const {
		return _pid == pid;
	}
	inline void SetFullName(const std::wstring &fullName) {
		if (!fullName.empty())
		{
			_fullName = fullName;
			_name = PathFindFileName(fullName.c_str());
		}
	}
	inline const std::wstring &full_name() const {
		return _fullName.empty() ? _name : _fullName;
	}
	inline bool IsSTA() const {
		return RunnerConfig::AppIsSTA(_name);
	}
	static std::vector<FileHandle> SearchFileHandlersInProcesses(const std::wstring &fileName, const std::vector<ProcessContext> &processes);
private:
	static std::vector<ProcessContext> SearchProcessesByName(const std::wstring &processName);
	std::wstring _name;
	std::wstring _fullName;
	DWORD _pid = 0;
	HANDLE _handle= INVALID_HANDLE_VALUE;
	DWORD _exitCode = 0;

};