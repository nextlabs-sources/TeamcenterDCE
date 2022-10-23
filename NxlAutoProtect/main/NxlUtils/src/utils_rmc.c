#include "utils_rmc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils_log.h"
#include "utils_mem.h"
#include <utils_system.h>
#include "utils_string.h"
#ifdef WNT
#include <Shlwapi.h>
#include "utils_nxl.h"
#include "internal.h"

#define RMC_SERVICE_NAME "nxrmserv"
#define RMC_CMD_NAME "nxrmconv.exe"

#define CMD_MAX_LEN 2048

BOOL RMC_is_installed()
{
	static int ret = FALSE;
	
	//check whether RMC is installed only when rms was not installed
	if(ret == FALSE)
	{
		service_info_t serviceInfo;
		if(get_service_info(RMC_SERVICE_NAME, QUERY_EXISTS, &serviceInfo)==0)
		{
			ret = serviceInfo.isInstalled;
		}
	}

    return ret;
}
BOOL RMC_is_running()
{
	service_info_t serviceInfo;
	if(get_service_info(RMC_SERVICE_NAME, QUERY_EXISTS|QUERY_STATUS, &serviceInfo)==0)
	{
		return serviceInfo.isInstalled && serviceInfo.isRunning;
	}
	return FALSE;
}

BOOL file_is_nxl(const char *filePath)
{
	if(file_exist(filePath))
	{
		if(string_ends_with(filePath, ".nxl", FALSE))
		{
			return TRUE;
		}
		else
		{
			char nxlFile[MAX_PATH];
			sprintf_s(nxlFile, MAX_PATH, "%s.nxl", filePath);
			return file_exist(nxlFile);
		}
	}
	return FALSE;
}

const char *helper_exe()
{
	static char *exe = NULL;
	if(exe == NULL)
	{
		exe = search_dce_lib("NxlHelper.exe");
	}
	return exe;
}
BOOL helper_is_protected(const char *file)
{
	BOOL isProtected = FALSE;
	if(file_exist(file))
	{
		const char *nxlhelper = helper_exe();
		if( nxlhelper != NULL )
		{
			char cmd[CMD_MAX_LEN+1];
			string_list_p lines = string_list_new();
			sprintf_s(cmd, CMD_MAX_LEN, "\"%s\" -isprotected \"%s\"", nxlhelper, file);
			if(process_read_output(cmd, lines, 200))
			{
				isProtected = stricmp(lines->items[0], "yes") == 0	//old nxlhelper
					|| string_starts_with(lines->items[0], "nxl"); //new nxlhelper supporting nxlv2;
			}
			string_list_free(lines);
		}
	}
	else
	{
		ERRLOG("FileNotFound-'%s'", file);
	}
	return isProtected;
}
//Nxl tags starts from 2k
#define NXL_SECTION_EXT 0x800
#define FILE_EXT_KEY L"$FileExt="
#define FILE_EXT_KEY_LEN 9
#define EXT_BUFF_SIZE 10
char *RMC_get_orig_ext(const char *file)
{
	char *ext = NULL;
	FILE *srcFile;
	int err;
	if(0==(err=fopen_s(&srcFile, file, "rb"))
		&& srcFile != NULL)
	{
		//skipped first 2k
		if(0==fseek(srcFile, NXL_SECTION_EXT, SEEK_SET))
		{
			wchar_t keyBuff[FILE_EXT_KEY_LEN + 1];
			if(fread(&keyBuff, sizeof(wchar_t), FILE_EXT_KEY_LEN, srcFile)>=0)
			{
				DBGLOG("==>'%S'", keyBuff);
				if(0 == wcsncmp(keyBuff, FILE_EXT_KEY, FILE_EXT_KEY_LEN))
				{
					wchar_t extBuf[EXT_BUFF_SIZE + 1];
					//read extension
					if(fread(&extBuf, sizeof(wchar_t), EXT_BUFF_SIZE, srcFile)>=0)
					{
						size_t wlen = wcslen(extBuf);
						if(wlen > 0)
						{
							size_t converted = 0;
							size_t extSize = EXT_BUFF_SIZE + 1;
							NXL_ALLOCN(ext, char, extSize);
							CALL_DTL(wcstombs_s(&converted, ext, extSize, extBuf, _TRUNCATE));
							DBGLOG("OrigFileExtension('%s')='%s'", file, ext);
						}
					}
				}
			}
		}
		fclose(srcFile);
	}
	else
	{
		DBGLOG("Failed to open - %s", file);
	}
	return ext;
}

