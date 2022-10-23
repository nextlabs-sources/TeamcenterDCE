#ifndef NXL_UTILS_EVALUATOR_LOCAL_H_INCLUDED
#define NXL_UTILS_EVALUATOR_LOCAL_H_INCLUDED
#include "nxl_evaluator_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI eval_result_t local_evaluate( const char *action,
	eval_app_ro app, eval_user_ro user, eval_res_ro src, 
	obligation_list *obligations, int *nob);

#ifdef __cplusplus
}
#endif

#endif
