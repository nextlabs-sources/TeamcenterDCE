#ifndef NXL_UITLS_NXL_H_INCLUDED
#define NXL_UITLS_NXL_H_INCLUDED
#include "nxl_utils_exports.h"

#define ENV_DCE_ROOT "RMX_ROOT"

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI const char *get_utils_dir();
NXLAPI char *search_dce_lib(const char *libName);
NXLAPI int IP_local_number();

#ifdef __cplusplus
}
#endif

#endif