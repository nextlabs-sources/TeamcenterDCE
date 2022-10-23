#pragma once
#include <NxlUtils/NxlPath.hpp>
#include <sstream>

class NxlTool {
public:
	inline DWORD error() const { return _errorCode; }
	inline bool found() const { return _toolPath.IsValid(); }
	static NxlPath SearchExe(const std::vector<NxlPath> &searchPaths, const std::wstring &exename)
	{
		for (int i = 0; i < searchPaths.size(); i++)
		{
			auto exe = searchPaths[i].AppendPath(exename);
			if (exe.CheckFileExists()) {
				DBGLOG("[%d]'%s'(Found)", i, exe.tstr());
				return exe;
			}
			else
			{
				DBGLOG("[%d]'%s'(FileNotFound)", i, exe.tstr());
			}
		}
		return NxlPath();
	}
	bool CreateReadPipe(DWORD bufferSize, HANDLE *pReadPipe, HANDLE *pWritePipe)
	{
		if (pReadPipe == nullptr || pWritePipe == nullptr) return false;

		HANDLE hReadPipe = NULL;
		HANDLE hWritePipe = NULL;
		SECURITY_ATTRIBUTES saAttr;
		//Create Read and WritePipes for STDOUT of child process
		// Set the bInheritHandle flag so pipe handles are inherited. 
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;
		// Create a pipe for the child process's STDOUT.  
		if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, bufferSize))
		{
			ERRLOG("CreatePipe Failed(%lu)", GetLastError());
			goto CLOSE_PIPES;
		}
		// Ensure the read handle to the pipe for STDOUT is not inherited.
		if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
		{
			ERRLOG("SetHandleInformation Failed(%lu)", GetLastError());
			goto CLOSE_PIPES;
		}
		*pReadPipe = hReadPipe;
		*pWritePipe = hWritePipe;
		return true;
	CLOSE_PIPES:
		_errorCode = GetLastError();
		if (hReadPipe != NULL)
		{
			CloseHandle(hReadPipe);
		}
		if (hWritePipe != NULL)
		{
			CloseHandle(hWritePipe);
		}
		return false;
	}
	bool StartProcess(const NxlString &cmd, HANDLE hReadPipe, HANDLE hWritePipe)
	{
		BOOL bSuccess = FALSE;
		//starting process
		PROCESS_INFORMATION piProcInfo;
		STARTUPINFOW siStartInfo;

		// Set up members of the PROCESS_INFORMATION structure.  
		ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
		// Set up members of the STARTUPINFO structure. 
		// This structure specifies the STDIN and STDOUT handles for redirection. 
		ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdError = hWritePipe;//GetStdHandle(STD_ERROR_HANDLE);
		siStartInfo.hStdOutput = hWritePipe;
		//siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
		siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
		siStartInfo.wShowWindow = SW_HIDE;

		// Create the child process.     
		bSuccess = CreateProcessW(NULL,
			const_cast<LPWSTR>(cmd.wchars()),     // command line 
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			0,             // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&siStartInfo,  // STARTUPINFO pointer 
			&piProcInfo);  // receives PROCESS_INFORMATION 

						   //https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx
						   //It is important for the parent process to close its handle to the write end of the pipe before calling ReadFile
		// If an error occurs, exit the application. 
		if (!bSuccess)
		{
			_errorCode = GetLastError();
			ERRLOG("CreateProcess('%s') Failed(%lu)", cmd.tchars(), _errorCode);
			return false;
		}
		DBGLOG("New Process Created:%lu('%s')", piProcInfo.dwProcessId, cmd.tchars());
		// Close handles to the child process and its primary thread.
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
		return true;
	}
	bool Execute(const std::vector<std::wstring> &args, std::vector<std::string> *pOutputLines = nullptr) {
		if (!found()||!ASSERT_FILE_EXISTS(_toolPath)) return false;
		//build command
		std::wstringstream wss;
		wss << L"\"" << _toolPath.wstr() << L"\"";
		for (int i = 0; i < args.size(); i++) {
			wss << L" " << args[i];
		}
		auto cmd = wss.str();
		HANDLE hReadPipe = NULL;
		HANDLE hWritePipe = NULL;


		if (!CreateReadPipe(0, &hReadPipe, &hWritePipe)) return false;
		bool ret = StartProcess(cmd, hReadPipe, hWritePipe);
		//https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx
		//It is important for the parent process to close its handle to the write end of the pipe before calling ReadFile
		LOG_CALLED(CloseHandle(hWritePipe));
		if (ret)
		{
			//DBGLOG("Reading Pipe...");
	#define PIPE_BUFFER_SIZE 1024
			char readBuf[PIPE_BUFFER_SIZE + 1] = { 0 };
			DWORD nread;
			while (ReadFile(hReadPipe, readBuf, PIPE_BUFFER_SIZE, &nread, NULL))
			{
#ifdef UNICODE
				DBGLOG("ReadPipe(%d):'%S'", nread, readBuf);
#else
				DBGLOG("ReadPipe(%d):'%s'", nread, readBuf);
#endif
				if (pOutputLines != nullptr)
				{
					std::stringstream ss;
					for (DWORD i = 0; i <= nread; i++)
					{
						if (i == nread || readBuf[i] == '\r' || readBuf[i] == '\n') {
							//new string
							auto s = ss.str();
							if (!s.empty())
							{
								pOutputLines->emplace_back(std::move(s));
							}
							ss.clear();
						}
						else {
							ss << readBuf[i];
						}
					}
					TRACELOG("nlines=%d", pOutputLines->size());
				}
				//reset buffer
				memset(readBuf, 0, sizeof(readBuf));
			}
			switch (GetLastError())
			{
				case ERROR_SUCCESS:
					TRACELOG("ReadFile() finished SUCCESS");
					break;
				case ERROR_BROKEN_PIPE:
					DBGLOG("ReadFile() finished as pipe closed");
					break;
				default:
					DBGLOG("ReadFile() failed with %lu(%#X)", GetLastError());
					break;
			}
		}
		LOG_CALLED(CloseHandle(hReadPipe));
		return ret;
	}
protected:
	NxlTool() :_errorCode(0) {};
	NxlPath _toolPath;
	DWORD _errorCode;
};