const char *get_rmc_dir()
{
	static char *rmcDir = NULL;
	if(rmcDir == NULL)
	{
		service_info_t serviceInfo;
		if(get_service_info(RMC_SERVICE_NAME, QUERY_EXISTS|QUERY_CMD, &serviceInfo)==0)
		{
			if(serviceInfo.isInstalled)
			{
				if(string_starts_with(serviceInfo.cmd, "\""))
				{
					rmcDir = string_dup(serviceInfo.cmd+1);
				}
				else
				{
					rmcDir = string_dup(serviceInfo.cmd);
				}
				PathRemoveFileSpec(rmcDir);
				MOCK_FREE(rmcDir);
			}
		}
	}

    return rmcDir;
}
const char *get_rmc_cmd_exe()
{
	static BOOL initialized = FALSE;
	static char exe[MAX_PATH] = "";
	if(!initialized)
	{
		const char *rmcDir = get_rmc_dir();
		if(rmcDir != NULL)
		{
			//use RMC as default tool
			sprintf(exe, "%s\\%s", rmcDir, RMC_CMD_NAME);
			initialized = TRUE;
			DBGLOG("Tool.exe='%s'", exe);
		}
		else
		{
			return NULL;
		}
	}
	return exe;
}
static BOOL get_key_val(char *tagBuff, char **key, char **val)
{
	BOOL ret = FALSE;
	*key = NULL;
	*val = NULL;
	if(tagBuff != NULL)
	{
		char *pval = strchr(tagBuff, '=');
		if(pval != NULL)
		{
			pval[0] = '\0';
			pval++;
			//add key val to results
			*key = string_trim(tagBuff);
			*val = string_trim(pval);
			ret = TRUE;
		}
	}
	return ret;
}

BOOL helper_get_tags(const char *file, kvl_p tagList)
{
	if(file_exist(file))
	{
		const char *nxlhelper = helper_exe();
		if(file_exist(nxlhelper))
		{
			BOOL isProtected = FALSE;
			char cmd[CMD_MAX_LEN+1];
			string_list_p outputLines = string_list_new();
			sprintf_s(cmd, CMD_MAX_LEN, "\"%s\" -gettags \"%s\"", nxlhelper, file);
			if (process_read_output(cmd, outputLines, 200)
				&& outputLines->count > 0)
			{
				isProtected = stricmp(outputLines->items[0], "yes") == 0	//old nxlhelper
					|| string_starts_with(outputLines->items[0], "nxl"); //new nxlhelper supporting nxlv2;
				if(tagList != NULL)
				{
					int iline;
					char *key;
					char *val;
					for(iline = 1; iline < outputLines->count; iline++)
					{
						if(get_key_val(outputLines->items[iline], &key, &val))
						{
							//add key val to results
							DBGLOG("[%d]Key='%s' Val='%s'", iline, key, val);
							kvl_set_internal(tagList, key, val);
						}
						else
						{
							ERRLOG("[%d]Invalid Tag:'%s'", iline, outputLines->items[iline]);
						}
					}
				}
				else
				{
					ERRLOG("Input tagList is not initialized!");
				}
			}
			string_list_free(outputLines);
			return isProtected;
		}
		else
		{
			ERRLOG("FileNotFound-'%s'", nxlhelper);
		}
	}
	else
	{
		ERRLOG("FileNotFound-'%s'", file);
	}
	return FALSE;
}
int RMC_get_tags(const char *filePath, kvl_p list)
{
	if(file_exist(filePath))
	{
		const char *exe = get_rmc_cmd_exe();
		if(list != NULL
			&& file_exist(exe))
		{
			int ntags = 0;
			char cmd[CMD_MAX_LEN+1];
			string_list_p outputLines = string_list_new();
			sprintf_s(cmd, CMD_MAX_LEN, "\"%s\" readtag \"%s\"", exe, filePath);
			if (process_read_output(cmd, outputLines, 200))
			{
				int iline;
				char *key;
				char *val;
				for(iline = 0; iline < outputLines->count; iline++)
				{
					if(get_key_val(outputLines->items[iline], &key, &val))
					{
						//add key val to results
						DBGLOG("[%d]Key='%s' Val='%s'", ntags, key, val);
						kvl_set_internal(list, key, val);
						ntags++;
					}
					else
					{
						DBGLOG("Invalid Tag:'%s'", outputLines->items[iline]);
					}
				}
				if(ntags <= 0)
				{
					//TODO:verify RMC version
					DBGLOG("No tags are found from process output");
				}
			}
			string_list_free(outputLines);
			return ntags;
		}
		else
		{
			ERRLOG("FileNotFound-'%s'", exe);
		}
	}
	else
	{
		ERRLOG("FileNotFound-'%s'", filePath);
	}
	return -1;
}

