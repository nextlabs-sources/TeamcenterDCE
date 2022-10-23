#ifndef NXL_UTILS_HASHTABLE_INTERNAL_H_INCLUDED
#define NXL_UTILS_HASHTABLE_INTERNAL_H_INCLUDED
#include <hashtable.h>

typedef unsigned long hashcode_t;
typedef struct _entry_s
{
	hashcode_t hashcode;	//internal use only
	char *key;
	void *value;
	int inext;	//internal use only - index to next entry in the same bucket
}_entry_t, *_entry_p, *_entry_list;

typedef enum _value_type
{
	type_undefined = 0
	,type_string
	//,type_pointer
	//,type_num
}vtype_e;

typedef struct _hashtable_s
{
	_entry_list entries;	//key value entries
	int capacity;	//size of entries
	int *buckets;	//buckets
	int bucketSize;	//size of bukets
	int ivacant;	//internal use only:TODO-remove key
	vtype_e vtype;
}*_hashtable_p;

#define REPO(ht) ((_hashtable_p)ht->repo)

#define HT_DEFAULT_SIZE 10

#define ERROR_VTYPE(ht, t) DBGLOG("This hashtable(%p)'s value type(%d) doesn't match with '"#t"'(%d)", ht, REPO(ht)->vtype, t)

BOOL internal_set(hashtable_p ht, const char *key, void *val, ondup_e ondup, void **oldValue);
_entry_p internal_search(hashtable_ro ht, const char *key);
//can set
BOOL internal_validateType(hashtable_p ht, vtype_e vtype);

#endif 