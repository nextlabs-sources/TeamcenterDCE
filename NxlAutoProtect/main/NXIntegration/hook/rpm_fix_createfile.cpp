#include <hook/hook_defs.h>
#include <hook/windows/windows_defs.h>
#include <NxlUtils/NxlPath.hpp>
#include <stdafx.h>

static pfCreateFileW CreateFileW_original;
static pfCreateFileW CreateFileW_next;
static BOOL createFileW_hooked = FALSE;

static HANDLE CreateFileW_hooked(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	//WORKaround:when opening a protected file without .nxl in lpFileName(there is .nxl on disk) in RPM folder with CREATE_ALWAYS, RPM return access denied
	if (dwCreationDisposition == CREATE_ALWAYS)
	{
		if (NxlPath::EndsWith(lpFileName, L".pdf"))//TODO:white list file extension
		{
			NxlPath filePath(lpFileName);
			DBGLOG("CreateFileW(file='%s', access=%#X share=%#X, disposition=%#X flags=%#X)", filePath.tstr(), dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes);
			bool isProtected;
			bool isInRPMDir;
			if (ASSERT_FILE_EXISTS(filePath)
				&& (g_rpmSession->IsFileInRPMDir(filePath.wstr(), &isInRPMDir) == 0 && isInRPMDir)//is in rpm folder
				&& (g_rpmSession->HelperCheckFileProtection(filePath.wstr(), &isProtected) == 0 && isProtected)) //is protected
			{
				filePath = filePath.Append(".nxl");
				DBGLOG("Reparsing to '%s'", filePath.tstr());
				HANDLE hnd = CreateFileW_next(filePath.wstr(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
				DWORD lasterror = GetLastError();
				DBGLOG("Handle-%p:'%s'", hnd, filePath.tstr());
				SetLastError(lasterror);
				return hnd;
			}
		}
	}
	return CreateFileW_next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


BOOL fix_rpm_createfile_install()
{
	//HOOK(CreateFileW);
	HMODULE mKernalbase = GetModuleHandle("kernelbase.dll");
	HOOK_byName(CreateFileW, mKernalbase);
	return IS_HOOKED(CreateFileW);
}

void fix_rpm_createfile_uninstall()
{
	UNHOOK(CreateFileW);
}