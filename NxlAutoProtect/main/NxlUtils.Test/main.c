#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "utils_log.h"
#include <utils_system.h>
#include <utils_string.h>

int main()
{
	result_t ret;
	INIT(ret);
	initial_log(gent_file_path("result.log"));
	OUTPUT("=======Running Test.....");
#ifdef DEBUG
	RUN(test_string_wildcmp(), ret);
#else
	RUN(test_log(), ret);
	RUN(test_string(), ret);
	RUN(test_string_list(), ret);
	RUN(test_kvl(), ret);
	RUN(test_hashtable(), ret);
	RUN(test_nxl(), ret);
	//RUN(test_jni(), ret);
	//RUN(test_evaluator(), ret);
	RUN(test_openaz(), ret);
#if defined(WNT)
	RUN(test_windows(), ret);
	//RUN(test_rmc(), ret);
#elif defined(__linux__)
	//TODO:add unit tests for linux apis
#endif
#endif

	ASSERT(MEM_ALLOCATED, 0, ret, __LINE__);
	OUTPUT("========Finished:Pass-%d Fail-%d", ret.npass, ret.nfail);
	getchar();
	return 0;
}
const char *gent_file_path(const char *name)
{
	static char currentDir[MAX_PATH] = "";
	static char tmpname[MAX_PATH];
	if(currentDir == NULL
		|| strlen(currentDir)==0)
	{
#if defined(WNT)
		if(!get_module_dir(TARGETFILENAME, currentDir, MAX_PATH))
#elif defined(__linux__)
		if(!get_lib_dir((void*)main, currentDir, MAX_PATH))
#endif
		{
			return NULL;
		}
	}
	sprintf(tmpname, "%s"PATH_DELIMITER"%s", currentDir, name);
	return tmpname;
}