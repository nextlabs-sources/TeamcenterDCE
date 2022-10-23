#ifndef NXL_UTILS_KEY_VALUE_PAIR_H_INCLUDED
#define NXL_UTILS_KEY_VALUE_PAIR_H_INCLUDED
#include "nxl_utils_exports.h"
#include "utils_common.h"

/*	Note:
	1, Use kvl_* to manipulate this structure, don't tempt the member directly
	2, Create a copy of items/items[i] when you need it for external use
*/
struct _kvl_private_s;
typedef struct _key_value_list_s
{
	unsigned int count;
	char * const *keys;
	char * const *vals;
	struct _kvl_private_s *repo;
}*kvl_p;
typedef const struct _key_value_list_s *kvl_ro;

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI kvl_p kvl_new(unsigned int size);
NXLAPI void kvl_free(kvl_p kvl);
NXLAPI int kvl_add(kvl_p kvl, const char *key, const char *val);
//update the value when the key exists, or add new key&value
NXLAPI int kvl_setOrAdd(kvl_p kvl, const char *key, const char *val);
NXLAPI BOOL kvl_set(kvl_p kvl, int index, const char *newKey, const char *newVal);
NXLAPI BOOL kvl_removeAt(kvl_p kvl, int index);
NXLAPI kvl_p kvl_distinct(kvl_ro kvl);
NXLAPI void kvl_clear(kvl_p kvl);
NXLAPI const char *kvl_indexer(kvl_ro list, const char *key);

#ifdef __cplusplus
}
#endif

#endif