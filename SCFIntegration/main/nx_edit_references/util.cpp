#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include "log.h"
#include "util.h"
#include <time.h>

BOOL IsNxlFile(const char* szFilePath)
{
	if(_access(szFilePath,0)!=0)
		return FALSE;

	FILE *fp=NULL;
	errno_t errNum=fopen_s(&fp,szFilePath,"r");
	if(errNum!=0)
		return FALSE;
	char buff[8]="";
	size_t len=fread(buff,1,8,fp);
	fclose(fp);

	if(len!=8)
		return FALSE;

	if( buff[0]=='N'&&
		buff[1]=='X'&&
		buff[2]=='L'&&
		buff[3]=='F'&&
		buff[4]=='M'&&
		buff[5]=='T'&&
		buff[6]=='@')
	{
		return TRUE;
	}
	return FALSE;
}

BOOL IsFileExistingW(const WCHAR *pPath)
{
	if(0==_waccess(pPath,0))
		return TRUE;

	return FALSE;
}

BOOL IsFileExistingA(const char *pPath)
{
	if(0==_access(pPath,0))
		return TRUE;

	return FALSE;
}

const char *time_tostring(time_t time)
{
	static char timeBuf[19+1];//YYYY-mm-dd HH:MM:ss\0
	struct tm tm;
#if defined(WNT)
	localtime_s(&tm, &time);
#elif defined(__linux__)
	localtime_r(&time, &tm);
#endif
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

	return timeBuf;
}

const char *get_utils_dir()
{
	static char utilsDir[MAX_PATH] = {0};
	if(string_isNullOrSpace(utilsDir))
	{
#if defined(WNT)
		if(!get_module_dir(TARGETFILENAME, utilsDir, MAX_PATH))
#elif defined(__linux__)
		if(!get_lib_dir((void*)get_utils_dir, utilsDir, MAX_PATH))
#endif
		{
			utilsDir[0] = '\0';
		}
		else
		{
			//DBGLOG("NxlUitls Folder=%s", utilsDir);
		}
	}
	return utilsDir;
}

BOOL string_isNullOrSpace(const char *str)
{
	if(str!=NULL)
	{
		//is not NULL
		while(isspace((unsigned char)*str))
		{
			str++;
		}
		return (*str=='\0')?TRUE:FALSE;
	}
	return TRUE;
}

BOOL get_module_dir(const char *moduleName, char *dirBuf, int bufSize)
{
	if(!string_isNullOrSpace(moduleName))
	{
		HMODULE module;
		//initialize current module path
		if((module = GetModuleHandleA(moduleName)) != NULL)
		{
			int len  = GetModuleFileNameA(module, dirBuf, bufSize);
			if(len > 0)
			{
				//DBGLOG("GetModuleFileName=%s", dirBuf);
				PathRemoveFileSpecA(dirBuf);
				return TRUE;
			}
			else
			{
				ERRLOG("Failed to GetModuleFileName!(ErrorCode:%#X)", GetLastError());
			}
		}
		else
		{
			ERRLOG("Failed to GetModuleHandle(%s)!(ErrorCode:%#X)", moduleName, GetLastError());
		}
	}
	return FALSE;
}
