//#include "stdafx.h"
#include "hook_log.h"
#include <stdarg.h>
#include <time.h>
//#include <Shlwapi.h>
#include <stdio.h>
#include <Windows.h>

#define LOG_MSG_MAX (1024)


static const char *g_logFile = NULL;
const char *get_log_file()
{
	return g_logFile;
}

BOOL initial_log(const char *filePath)
{
	FILE *file = NULL;
	if(g_logFile != NULL)
	{
		nxl_log(TO_DBGVIEW, PROJECT_NAME"!WARN!Old log file exists: %s", g_logFile);
	}
	if((file = fopen(filePath, "w")) != NULL)
	{
		static char logFile[MAX_PATH];
		strcpy_s(logFile, MAX_PATH, filePath);
		nxl_log(TO_DBGVIEW, PROJECT_NAME"!New log file is created: %s", logFile);
		fclose(file);
		g_logFile = logFile;
		nxl_log(TO_DBGVIEW|TO_LOGFILE, TARGETFILENAME":Version="FILE_VER" Folder='%s'", get_utils_dir());
		return TRUE;
	}
	else
	{
		g_logFile = NULL;
		nxl_log(TO_DBGVIEW, PROJECT_NAME"!ERROR!Failed to create: '%s'", filePath);
		return FALSE;
	}
}

const char *build_message(const char *fmt, va_list args)
{
	static char msg[LOG_MSG_MAX + 1];
	//build message
	//format input message
	int size = vsnprintf_s(msg, sizeof(msg), LOG_MSG_MAX, fmt, args);
	//append terminating char
	if(size >= LOG_MSG_MAX||size < 0)
		msg[LOG_MSG_MAX] = '\0';
	return msg;
}

const char *build_timestamp(const char *msg)
{
	static char log[19 + 2 + LOG_MSG_MAX + 1 + 1];//time-19;[]-2;MSG;newline-1;NULL-1
	//write debug log to log file
	sprintf_s(log, sizeof(log), "[%s]%s\n", time_tostring(time(NULL)), msg);
	return log;
}

static const char *g_dbgLogFile = NULL;
void write_debug_message(const char *msg)
{
	if(g_dbgLogFile == NULL)
	{
		static char dbgLogFile[MAX_PATH] = "";
		char tmpFolder[MAX_PATH];
		if(GetTempPathA(MAX_PATH, tmpFolder) > 0)
		{
			const char *processName = NULL;
			DWORD pid;
#if defined(WNT)
			char buff[MAX_PATH];
			DWORD ret = GetModuleFileNameA(NULL, buff, MAX_PATH);
			if(ret >0 && ret < MAX_PATH)
			{
				processName = PathFindFileNameA(buff);
			}
			pid = GetCurrentProcessId();
#elif defined(__linux__)
			processName = program_invocation_short_name;
			pid = getpid();
#endif
			sprintf_s(dbgLogFile, MAX_PATH, "%s"PATH_DELIMITER"%s%lu.nxllog", tmpFolder, processName, pid);
			g_dbgLogFile = dbgLogFile;
		}
		else
		{
			return;
		}
	}
	writemsg(g_dbgLogFile, msg);
	writemsg(g_dbgLogFile, "\n");
}

void writemsg(const char *filePath, const char *msg)
{
	if(filePath!=NULL)
	{
		FILE *file = fopen(filePath, "a");
		if(file == NULL)
		{
			nxl_log(TO_DBGVIEW, PROJECT_NAME"!ERROR!Failed to Open log file-'%s' for appending", filePath);
		}
		else
		{
			fputs(msg, file);
			fclose(file);
		}
	}
}

const char *nxl_log(int targets, const char *fmt, ...)
{
	const char *msg;
	va_list args;

	va_start(args,fmt);
	msg = build_message(fmt, args);
	//END
	va_end(args);

	//output to console
	if(targets&TO_CONSOLE)
		printf("%s\n",msg);
	//output to DbgViewer
	if(targets&TO_DBGVIEW)
		OutputDebugStringA(msg);

	//output to log file
	if(g_logFile!=NULL
		&& targets&TO_LOGFILE)
	{
		writemsg(g_logFile, build_timestamp(msg));
	}
	return msg;
}

void init_nxl_scf_hook_log()
{
	//initial log file
	char logFile[MAX_PATH];
	if(GetTempPathA(MAX_PATH, logFile) > 0)
	{
		char logName[100] = "";
#if defined(WNT)
		sprintf_s(logName, 100, PROJECT_NAME".%lu.log", GetCurrentProcessId());
#elif defined(__linux__)
		sprintf_s(logName, 100, PROJECT_NAME".%d.log", getpid());
#endif
		strcat_s(logFile, MAX_PATH, logName);
		if(!initial_log(logFile))
		{
			ERRLOG("Create LogFile Failed('%s')", logFile);
		}
	}
	else
	{
		ERRLOG("GetTempPath Failed:%lu", GetLastError());
	}
}
