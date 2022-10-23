#ifndef NXL_UTILS_MEM_H_INCLUDED
#define NXL_UTILS_MEM_H_INCLUDED
#include <stdlib.h>
#include <string.h>
#include "nxl_utils_exports.h"

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI void *monitor_track(void *p, const char *name, const char *file, const char *func, int line);
NXLAPI void *monitor_retrack(void *oldPtr, void *newPtr, const char *name, const char *file, const char *func, int line);
NXLAPI void monitor_track_array(void **ps, int n, const char *name, const char *file, const char *func, int line);
NXLAPI void monitor_free(void *p, const char *name, const char *file, const char *func, int line);
NXLAPI void monitor_reset();
NXLAPI unsigned long monitor_allocated();
NXLAPI unsigned long monitor_list();

#ifdef __cplusplus
}
#endif

//Memory related
#ifndef DEBUG_MEM
#define NXL_ALLOC(ptr, t) (ptr = (t*)malloc(sizeof(t)))
#define NXL_ALLOCN(ptr, t, n) (ptr = (t*)malloc(sizeof(t)*(n)))
#define NXL_ALLOCS(ptr, t, size) (ptr = (t)malloc(size))
#define NXL_REALLOC(ptr, t, n) (ptr = (t*)realloc(ptr, sizeof(t)*(n)))
//for the string which is returned from NxlUtils, please use string_free() instead of NXL_FREE()
#define NXL_FREE(param) if(param != NULL){ free(param); param = NULL; }
#define NEED_FREE(p)
#define NEED_FREE_N(ps, n)
#define MOCK_FREE(p)
#define SCOPE_START DTLLOG("------------------------------------%s Start------------------------------------", __FUNCTION__)
#define SCOPE_END   DTLLOG("====================================%s End=====================================", __FUNCTION__)
#else
#define NXL_ALLOC(ptr, t) (ptr = (t*)monitor_track(malloc(sizeof(t)), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define NXL_ALLOCN(ptr, t, n) (ptr = (t*)monitor_track(malloc(sizeof(t)*(n)), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define NXL_ALLOCS(ptr, t, size) (ptr = (t)monitor_track(malloc(size), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define NXL_REALLOC(ptr, t, n) (ptr = (t*)monitor_retrack(ptr, realloc(ptr, sizeof(t)*(n)), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define NXL_FREE(param) if(param != NULL){ free(param); monitor_free(param, #param, __FILE__, __FUNCTION__, __LINE__); param = NULL; }
#define NEED_FREE(p) monitor_track(p, #p, __FILE__, __FUNCTION__, __LINE__)
#define NEED_FREE_N(ps, n) monitor_track_array((void**)ps, n, #ps, __FILE__, __FUNCTION__, __LINE__)
#define MOCK_FREE(p) monitor_free(p, #p, __FILE__, __FUNCTION__, __LINE__)

#define SCOPE_START DTLLOG("------------------------------------%s Start(%lu)------------------------------------", __FUNCTION__, monitor_allocated());monitor_list()
#define SCOPE_END   monitor_list();DTLLOG("====================================%s End(%lu)=====================================", __FUNCTION__, monitor_allocated())
#endif

#endif