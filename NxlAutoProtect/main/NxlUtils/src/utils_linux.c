#ifdef __linux__
#include <errno.h>	//errono
#include <unistd.h>	//gethostname, getlogin
#include <netdb.h>	//gethostbyname
#include <dirent.h> //opendir readdir
#include <stdio.h> //popen

//inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//
#include <utils_system.h>
#include <utils_log.h>
#include <utils_mem.h>
#include <utils_string.h>

extern int h_errno;

user_info_ro get_logon_info()
{
	static struct _user_info_s *info = NULL;
	if(info == NULL)
	{
		const char *userName = getlogin();
		if(userName == NULL)
		{
			ERRLOG("Failed to get user name");
		}
		else
		{
			NXL_ALLOC(info, struct _user_info_s);
			info->name = string_dup(userName);
			info->sid = info->name;
			//set user name
			DBGLOG("User Name=%s", userName);
			MOCK_FREE(info);//cache;
		}
	}
	return info;
}
char *IP_get_local()
{
	char *ip = NULL;
	char nameBuf[255];
	struct hostent *hostInfo;
	int error;
	CALL_DTL((error = gethostname(nameBuf, sizeof(nameBuf))));
	if(error==0)
	{
		DBGLOG("Local Host Name : %s", nameBuf);
		CALL_DTL(hostInfo = gethostbyname(nameBuf));
		if(hostInfo != NULL && hostInfo->h_length > 0)
		{
			ip = inet_ntoa (*(struct in_addr *)*hostInfo->h_addr_list);
			DBGLOG("Local IP address is '%s'", ip);
		}
		else
		{
			ERRLOG("Failed to gethostbyname(%d)='%s'!", h_errno, hstrerror(h_errno) );
		}
	}
	else
	{
		DBGLOG("Failed to gethostname(%d)", errno );
	}
	return ip;
}

const char *host_get_name()
{
	static char hostname[MAX_PATH] = "";
	if(string_isNullOrSpace(hostname))
	{
		int error;
		if((error = gethostname(hostname, sizeof(hostname)))==0)
		{
			//DBGLOG("Local Host Name : %s", hostname);
		}
		else
		{
			DBGLOG("Failed(%d) to gethostname", error);
			hostname[0] = '\0';
		}
	}
	return hostname;
}
#include <dlfcn.h>
BOOL get_lib_dir(void *funcPtr, char *buf, int bufSize)
{
	if(funcPtr != NULL)
	{
		Dl_info dl_info;
		if(dladdr(funcPtr, &dl_info) != 0)
		{
			DBGLOG("[%p]FName='%s' SName='%s'", funcPtr, dl_info.dli_fname, dl_info.dli_sname);
			if(dl_info.dli_fname != NULL)
			{
				strcpy(buf, dl_info.dli_fname);
				return path_remove_name(buf);
			}
		}
	}
	return FALSE;
}

BOOL file_exist (const char *fileName)
{
	return fileName!=NULL && access(fileName, F_OK)==0;
}

string_list_p dir_list_files(const char *dirPath)
{
	DIR *pDir;
	struct dirent *pDirent;
	string_list_p pFiles = string_list_new();
	if (pDir = opendir(dirPath)) {
		while ((pDirent = readdir(pDir)) != NULL) {
			switch(pDirent->d_type)
			{
			case DT_DIR:
				DBGLOG("\t%s <dir>", pDirent->d_name);
				break;
			case DT_REG:
				DBGLOG("%s <reg>", pDirent->d_name);
				string_list_add(pFiles, pDirent->d_name);
				break;
			case DT_UNKNOWN:
			default:
				DBGLOG("%s <%d>", pDirent->d_name, pDirent->d_type);
				string_list_add(pFiles, pDirent->d_name);
				break;
			}
		}
		closedir(pDir);
	}
	return pFiles;
}
BOOL process_read_output(char *cmd, string_list_p lines, int timeout)
{
	FILE *fp = NULL;
	char buffer[1035];
	DBGLOG("calling '%s'", cmd);
	/* Open the command for reading. */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		ERRLOG("Failed to run command");
		return FALSE;
	}

	/* Read the output a line at a time - output it. */
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		DBGLOG("==>%s", buffer);
		if (lines != NULL) {
			string_list_add(lines, buffer);
		}
	}

	/* close */
	pclose(fp);
	DBGLOG("finished");
	return TRUE;
}
#endif
