#include "test.h"
#include "utils_common.h"
#include <key_value_list.h>

result_t test_kvl_add(kvl_p list)
{
	result_t ret;
	char *keys[] = {NULL, NULL, NULL, "KEY", "KEY", "KEY", "", "", ""};
	char *vals[] = {NULL, ""  , "val", NULL, "",	"VAL", NULL, "", "VAL"};
	char *indexers[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	int oldCount = list->count;
	int icase;
	int ncase = sizeof(keys)/sizeof(char*);
	INIT(ret);
	for(icase=0; icase < ncase; icase++)
	{
		const char *indexer;
		int index = kvl_add(list, keys[icase], vals[icase]);
		ASSERT(index, oldCount + icase, ret, icase);
		ASSERT(list->count, oldCount + icase + 1, ret, icase);
		ASSERT_STR(list->keys[index], keys[icase], ret, icase);
		ASSERT_STR(list->vals[index], vals[icase], ret, icase);
		indexer = kvl_indexer(list, keys[icase]);
		ASSERT_STR(indexer, indexers[icase], ret, icase);
	}
	return ret;
}
result_t test_kvl_set(kvl_p list)
{
	result_t ret;
	char *keys[] = {"key1", "key2", "key3", "KEY4", "KEY5", "KEY6"};
	char *vals[] = {"val11", "val22"  , "val33", "val4", "val5","val6"};
	int indexes[] = {0, 1, 2, 3, 4, 5};
	int icase;
	int ncase = sizeof(keys)/sizeof(char*);
	INIT(ret);
	for(icase=0; icase < ncase; icase++)
	{
		int index = kvl_setOrAdd(list, keys[icase], vals[icase]);
		const char* indexer = kvl_indexer(list, keys[icase]);
		ASSERT(index, indexes[icase], ret, icase);
		ASSERT_STR(indexer, vals[icase], ret, icase);
	}
	return ret;
}
result_t test_kvl()
{
	result_t ret;
	kvl_p list = NULL;
	INIT(ret);
	list = kvl_new(0);	PASS(ret);
	SUBRUN(test_kvl_add(list), ret);
	kvl_clear(list);	
	ASSERT(list->count, 0, ret, __LINE__);
	ASSERT(list->keys, NULL, ret, __LINE__);
	ASSERT(list->vals, NULL, ret, __LINE__);
	kvl_free(list);		PASS(ret);

	list = kvl_new(10);
	kvl_add(list, "key1", "val1");
	kvl_add(list, "key2", "val2");
	kvl_add(list, "key3", "val3");
	SUBRUN(test_kvl_set(list), ret);
	kvl_free(list);		PASS(ret);
	return ret;
}