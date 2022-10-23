#include "hook_windows_defs.h"
#include "hook_log.h"
#include <io.h>
#include <fcntl.h>
#include "hook_defs.h"


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

BOOL visview_install()
{
	HOOK_byName(_stat64i32, GetModuleHandle("msvcr100.dll"));
	return IS_HOOKED(_stat64i32);
}

void visview_uninstall()
{
	UNHOOK(_stat64i32);
}