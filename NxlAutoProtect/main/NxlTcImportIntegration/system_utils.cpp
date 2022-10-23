#include <stdafx.h>
#include <utils_system.h>

#ifdef WNT

#include <Windows.h>
#include <Winternl.h>
#include <codecvt>

typedef NTSTATUS(*pfNtQueryInformationProcess)(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength
	);
static pfNtQueryInformationProcess NtQueryInformationProcess_dynamic = NULL;
NTSTATUS NtQueryInformationProcess_inner(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength)
{
	//initialize NtQueryObject
	if (NtQueryInformationProcess_dynamic == NULL)
	{
		HMODULE ntdllModule = GetModuleHandle("ntdll.dll");
		if (ntdllModule == NULL)
		{
			ntdllModule = LoadLibrary("ntdll.dll");
		}
		NtQueryInformationProcess_dynamic = (pfNtQueryInformationProcess)GetProcAddress(ntdllModule, "NtQueryInformationProcess");
		DBGLOG("ntdll Module=%p NtQueryInformationProcess=%p", ntdllModule, NtQueryInformationProcess_dynamic);
	}
	return NtQueryInformationProcess_dynamic(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
}

std::vector<std::string> load_cmd_args()
{
	std::vector<std::string> cmdArgs;
	HANDLE phandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	if (phandle != NULL)
	{
		//show command line information
		PROCESS_BASIC_INFORMATION baseInformation;
		NTSTATUS ntstatus = NtQueryInformationProcess_inner(phandle, ProcessBasicInformation, &baseInformation, sizeof(baseInformation), NULL);
		if (NT_SUCCESS(ntstatus))
		{
			PPEB peb = (PPEB)baseInformation.PebBaseAddress;
			if (peb != NULL)
			{
				PRTL_USER_PROCESS_PARAMETERS params = (PRTL_USER_PROCESS_PARAMETERS)peb->ProcessParameters;
				if (params != NULL)
				{
					DBGLOG("ImagePathName='%S'", params->ImagePathName.Buffer);
					int nargs = 0;
					WCHAR **args = CommandLineToArgvW(params->CommandLine.Buffer, &nargs);
						std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
					for (int iarg = 0; iarg < nargs; iarg++)
					{
						DBGLOG("CommandArgs[%d]='%S'", iarg, args[iarg]);
						cmdArgs.push_back(myconv.to_bytes(args[iarg]));
					}
				}
			}
		}
		else
		{
			DBGLOG("NtQueryInformationProcess_inner() returns %X", ntstatus);
		}
	}
	CloseHandle(phandle);
	return cmdArgs;
}
#else

std::vector<std::string> load_cmd_args()
{
	std::vector<std::string> args;
	//TODO:
	FILE *fp;
	char cmdlineFile[1024];
	sprintf(cmdlineFile, "/proc/%d/cmdline", getpid());
	DBGLOG("Reading cmdline args from '%s'...", cmdlineFile);
	fp = fopen(cmdlineFile, "r"); /* Open in BINARY mode */
	if (fp == NULL) {
		DBGLOG("open file failed");
		return args;
	}
	char buffer[1024];
	/* Read the entire file into contents */
	int nread = fread(buffer, sizeof(char), sizeof(buffer), fp);
	DBGLOG("read %d chars", nread);
	fclose(fp);
	int argOffset = 0;
	while (argOffset < nread) {
		char *arg = buffer + argOffset;
		int arglen = strlen(arg);
		DBGLOG("arg[%lu]:'%s'(len=%d)", args.size(), arg, arglen);
		args.push_back(arg);
		argOffset += arglen + 1;
	}
	return args;
}
#endif