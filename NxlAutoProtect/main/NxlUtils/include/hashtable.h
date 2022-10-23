/*
	This header file defines some hashtable related APIs
	The current version only support add/set/contains/tryget operation
	The remove operation hasnot been implemented yet because it's not needed in current solution
*/
#ifndef NXL_UTILS_HASHTABLE_H_INCLUDED
#define NXL_UTILS_HASHTABLE_H_INCLUDED
#include "nxl_utils_exports.h"
#include "utils_common.h"
typedef struct kvp_s
{
	const char *key;
	const void *value;
}*kvp_p, *kvp_array;

typedef struct hashtable_s
{
	int count;	//count of entries, do not set
	kvp_array kvps;	//KeyValuePairs, do not tempt!
	void *repo;	//internal use only; do not tempt!
}*hashtable_p;
typedef const struct hashtable_s *hashtable_ro;

typedef enum _ondup_e
{
	ondup_skip,
	ondup_override
} ondup_e;

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI hashtable_p ht_create(int capacity);
//free hashtable struct and the key value of entries
NXLAPI void ht_free(hashtable_p ht);
NXLAPI BOOL ht_contains_key(hashtable_ro ht, const char *key);
//TODO:
//NXLAPI BOOL ht_remove_key(hashtable_ro ht, const char *key);

//when values are chars
NXLAPI const char *ht_get_chars(hashtable_ro ht, const char *key);
NXLAPI const char *ht_set_chars(hashtable_p ht, const char *key, const char *value);
NXLAPI BOOL ht_tryget_chars(hashtable_ro ht, const char *key, const char **val);
NXLAPI int ht_merge_chars(hashtable_p tar, hashtable_ro src, ondup_e ondup);

#ifdef __cplusplus
}
#endif

#endif
