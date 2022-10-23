#ifndef NXL_UTILS_EVALUATOR_DEFS_H_INCLUDED
#define NXL_UTILS_EVALUATOR_DEFS_H_INCLUDED
#include "nxl_utils_exports.h"
#include "key_value_list.h"
#include "hashtable.h"

typedef int eval_result_t;
#define EVAL_ALLOW 1
#define EVAL_DENY -1
#define EVAL_DEFAULT 0

typedef struct eval_app_s
{
	const char *name;
	const char *url;
	kvl_ro attributes;
}const *eval_app_ro;

typedef struct eval_user_s
{
	const char *name;
	const char *id;
	kvl_ro attributes;
}const *eval_user_ro;

typedef struct eval_res_s
{
	const char *name;
	const char *type;
	kvl_ro attributes;
}const *eval_res_ro;

#define RES_TYPE_PORTAL "tc"
#define RES_KEY_URL "url"

typedef struct obligation_s
{
	char *name;
	char *policy;
	hashtable_p attributes;
}*obligation_p, *obligation_list;
typedef const struct obligation_s *obligation_ro, *obligation_list_ro;

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI eval_user_ro default_eval_user();
NXLAPI void obligations_free(obligation_list obs, int nob);

#ifdef __cplusplus
}
#endif

#endif
