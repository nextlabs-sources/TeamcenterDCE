#ifndef NXL_UTILS_TEST_H_INCLUDED
#define NXL_UTILS_TEST_H_INCLUDED
#include <stdio.h>
#include <string.h>
#include "utils_log.h"
#include "utils_mem.h"
typedef struct result_s
{
	int npass;
	int nfail;
}result_t, *result_p;

const char *gent_file_path(const char *name);

#define OUTPUT(fmt, ...) writemsg(gent_file_path("result.log"), nxl_log(TO_CONSOLE, fmt, ##__VA_ARGS__));writemsg(gent_file_path("result.log"), "\n");

#define INIT(r) {r.npass=0; r.nfail=0;}
#define ADD(a, b) {a.npass += b.npass; a.nfail += b.nfail;}
#define PASS(r) r.npass++
#define FAIL(r) r.nfail++

#define RUN(x, r) {result_t rrr;initial_log(gent_file_path(#x".log"));SCOPE_START; rrr = x;OUTPUT(#x" Finished:Pass-%d Fail-%d", rrr.npass, rrr.nfail);ADD(r, rrr);SCOPE_END;}
#define SUBRUN(x, r) {result_t rrr = x;SCOPE_START;OUTPUT(#x" Finished:Pass-%d Fail-%d", rrr.npass, rrr.nfail);ADD(r, rrr);SCOPE_START;}
#define ASSERT(act, exp, r, cid) if((act)==(exp)){PASS(r);}else{OUTPUT("%s():[%d]Actual("#act")=%d Expect("#exp")=%d(Line=%d)", __FUNCTION__, cid, act, exp, __LINE__); FAIL(r);}
#define ASSERT_STR(act, exp, r, cid) if(act==NULL&&exp==NULL){PASS(r);}else if(act!=NULL&&exp!=NULL&&strcmp((act),(exp))==0){PASS(r);}else{FAIL(r);OUTPUT("%s():[%d]Actual=%s Expect=%s(Line=%d)", __FUNCTION__, cid, act, exp, __LINE__);}

#define MEM_ALLOCATED monitor_allocated()
//Done:
result_t test_hashtable();
result_t test_jni();
result_t test_log();
result_t test_nxl();//need add more cases
result_t test_string();//Partially
result_t test_string_wildcmp();
result_t test_evaluator();
result_t test_openaz();
result_t test_windows();
result_t test_rmc();
result_t test_string_list();
result_t test_kvl();
//TODO
result_t test_time();
#endif