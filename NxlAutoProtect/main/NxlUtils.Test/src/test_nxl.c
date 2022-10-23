#include "test.h"
#include "utils_nxl.h"
#include "utils_common.h"

result_t test_ip()
{
	result_t ret;
	int ipnum;
	INIT(ret);

	ipnum = IP_local_number();
	PASS(ret);
	return ret;
}


result_t test_nxl()
{
	result_t ret;
	INIT(ret);
	SUBRUN(test_ip(), ret);
	return ret;
}