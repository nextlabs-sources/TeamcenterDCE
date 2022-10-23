#include <hook/hook_defs.h>
#include <hook/libugmr.h>
#include <stdio.h>
#include <uf.h>
#include <time.h>

void nx_log(const char *fmt, ...)
{
	char buf[1024];
	
	//add timestamp
	struct tm tm;
	time_t t = time(NULL);
	localtime_s(&tm, &t);
	size_t tlen = strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", &tm);

	//format message	
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(buf+tlen, sizeof(buf) - tlen,  sizeof(buf) - tlen, fmt, args);
	va_end(args);

	//print log to NX syslog
	UF_print_syslog(buf, FALSE);
}
#define NXLOG(fmt, ...) nx_log(PROJECT_NAME "!" fmt "\n", ##__VA_ARGS__)

#define NXL_FILE_HEADER "NXLFMT"
void fix_nxl_extension(const char *fileName)
{
	//verify if file name is correct and exist
	if(fileName == NULL 
		|| GetFileAttributes(fileName) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}
	//verify if file name already ends with .nxl extension
	size_t fileNameLen = strlen(fileName);
	if(fileNameLen > 4
		&& _stricmp(fileName + fileNameLen - 4, ".nxl") == 0)
	{
		//input file ends with .nxl extension
		return;
	}
	//
	char nxlFile[MAX_PATH];
	sprintf_s(nxlFile, MAX_PATH, "%s.nxl", fileName);
	if(GetFileAttributes(nxlFile) != INVALID_FILE_ATTRIBUTES)
	{
		//file has hidden .nxl extension
		return;
	}
	//check if the file content is protected
	//Use in-process file read to check whether the file is protected or not
	//if the file is protected and doesn't have .nxl extension, RMC won't decrypt for NX
	//the NXLFMT! header can be read correctly
	FILE *fileHandle;
	BOOL isprotected = FALSE;
	if(0== fopen_s(&fileHandle, fileName, "r")
		&& fileHandle != NULL)
	{
		char header[sizeof(NXL_FILE_HEADER)] = "";
		size_t nread = fread(header, sizeof(char), sizeof(header), fileHandle);
		NXLOG("'%s':header(%d)=%s", fileName, nread, header);
		if(strncmp(header, NXL_FILE_HEADER, sizeof(NXL_FILE_HEADER)-1) == 0)
		{
			isprotected = TRUE;
		}
		fclose(fileHandle);
	}
	if(!isprotected)
	{
		return;
	}
	//append .nxl extension on protected file
	if(MoveFile(fileName, nxlFile))
	{
		NXLOG("Renamed '%s' to '%s'", fileName, nxlFile);
	}
	else
	{
		NXLOG("Rename '%s' to '%s' failed(Error:%lu)", fileName, nxlFile, GetLastError());
	}
}

/*
	FCC_DownloadFilesFromPLM
*/
static pfFCC_DownloadFilesFromPLM FCC_DownloadFilesFromPLM_original = NULL;
static pfFCC_DownloadFilesFromPLM FCC_DownloadFilesFromPLM_next = NULL;
static int FCC_DownloadFilesFromPLM_hooked (
	const char *policy,
	int nUids,
	const char **uids,
	CALLBACK_PTR calback,
	const void *clientObject,
	char ** const localFiles,			//INPUT ARRAY for output
	int * const fails					//INPUT ARRAY for output
	)
{
	int i, ret;
	DWORD tid = GetCurrentThreadId();
	NXLOG("[%lu]HOOK-START:FCC_DownloadFilesFromPLM(policy='%s', nUids=%d)", tid, policy, nUids);

	try
	{
		ret = FCC_DownloadFilesFromPLM_next(policy, nUids, uids, calback, clientObject, localFiles, fails);
	}
	catch(...)
	{
		NXLOG("[%lu]Exception happened during calling FCC_DownloadFilesFromPLM_next", tid);
	}
	for(i=0; i<nUids; i++)
	{
		NXLOG("[%lu]==>[%d/%d]Uid='%s' File='%s' Fail=%d", tid, i+1, nUids, uids[i], localFiles[i], fails[i]);
		fix_nxl_extension(localFiles[i]);
	}
	
	NXLOG("[%lu]HOOK-END:FCC_DownloadFilesFromPLM(policy='%s', nUids=%d) returns %d", tid, policy, nUids, ret);
	return ret;
}
/*
	FCC_DownloadFilesToLocation
*/
static pfFCC_DownloadFilesToLocation FCC_DownloadFilesToLocation_original = NULL;
static pfFCC_DownloadFilesToLocation FCC_DownloadFilesToLocation_next = NULL;
static int FCC_DownloadFilesToLocation_hooked(
	char const * policy,
	int nUids,
	char const ** const uids,
	CALLBACK_PTR callback,
	void const * clientobject,
	char const * directoryPath,
	char const * * const localFiles,	//OUTPUT ARRAY
	int * const fails					//OUTPUT ARRAY
	)
{
	int i, ret;
	DWORD tid = GetCurrentThreadId();
	NXLOG("[%lu]HOOK-START:FCC_DownloadFilesToLocation(policy='%s', nUids=%d directory='%s')", tid, policy, nUids, directoryPath);

	try
	{
		ret = FCC_DownloadFilesToLocation_next(policy, nUids, uids, callback, clientobject, directoryPath, localFiles, fails);
	}
	catch(...)
	{
		NXLOG("[%lu]Exception happened during calling FCC_DownloadFilesFromPLM_next", tid);
	}
	for(i=0; i<nUids; i++)
	{
		NXLOG("[%lu]==>[%d/%d]Uid='%s' File='%s' Fail=%d", tid, i+1, nUids, uids[i], localFiles[i], fails[i]);
		fix_nxl_extension(localFiles[i]);
	}
	
	NXLOG("[%lu]HOOK-END:FCC_DownloadFilesToLocation(policy='%s', nUids=%d directory='%s') return %d", tid, policy, nUids, directoryPath, ret);
	return ret;
}

void fix_fcc_download()
{
	//NOTE:These two apis may be running in multiple threading
	//therefore the hook code must be thread-safe
	HOOK_API("libugmr.dll", FCC_DownloadFilesFromPLM);
	HOOK_API("libugmr.dll", FCC_DownloadFilesToLocation);
}