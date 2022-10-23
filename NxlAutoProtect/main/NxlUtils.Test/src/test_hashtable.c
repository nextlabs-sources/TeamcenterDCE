#include "test.h"
#include <internal_hashtable.h>

result_t test_merge()
{
	char *keys1[] = {"A", "B", "C", "D"};
	char *vals1[] = {"A1", "B1", "C1", "D1"};
	int n1 = sizeof(keys1)/sizeof(char*);
	char *keys2[] = {"C", "D", "E", "F"};
	char *vals2[] = {"C1", "D2", "E2", "F2"};
	int n2 = sizeof(keys2)/sizeof(char*);
	char *keys3[] = {"A","B","C","D", "E","F", "G"};
	char *valsskip[] = {"A1","B1", "C1", "D1", "E2", "F2", NULL};
	char *valsover[] = {"A1","B1", "C1", "D2", "E2", "F2", NULL};
	int n3 = sizeof(keys3)/sizeof(char*);
	int i;
	unsigned long allocated = MEM_ALLOCATED;
	hashtable_p ht1 = NULL;
	hashtable_p ht2 = NULL;
	result_t ret;
	ondup_e ondup = ondup_skip;
	char **expects = NULL;
	INIT(ret);
NEXT_CASE:
	ht1 = ht_create(1);
	ht2 = ht_create(1);
	for(i=0; i<n1; i++)
	{
		ht_set_chars(ht1, keys1[i], vals1[i]);
	}
	for(i=0; i<n2; i++)
	{
		ht_set_chars(ht2, keys2[i], vals2[i]);
	}
	ht_merge_chars(ht1, ht2, ondup);
	expects = ondup==ondup_skip?valsskip:valsover;
	for(i=0; i<n3; i++)
	{
		const char *val = ht_get_chars(ht1, keys3[i]);
		ASSERT_STR(val, expects[i], ret, i);		
	}
	ht_free(ht1);
	ht_free(ht2);
	ASSERT(MEM_ALLOCATED, allocated, ret, __LINE__);
	if(ondup==ondup_skip)
	{
		ondup=ondup_override;
		goto NEXT_CASE;
	}
	return ret;
}

result_t test_create()
{
	int cases[] = {0, 5, HT_DEFAULT_SIZE, 100};
	int ncases = sizeof(cases)/sizeof(int);
	int expects[] = {HT_DEFAULT_SIZE, 5, HT_DEFAULT_SIZE, 100};
	int nexp = sizeof(expects)/sizeof(int);
	int i;
	unsigned long allocated;
	hashtable_p ht;
	result_t ret;
	INIT(ret);
	for(i=0; i<ncases&&i<nexp; i++)
	{
		allocated = MEM_ALLOCATED;
		ht = ht_create(cases[i]);
		ASSERT(REPO(ht)->capacity, expects[i], ret, i);

		ht_free(ht);
		ASSERT(MEM_ALLOCATED, allocated, ret, i);
	}
	return ret;
}
result_t test_getset()
{
	char *setkeys[] = {NULL, NULL, "A", "A",  "A",  "B", "B"};
	char *setvals[] = {NULL, "ABC", NULL,"B", "BBB","BBB","BBB"};
	char *getkeys[] = {NULL, NULL, "A", "A",  "A",  "B", "C"};
	char *expvals[] = {NULL, NULL, NULL, "B", "BBB","BBB",NULL};
	BOOL expcons[] =  {FALSE,FALSE,TRUE, TRUE, TRUE, TRUE, FALSE };
	int ncases = sizeof(setkeys)/sizeof(char*);
	unsigned long allocated = MEM_ALLOCATED;
	hashtable_p ht = ht_create(100);
	const char *tmp;
	BOOL bl;
	result_t ret;
	int i;
	INIT(ret);

	for(i=0; i<ncases; i++)
	{
		ht_set_chars(ht, setkeys[i], setvals[i]);
		PASS(ret);

		tmp = ht_get_chars(ht, getkeys[i]);
		ASSERT_STR(tmp, expvals[i], ret, i);

		bl = ht_contains_key(ht, getkeys[i]);
		ASSERT(bl, expcons[i], ret, i);
		
		bl = ht_tryget_chars(ht, getkeys[i], &tmp);
		ASSERT(bl, expcons[i], ret, i);
		ASSERT_STR(tmp, expvals[i], ret, i);
	}
	ht_free(ht);
	ASSERT(MEM_ALLOCATED, allocated, ret, i);
	return ret;
}


result_t test_hashtable()
{
	result_t ret;
	INIT(ret);
	SUBRUN(test_create(), ret);
	SUBRUN(test_getset(), ret);
	SUBRUN(test_merge(), ret);
	return ret;
}