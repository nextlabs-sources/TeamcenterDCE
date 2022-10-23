#include <hook/hook_defs.h>
#include <hook/windows/windows_defs.h>
#include <NxlUtils/NxlPath.hpp>
#include <stdafx.h>

static pfDeleteFileW DeleteFileW_original;
static pfDeleteFileW DeleteFileW_next;
static BOOL deleteFileW_hooked = FALSE;

static BOOL DeleteFileW_hooked(
	LPCWSTR lpFileName)
{
	BOOL deleted = false;
	NxlPath filePath(lpFileName);
	if (filePath.CheckFileExists())
	{
		NxlPath nxlfile = filePath.EnsureNxlExtension();
		NxlPath oriFile = filePath.RemoveNxlExtension();
		if (nxlfile.CheckFileExists()
			&& oriFile.CheckFileExists())
		{
			bool isInRPMDir;
			DBGLOG("DeleteFileW(file='%s')...", filePath.tstr());
			if (g_rpmSession->IsFileInRPMDir(filePath.wstr(), &isInRPMDir) == 0
				&& isInRPMDir)
			{
				return g_rpmSession->RPMDeleteFile(oriFile.wstr()) == 0;
			}
		}
	}
	BOOL ret = DeleteFileW_next(lpFileName);
	if (!ret)
	{
		DWORD lasterror = GetLastError();
		DBGLOG("Failed(%#X) to delete file-'%s'", lasterror, filePath.str());
		SetLastError(lasterror);
	}
	return ret;
}


BOOL fix_rpm_deletefile_install()
{
	//HOOK(DeleteFileW);
	HMODULE mKernalbase = GetModuleHandle("kernelbase.dll");
	HOOK_byName(DeleteFileW, mKernalbase);
	return IS_HOOKED(DeleteFileW);
}

void fix_rpm_deletefile_uninstall()
{
	UNHOOK(DeleteFileW);
}