#include <internal_hashtable.h>
#include "utils_log.h"
#include <stdlib.h>
#include "utils_mem.h"
#include "utils_string.h"

/*
When values are char*
*/

const char *ht_get_chars(hashtable_ro ht, const char *key)
{
	const char *val;
	if(ht_tryget_chars(ht, key, &val))
	{
		return val;
	}
	return NULL;
}
BOOL ht_tryget_chars(hashtable_ro ht, const char *key, const char **val)
{
	if(ht != NULL
		&& key != NULL)
	{
		if(REPO(ht)->vtype==type_string)
		{
			_entry_p entry = internal_search(ht, key);
			*val = NULL;
			if(entry!=NULL)
			{
				*val = (const char *)entry->value;
				return TRUE;
			}
		}
	}
	return FALSE;
}
const char *ht_set_chars(hashtable_p ht, const char *key, const char *val)
{
	if(ht != NULL
		&& key != NULL
		&& internal_validateType(ht, type_string))
	{
		char *dupval = string_dup(val);
		void *oldValue;
		if(internal_set(ht, key, dupval, ondup_override, &oldValue))
		{
			MOCK_FREE(dupval);
			if(oldValue!=NULL)
				NXL_FREE(oldValue);
			return dupval;
		}
		else
		{
			NXL_FREE(dupval);
		}
	}
	return NULL;
}
int ht_merge_chars(hashtable_p tar, hashtable_ro src, ondup_e ondup)
{
	int ret = 0;
	if(tar!=NULL && src!=NULL && src->count>0)
	{
		_hashtable_p srcRepo = REPO(src);
		if(srcRepo->vtype==type_string)
		{
			if(internal_validateType(tar, type_string))
			{
				int ientry;
				int skipped = 0;
				for(ientry=0; ientry<src->count; ientry++)
				{
					char *dupval = string_dup((char*)(srcRepo->entries[ientry].value));
					void *oldValue;
					if(internal_set(tar, srcRepo->entries[ientry].key, dupval, ondup, &oldValue))
					{
						MOCK_FREE(dupval);
						if(oldValue!=NULL)
						{
							NXL_FREE(oldValue);
						}
						ret++;
					}
					else
					{
						NXL_FREE(dupval);
						skipped++;
					}
				}
				DBGLOG("Added %d Skipped %d Total %d", ret, skipped, src->count);
			}
		}
		else
		{
			ERROR_VTYPE(src, type_string);
		}
	}
	DTLLOG("Done(%d)", ret);
	return ret;
}
