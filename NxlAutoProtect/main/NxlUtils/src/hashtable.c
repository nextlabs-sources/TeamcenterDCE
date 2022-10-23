#include <internal_hashtable.h>
#include "utils_mem.h"
#include "utils_string.h"
#include "utils_log.h"

static int getPrime(int min)
{
	static int primes[] = {3, 7, 11, 17, 23, 29, 37, 47, 59, 71, 89, 107, 131, 163, 197, 239, 293, 353, 431, 521, 631, 761, 919,
            1103, 1327, 1597, 1931, 2333, 2801, 3371, 4049, 4861, 5839, 7013, 8419, 10103, 12143, 14591,
            17519, 21023, 25229, 30293, 36353, 43627, 52361, 62851, 75431, 90523, 108631, 130363, 156437,
            187751, 225307, 270371, 324449, 389357, 467237, 560689, 672827, 807403, 968897, 1162687, 1395263,
            1674319, 2009191, 2411033, 2893249, 3471899, 4166287, 4999559, 5999471, 7199369};
	static int nprimes = sizeof(primes)/sizeof(int);
	int i = 0;
	while(i<nprimes)//checking i-th primes
	{
		if(primes[i]>min) return primes[i];
		i++;
	}
	return primes[nprimes-1];
}

/*
 Internal & Private members
*/
static hashcode_t getHashCode_chars(const char *key)
{
    hashcode_t hash = 5381;
    while (*key!='\0')
	{
        hash = ((hash << 5) + hash) + *key; /* hash * 33 + *key */
		key++;
	}

    return hash;
}
static BOOL private_resize(hashtable_p ht, int newSize)
{
	int newBucketSize = getPrime(newSize);
	int *newBuckets;
	_entry_list newEntries;
	int i;
	_hashtable_p repo = REPO(ht);
	DTLLOG("Resizing from %d to %d(Count:%d)", repo->capacity, newSize, ht->count);
	NXL_ALLOCN(newBuckets, int, newBucketSize);
	NXL_ALLOCN(newEntries, _entry_t, newSize);
	for(i=0; i<newBucketSize; i++)
	{
		newBuckets[i] = -1;
	}
	for(i=0; i<ht->count; i++)
	{
		if(repo->entries[i].key!=NULL)
		{
			int ibucket = repo->entries[i].hashcode%newBucketSize;
			newEntries[i] = repo->entries[i];
			newEntries[i].inext = newBuckets[ibucket];
			newBuckets[ibucket] = i;
		}
	}
	//free
	NXL_FREE(repo->buckets);
	repo->buckets = newBuckets;
	repo->bucketSize = newBucketSize;
	MOCK_FREE(newBuckets);
	//
	NXL_FREE(repo->entries);
	repo->entries = newEntries;
	repo->capacity = newSize;
	MOCK_FREE(newEntries);
	//
	NXL_REALLOC(ht->kvps, struct kvp_s, newSize);
	MOCK_FREE(ht->kvps);
	DTLLOG("DONE!");
	return TRUE;
}
//set without check value type
BOOL internal_set(hashtable_p ht, const char *key, void *val, ondup_e ondup, void **oldValue)
{
	_hashtable_p repo = REPO(ht);
	hashcode_t hash = getHashCode_chars(key);
	int ibucket = hash%repo->bucketSize;
	int ientry = repo->buckets[ibucket];
	*oldValue = NULL;
	while(ientry>=0)
	{
		_entry_p entry = &(repo->entries[ientry]);
		if(entry->hashcode==hash
			&& strcmp((char *)entry->key, key)==0)
		{
			//found existing one
			switch(ondup)
			{
				case ondup_override:
					DTLLOG("Overriding %s with %s for key-%s", entry->value, val, key);
					*oldValue = entry->value;
					entry->value = val;
					ht->kvps[ientry].value = val;
					return TRUE;
				case ondup_skip:
					//TODO:for other ondups
				default:
					return FALSE;
			}
		}
		ientry = entry->inext;
	}
	//adding a new entry
	if(ht->count==repo->capacity)
	{
		if((!private_resize(ht, repo->capacity*2))) return FALSE;
		//bucket size is changed, update ibucket accordingly
		ibucket = hash%repo->bucketSize;
	}
	//
	repo->entries[ht->count].hashcode = hash;
	repo->entries[ht->count].key = string_dup(key);
	repo->entries[ht->count].value = val;
	repo->entries[ht->count].inext = repo->buckets[ibucket];
	repo->buckets[ibucket] = ht->count;
	ht->kvps[ht->count].key = repo->entries[ht->count].key;
	ht->kvps[ht->count].value = val;
	MOCK_FREE(repo->entries[ht->count].key);
	ht->count++;
	return TRUE;
}
_entry_p internal_search(hashtable_ro ht, const char *key)
{
	_hashtable_p repo = REPO(ht);
	hashcode_t hash = getHashCode_chars(key);
	int ibucket = hash%repo->bucketSize;
	int ientry = repo->buckets[ibucket];
	_entry_p entry = NULL;
	while(ientry>=0)
	{
		entry = &(repo->entries[ientry]);
		if(entry->hashcode == hash
			&& strcmp(entry->key, key)==0)
		{
			return entry;
		}
		ientry = entry->inext;
	}
	return NULL;
}

BOOL internal_validateType(hashtable_p ht, vtype_e valueType)
{
	if(valueType==type_undefined) return FALSE;
	else if(REPO(ht)->vtype == type_undefined)
	{
		REPO(ht)->vtype = valueType;
	}
	else if(REPO(ht)->vtype != valueType)
	{
		DBGLOG("This hashtable(%p)'s value type(%d) is not matched with %d", ht, REPO(ht)->vtype, valueType);
		return FALSE;
	}
	return TRUE;
}

/*
Implementation of public members
*/
void ht_free(hashtable_p ht)
{
	if(ht!=NULL)
	{
		int i;
		_hashtable_p repo = REPO(ht);
		for(i=0; i<ht->count; i++)
		{
			//because the value of the key and value is always created internally
			//it's
			NXL_FREE(repo->entries[i].key);
			if(repo->vtype == type_string)
			{
				char *val = (char *)repo->entries[i].value;
				NXL_FREE(val);
			}
			//TODO:for other types
			repo->entries[i].value = NULL;
		}
		NXL_FREE(repo->entries);
		NXL_FREE(repo->buckets);
		NXL_FREE(repo);
		NXL_FREE(ht->kvps);
		NXL_FREE(ht);
	}
	DTLLOG("DONE!");
}
hashtable_p ht_create(int capacity)
{
	hashtable_p ht;
	_hashtable_p repo;
	//initialize repo
	NXL_ALLOC(repo, struct _hashtable_s);
	repo->entries = NULL;
	repo->capacity = 0;
	repo->buckets = NULL;
	repo->bucketSize = 0;
	repo->vtype = type_undefined;
	repo->ivacant = -1;
	MOCK_FREE(repo);

	//initialize hashtable
	NXL_ALLOC(ht, struct hashtable_s);
	ht->kvps = NULL;
	ht->count = 0;
	ht->repo = repo;
	//initialize private repository object
	capacity = capacity > 0 ? capacity : HT_DEFAULT_SIZE;
	//resize as capacity
	private_resize(ht, capacity);
	DTLLOG("DONE!");
	return ht;
}
BOOL ht_contains_key(hashtable_ro ht, const char *key)
{
	return ht != NULL
		&& key != NULL
		&& internal_search(ht, key) != NULL;
}
