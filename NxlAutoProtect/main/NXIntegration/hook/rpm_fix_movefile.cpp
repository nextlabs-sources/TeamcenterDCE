#include <hook/hook_defs.h>
#include <hook/windows/windows_defs.h>
#include <NxlUtils/NxlPath.hpp>
#include <stdafx.h>


static pfMoveFileExW MoveFileExW_original;
static pfMoveFileExW MoveFileExW_next;

static
BOOL
MoveFileExW_hooked(
	__in     LPCWSTR lpExistingFileName,
	__in_opt LPCWSTR lpNewFileName,
	__in     DWORD    dwFlags
)
{
	NxlPath srcFile(lpExistingFileName);
	if (!srcFile.HasNxlExtension()
		&& !ASSERT_FILE_EXISTS(srcFile))
	{
		NxlPath nxlfile = srcFile.Append(".nxl");
		if (ASSERT_FILE_EXISTS(nxlfile))
		{
			//TODO:the reparse should be done for the newly protected file only, otherwise may affect existing file wihch has same name
			NxlPath targetFile = NxlPath(lpNewFileName).EnsureNxlExtension();
			if (targetFile.CheckExist())
			{
				DBGLOG("Deleting '%s'", targetFile.tstr());
				g_rpmSession->RPMDeleteFile(targetFile.wstr());
			}
			DBGLOG("Reparse MoveFileExW('%s', '%s', %#X)...", nxlfile.tstr(), targetFile.tstr(), dwFlags);
			return MoveFileExW_next(nxlfile.wstr(), targetFile.wstr(), dwFlags);
		}
	}
	return MoveFileExW_next(lpExistingFileName, lpNewFileName, dwFlags);
}


BOOL fix_rpm_movefile_install()
{
	HMODULE mKernalbase = GetModuleHandle("kernelbase.dll");
	//MoveFileEx not implemented in kernelbase.dll of win7
	//hook movefileexw in kernelbase.dll, if failed try kernel32.dll
	HOOK_byName(MoveFileExW, mKernalbase);
	HOOK(MoveFileExW);
	return IS_HOOKED(MoveFileExW);
}

void fix_rpm_movefile_uninstall()
{
	UNHOOK(MoveFileExW);
}