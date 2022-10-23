#ifndef NXL_DCE_MEM_H_INCLUDED
#define NXL_DCE_MEM_H_INCLUDED
#include <base_utils/Mem.h>	//MEM_alloc(); MEM_realloc(); MEM_free();
#include "utils_mem.h"

//Use Teamcenter Mem module instead of NxlUtils for memory allocation
#ifndef DEBUG_MEM
#define TC_ALLOC(ptr, t) (ptr = (t*)MEM_alloc((int)sizeof(t)))
#define TC_ALLOCN(ptr, t, n) (ptr = (t*)MEM_alloc((int)(sizeof(t)*(n))))
#define TC_REALLOC(ptr, t, n) (ptr = (t*)MEM_realloc(ptr, (int)(sizeof(t)*(n))))
#define TC_FREE(param) if(param != NULL){ CALL_DTL(MEM_free(param)); param = NULL; }
#define TC_FREE2(param) if(param != NULL){ CALL_DTL(MEM_free(param));}
//#define NEED_FREE(p)
//#define NEED_FREE_N(ps, n)
//#define MOCK_FREE(p)
//#define RESET_MONITOR
#define TC_DUP(x) MEM_string_copy(x)
#define TC_NCOPY(x, n) MEM_string_ncopy(x, n)
#else
#define TC_ALLOC(ptr, t) (ptr = (t*)monitor_track(MEM_alloc((int)sizeof(t)), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define TC_ALLOCN(ptr, t, n) (ptr = (t*)monitor_track(MEM_alloc((int)(sizeof(t)*(n))), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define TC_REALLOC(ptr, t, n) (ptr = (t*)monitor_retrack(ptr, MEM_realloc(ptr, (int)(sizeof(t)*(n))), #ptr, __FILE__, __FUNCTION__, __LINE__))
#define TC_FREE(param) if(param != NULL){ CALL_DTL(MEM_free(param)); monitor_free(param, #param, __FILE__, __FUNCTION__, __LINE__); param = NULL; }
#define TC_FREE2(param) if(param != NULL){ CALL_DTL(MEM_free(param)); monitor_free(param, #param, __FILE__, __FUNCTION__, __LINE__);}
//#define NEED_FREE(p) monitor_track(p, #p, __FILE__, __FUNCTION__, __LINE__)
//#define NEED_FREE_N(ps, n) monitor_track_array((void**)ps, n, #ps, __FILE__, __FUNCTION__, __LINE__)
//#define MOCK_FREE(p) monitor_free(p, #p, __FILE__, __FUNCTION__, __LINE__)
//#define RESET_MONITOR monitor_reset()
#define TC_DUP(x) ((char*)monitor_track(MEM_string_copy(x), #x, __FILE__, __FUNCTION__, __LINE__))
#define TC_NCOPY(x, n) ((char*)monitor_track(MEM_string_ncopy(x, n), #x, __FILE__, __FUNCTION__, __LINE__))
#endif

#endif //NXL_DCE_MEM_H_INCLUDED