BOOL RMC_protect(const char *filePath, kvl_ro tags)
{
	if(!file_exist(filePath))
	{
		ERRLOG("FileNotFound-'%s'", filePath);
		return FALSE;
	}
	if(tags != NULL)
	{
		const char *exe = get_rmc_cmd_exe();
		DTLLOG("Protecting %s  with %d tags...", filePath, tags->count);
		if(file_exist(exe))
		{
			char cmd[CMD_MAX_LEN+1];
			int i;
			int cmdlen = 0;
			//append exe with quotes
			cmdlen += sprintf_s(cmd + cmdlen, CMD_MAX_LEN - cmdlen, "\"%s\"", exe);
			//append target file with quotes
			cmdlen += sprintf_s(cmd + cmdlen, CMD_MAX_LEN - cmdlen, " protect \"%s\" /s", filePath);
			//append tags
			for(i=0; i < tags->count; i++)
			{
				char tag[MAX_PATH];
				int len = sprintf_s(tag, MAX_PATH, " /t \"%s\"=\"%s\"", tags->keys[i], tags->vals[i]);
				if((cmdlen+len) < CMD_MAX_LEN)
				{
					strcat_s(cmd, CMD_MAX_LEN, tag);
					cmdlen += len;
				}
				else
				{
					ERRLOG("The command exceeds the length limitation(%d) because of too many tags!", CMD_MAX_LEN);
					break;
				}
			}
			process_read_output(cmd, NULL, 1000);
			//check protect result
			if(file_is_nxl(filePath))
			{
				DBGLOG("==>Success");
				return TRUE;
			}
			if(RMC_is_running())
			{
				ERRLOG("==>Somehow file-%s is not protected(ret=%d)", filePath, i);
			}
			else
			{
				ERRLOG("==>file-%s is not protected(ret=%d), as RMC Service is not running", filePath, i);
			}
		}
		else
		{
			ERRLOG("Failed to locate RMC executable file: %s", exe);
		}
	}
	else
	{
		ERRLOG("Input Tags is not empty");
	}
	return FALSE;
}
#endif
#define NXL_FILE_HEADER "NXLFMT"
BOOL file_is_protected(const char *file)
{
	FILE *fileHandle;
	BOOL ret = FALSE;

	if (!file_exist(file)) return FALSE;

	if (0 == fopen_s(&fileHandle, file, "r")
		&& fileHandle != NULL)
	{
		char header[sizeof(NXL_FILE_HEADER)] = "";
		int nread = fread(header, sizeof(char), sizeof(header), fileHandle);
		DBGLOG("header(%s)=%d", file, nread);
		if (strncmp(header, NXL_FILE_HEADER, sizeof(NXL_FILE_HEADER) - 1) == 0)
		{
			ret = TRUE;
		}
		fclose(fileHandle);
	}
	return ret;
}