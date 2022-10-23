#include <io.h>
#include <fcntl.h>
#include <hook/hook_defs.h>
#include <hook/rmc_fixes.h>
#include <hook/windows/windows_defs.h>

/*
	fix the wrong file size issue when calling _stat64i32
	There is a process that has view right on a specific protected file
	when the process try to get the file size by _stat64i32 on the protected file
	_stat64i32 return the size of the protected file, instead of the size of decrypted content
	First found in visview.exe, visview use the size returned by _stat64i32 to read the file content
	because the size of protected file is bigger than the size of decrypted content, visview.exe failed the loading on the file
	Solution:
	redirect the calling _stat64i32 to _fstat which returned correct file size
*/
static pf_stat64i32 _stat64i32_original;
static pf_stat64i32 _stat64i32_next;
static int _stat64i32_hooked(const char * _Name, struct _stat64i32 * _Stat)
{
	int result = -1;
	int fd = -1;
	CALL_START("_stat64i32('%s')", _Name);	
	if(-1 == (fd = open(_Name,  _O_BINARY, _S_IREAD|_S_IWRITE)))
	{
		DBGLOG("Failed to open file-'%s'", _Name);
		CALL_NEXT(result = _stat64i32_next(_Name, _Stat));
	}
	else
	{
	   // Get data associated with "fd": 
		CALL_NEXT(result = _fstat(fd, _Stat));
		close(fd);
	}
	CALL_END("_stat64i32('%s') returns %d(st_size=%ld)", _Name, result, _Stat->st_size);	
	return result;
}

BOOL fix_stat64_install()
{
	HOOK_byName(_stat64i32, GetModuleHandle("msvcr100.dll"));
	return IS_HOOKED(_stat64i32);
}

void fix_stat64_uninstall()
{
	UNHOOK(_stat64i32);
}
