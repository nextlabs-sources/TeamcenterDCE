#include "key_value_list.h"
#include <internal.h>
#include "utils_mem.h"
#include <utils_string_list.h>

typedef struct _kvl_private_s
{
	string_list_p keys;
	string_list_p vals;
}*kvl_private_t;

kvl_p kvl_new(unsigned int size)
{
	kvl_p list = NULL;
	NXL_ALLOC(list, struct _key_value_list_s);
	NXL_ALLOC(list->repo, struct _kvl_private_s);
	list->repo->keys = string_list_new();
	list->repo->vals = string_list_new();
	if(size > 0)
	{
		string_list_ensure_internal(list->repo->keys, size);
		string_list_ensure_internal(list->repo->vals, size);
	}
	list->keys = list->repo->keys->items;
	list->vals = list->repo->vals->items;
	list->count = 0;

	return list;
}
NXLAPI void kvl_free(kvl_p kvl)
{
	if(kvl != NULL)
	{
		if(kvl->repo!=NULL)
		{
			string_list_free(kvl->repo->keys);
			string_list_free(kvl->repo->vals);
		}
		NXL_FREE(kvl->repo);
	}
	NXL_FREE(kvl);
}
void kvl_clear(kvl_p kvl)
{
	if(kvl != NULL
		&& kvl->repo != NULL)
	{
		string_list_clear(kvl->repo->keys);
		string_list_clear(kvl->repo->vals);
		kvl->keys = NULL;
		kvl->vals = NULL;
		kvl->count = 0;
	}
}
NXLAPI int kvl_add(kvl_p kvl, const char *key, const char *val)
{
	if(kvl != NULL
		&& kvl->repo != NULL)
	{
		int index = string_list_add(kvl->repo->keys, key);
		kvl->keys = kvl->repo->keys->items;
		string_list_add(kvl->repo->vals, val);
		kvl->vals = kvl->repo->vals->items;
		//upate count
		kvl->count = kvl->repo->keys->count;
		return index;
	}
	return -1;
}
int kvl_set_internal(kvl_p kvl, char *key, char *val)
{
	if(kvl != NULL
		&& kvl->repo != NULL)
	{
		int index = string_list_add_internal(kvl->repo->keys, key);
		kvl->keys = kvl->repo->keys->items;
		string_list_add_internal(kvl->repo->vals, val);
		kvl->vals = kvl->repo->vals->items;
		//upate count
		kvl->count = kvl->repo->keys->count;
		return index;
	}
	return -1;
}

const char *kvl_indexer(kvl_ro list, const char *key)
{
	if(list != NULL
		&& list->repo != NULL)
	{
		int index = string_list_index_of(list->repo->keys, key);
		if(index >= 0 )
		{
			return list->vals[index];
		}
	}
	return NULL;
}
int kvl_setOrAdd(kvl_p list, const char *key, const char *val)
{
	if(list != NULL
		&& list->repo != NULL)
	{
		int index = string_list_index_of(list->repo->keys, key);
		if(index >= 0 )
		{
			if(string_list_set(list->repo->vals, val, index))
			{
				return index;
			}
		}
		else
		{
			//add
			return kvl_add(list, key, val);
		}
	}
	return -1;
}
BOOL kvl_set(kvl_p list, int index, const char *newKey, const char *newVal)
{
	if(list != NULL
		&& list->repo != NULL
		&& index >= 0
		&& index < list->count)
	{
		return string_list_set(list->repo->keys, newKey, index)
			&& string_list_set(list->repo->vals, newVal, index);
	}
	return FALSE;
}
BOOL kvl_removeAt(kvl_p list, int index)
{
	if(list != NULL
		&& list->repo != NULL
		&& index >= 0
		&& index < list->count)
	{
		if(string_list_removeAt(list->repo->keys, index)
			&& string_list_removeAt(list->repo->vals, index))
		{
			list->count = list->repo->vals->count;
			return TRUE;
		}

	}
	return FALSE;
}
kvl_p kvl_distinct(kvl_ro list)
{
	if(list != NULL
		&& list->repo != NULL)
	{
		kvl_p distinct = kvl_new(list->count);
		int i = 0;
		while(i < list->count)
		{
			//adding list[i]
			if(list->keys[i] != NULL 
					&& list->vals[i] != NULL)
			{
				int j;
				//check duplication
				for(j=0; j<distinct->count; j++)
				{
					if(strcmp(list->keys[i], distinct->keys[j]) == 0
							&& strcmp(list->vals[i], distinct->vals[j]) == 0)
					{
						//duplicted, go next
						goto ADD_NEXT;
					}
				}
				//no duplicated found
				kvl_add(distinct, list->keys[i], list->vals[i]);
			}
ADD_NEXT:
			i++;
		}
		return distinct;
	}
	return NULL;
}