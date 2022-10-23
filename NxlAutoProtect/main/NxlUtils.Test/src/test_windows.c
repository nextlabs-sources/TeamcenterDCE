#ifdef WNT
#include "test.h"
#include "utils_windows.h"

result_t test_service_info()
{
	service_info_t serviceInfo;
	int error = 0;
	result_t ret;
	INIT(ret);

	//
	error = get_service_info("dummyservice", QUERY_EXISTS, &serviceInfo);
	ASSERT(error, 1060, ret, __LINE__);
	
	error = get_service_info("Schedule", QUERY_EXISTS|QUERY_STATUS|QUERY_CMD, &serviceInfo);
	ASSERT(error, 0, ret, __LINE__);
	ASSERT(serviceInfo.isInstalled, TRUE, ret, __LINE__);
	ASSERT(serviceInfo.isRunning, TRUE, ret, __LINE__);
	ASSERT_STR(serviceInfo.cmd, "C:\\Windows\\system32\\svchost.exe -k netsvcs", ret, __LINE__);
	return ret;
}

result_t test_module_dir()
{
	char *cases[] = {"dummy.dll","kernel32.dll"};
	char *expects[] = {"", "C:\\Windows\\system32"};
	BOOL expectRets[] = {FALSE, TRUE};
	int icase;
	int ncase = sizeof(cases)/sizeof(char*);
	result_t ret;
	INIT(ret);
	for(icase=0; icase<ncase; icase++)
	{
		char dir[MAX_PATH] = {0};
		int success = get_module_dir(cases[icase], dir, MAX_PATH);
		ASSERT(success, expectRets[icase], ret, icase);
		ASSERT_STR(dir, expects[icase], ret, icase);
	}
	return ret;
}

result_t test_reg()
{
	char *keys[] = {NULL, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "SOFTWARE\\Microsoft\\Windows\\CurrentVersion","SOFTWARE\\Microsoft\\Windows\\CurrentVersion"};
	char *valkeys[] = {NULL, "ProgramFilesDir", NULL, ""};
	char *expects[] = {NULL, "C:\\Program Files", NULL, NULL};
	int n = sizeof(keys)/sizeof(char*);
	int i;
	result_t ret;
	INIT(ret);
	for(i=0; i<n; i++)
	{
		const char * val = REG_get(keys[i], valkeys[i]);
		ASSERT_STR(val, expects[i], ret, i);
	}
	return ret;
}

result_t test_windows()
{
	result_t ret;
	INIT(ret);
	SUBRUN(test_service_info(), ret);
	SUBRUN(test_module_dir(), ret);
	SUBRUN(test_reg(), ret);

	return ret;
}
#endif