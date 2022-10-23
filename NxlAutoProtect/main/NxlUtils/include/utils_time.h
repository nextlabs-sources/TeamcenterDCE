#ifndef NXL_UTILS_TIME_H_INCLUDED
#define NXL_UTILS_TIME_H_INCLUDED
#include <time.h>
#include "nxl_utils_exports.h"
#include "utils_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

//Time related utils
NXLAPI time_t time_convert(struct tm tm, float timezone);
NXLAPI const char *time_tostring(time_t time);

#ifdef __cplusplus
}
#endif

#endif