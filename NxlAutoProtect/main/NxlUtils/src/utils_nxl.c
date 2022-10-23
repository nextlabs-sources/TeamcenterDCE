#include <stdlib.h>
#include <stdio.h>
#include "utils_log.h"
#include "utils_nxl.h"
#include <utils_system.h>
#include <utils_string.h>


int IP_strtoi(char *ipstr)
{
	long num = 0;
	while(ipstr!=NULL)
	{
		char *end;
		long l = strtol(ipstr, &end, 10);
		DTLLOG("Found Number(%d) in '%s'  End='%s'", l, ipstr, end);
		if(ipstr!=end)
		{
			num =num*256 + l%256;
			DTLLOG("IP Number:%d", num);
		}
		if(*end == '\0')
			break;
		ipstr = ++end;
	}
	return num;
}

int IP_local_number()
{	
	static int ipNumber = 0;
	DBGLOG("Current IP Number(%d)!", ipNumber);
	if(ipNumber == 0)
	{
		ipNumber = IP_strtoi(IP_get_local());
	}
	return ipNumber;
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
char *search_dce_lib(const char *libName)
{
	char file[MAX_PATH] = "";
	//searching in current NxlUtils.dll folder
	sprintf(file, "%s" PATH_DELIMITER "%s", get_utils_dir(), libName);
	if(!file_exist(file))
	{
		char *dceRoot = NULL;
		//seaching in DCE installation folder
		DBGLOG("FileNotFound-'%s'", file);
		dceRoot = getenv(ENV_DCE_ROOT);
		if(dceRoot != NULL)
		{
			sprintf(file, "%s" PATH_DELIMITER "shared_libs" PATH_DELIMITER "%s", dceRoot, libName);
			if(!file_exist(file))
			{
				DBGLOG("FileNotFound-'%s'", file);
				return NULL;
			}
		}
		else
		{
			DBGLOG(ENV_DCE_ROOT" is not defined");
			return NULL;
		}
	}
	DBGLOG("Found-'%s'", file);
	return string_dup(file);
}