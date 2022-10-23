// stdafx.cpp : source file that includes just the standard includes
// NxlTcImportIntegration.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


char sMsgBuf[1024];
int report_error(const char *file, int line, const char *func, const char *call, int ret)
{
	if (ret)
	{
		char errMsg[133];
		UF_get_fail_message(ret, errMsg);
		NXERR("%s():%s returns %d(Error:%s) @Line-%d in File-%s", func, call, ret, errMsg, line, file);
	}
	else
	{
		//NXDBG("%s():%s returns %d", func, call, ret);
	}

	return (ret);
}
