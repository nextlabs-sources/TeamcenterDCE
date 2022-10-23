#include "hook_utils.h"
#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include "hook_log.h"
#include <time.h>

const char *GetPathDirectory(const char *fullPath)
{
	static char dir[MAX_PATH];
	if(NULL!=fullPath)
	{
		int iLastBackSlash = strlen(fullPath);
		while(iLastBackSlash>=0 && fullPath[iLastBackSlash]!='\\')
		{
			iLastBackSlash--;
		}

		if(iLastBackSlash>=0)
		{
			strncpy_s(dir, MAX_PATH, fullPath, iLastBackSlash);
			return dir;
		}
	}
	return NULL;
}

const char *GetPathFileName(const char *fullPath, BOOL withExt)
{
	static char name[MAX_PATH];
	if(fullPath!=NULL)
	{
		int len = strlen(fullPath);
		int iLastBackSlash = len - 1;
		int iStart;
		int iEnd = len - 1;
		while(iLastBackSlash >= 0 && fullPath[iLastBackSlash] != '\\') iLastBackSlash--;
		iStart = iLastBackSlash + 1;
		if(!withExt)
		{
			//search last dot
			int iLastDot = len-1;
			while(iLastDot>=iStart && fullPath[iLastDot]!='.') iLastDot--;
			if(iLastDot > iStart) iEnd = iLastDot - 1;
		}
		if(iEnd >= iStart)
		{
			strncpy_s(name, MAX_PATH, fullPath+iStart, iEnd - iStart + 1);
			return name;
		}
	}
	return NULL;
}

BOOL StringEndsWithA(LPCSTR str, LPCSTR suffix, BOOL cs)
{
	if(str == NULL)
	{
		return suffix==NULL;
	}
	else if(suffix == NULL)
	{
		return TRUE;
	}
	else
	{
		size_t strLen = strlen(str);
		size_t sufLen = strlen(suffix);
		if(sufLen == 0)
		{
			return TRUE;
		}
		else if(strLen < sufLen)
		{
			return FALSE;
		}
		else if(cs
			&&strcmp((str+(strLen-sufLen)), suffix) == 0)
		{
			//case sensitive
			return TRUE;
		}
		else if(!cs
			&& _stricmp((str+(strLen-sufLen)), suffix)==0)
		{
			//case insensitive
			return TRUE;
		}
	}
	return FALSE;
}

BOOL StringEndsWithW(LPCWSTR str, LPCWSTR suffix, BOOL cs)
{
	if(str == NULL)
	{
		return suffix==NULL;
	}
	else if(suffix == NULL)
	{
		return TRUE;
	}
	else
	{
		size_t strLen = wcslen(str);
		size_t sufLen = wcslen(suffix);
		if(sufLen == 0)
		{
			return TRUE;
		}
		else if(strLen < sufLen)
		{
			return FALSE;
		}
		else if(cs
			&&wcscmp((str+(strLen-sufLen)), suffix) == 0)
		{
			//case sensitive
			return TRUE;
		}
		else if(!cs
			&& _wcsicmp((str+(strLen-sufLen)), suffix)==0)
		{
			//case insensitive
			return TRUE;
		}
	}
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
