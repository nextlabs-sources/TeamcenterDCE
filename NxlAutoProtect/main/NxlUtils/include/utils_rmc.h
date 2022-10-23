#ifndef NXL_UTILS_RMC_H_INCLUDED
#define NXL_UTILS_RMC_H_INCLUDED
#include "nxl_utils_exports.h"
#include "utils_common.h"
#include "key_value_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

//return TRUE when the .nxl file exists
NXLAPI BOOL file_is_nxl(const char *filePath);
//return TRUE when the file content is protected
//use in process reading, therefore the current process must NOT has view right
NXLAPI BOOL file_is_protected(const char *filePath);
//NXLAPI char *RMC_get_orig_ext(const char *filePath);

/*RMC related utilities*/
NXLAPI BOOL RMC_is_installed();
NXLAPI BOOL RMC_is_running();
//return the count of the tags, return -1 if failed
NXLAPI int RMC_get_tags(const char *filePath, kvl_p returnList);//<returnList> must be allocated before this call
//return TRUE when protect success
NXLAPI BOOL RMC_protect(const char *filePath, kvl_ro list);

/*
CHECK NXL FILE via nxlhelper.exe
nxlhelper.exe is required
*/
//return TRUE when the file content is protected
NXLAPI BOOL helper_is_protected(const char *filePath);
//return TRUE when the file content is protected, the tags are filled in the returnList
NXLAPI BOOL helper_get_tags(const char *filePath, kvl_p returnList);//<returnList> must be allocated before this call

#ifdef __cplusplus
}
#endif

#endif