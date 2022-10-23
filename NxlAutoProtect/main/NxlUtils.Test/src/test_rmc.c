#ifdef WNT
#include "test.h"
#include "utils_rmc.h"
#include <utils_mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils_string.h>

char *create_file(const char *fileName)
{
	FILE *file = NULL;
	char *fullName = string_dup(gent_file_path(fileName));
	if(fullName == NULL)
	{
		return NULL;
	}
	file = fopen(fullName, "w");
	if(file)
	{
		fputs("This file is created for UnitTests\n", file);
		fputs("OriginalFileName is ", file);
		fputs(fullName, file);
		fclose(file);
		return fullName;
	}
	else
	{
		OUTPUT("Failed to create: %s", fullName);
		return NULL;
	}
}

result_t test_rmc()
{
	result_t ret;
	char *testFile;
	INIT(ret);
	ASSERT(RMC_is_installed(), TRUE, ret, __LINE__);
	ASSERT(RMC_is_running(), TRUE, ret, __LINE__);
	//prepare initial file
	testFile = create_file("testfile.txt");
	ASSERT(testFile!=NULL, TRUE, ret, __LINE__);
	if(testFile!=NULL)
	{
		char *keys[] = {"key1", "key2 contains space", "key3", "key4 has space"};
		char *vals[] = {"val1", "val2 contains space", "val3 has space", "val4"};
		int ntags = sizeof(keys)/sizeof(char*);
		int retProtect, i;
		kvl_p inputTags = kvl_new(ntags);
		kvl_p outputTags = kvl_new(ntags);

		for(i=0; i<ntags; i++)
		{
			kvl_add(inputTags, keys[i], vals[i]);
		}
		//CASE: check on non-protected file
		ASSERT(file_is_protected(testFile), FALSE, ret, __LINE__);
		ASSERT(helper_is_protected(testFile), FALSE, ret, __LINE__);

		//CASE: Protect file
		retProtect = RMC_protect(testFile,  inputTags);
		ASSERT(retProtect, TRUE, ret, __LINE__);
		//CASE: check on protected file
		ASSERT(file_is_protected(testFile), TRUE, ret, __LINE__);
		ASSERT(helper_is_protected(testFile), TRUE, ret, __LINE__);

		kvl_clear(outputTags);
		retProtect = RMC_get_tags(testFile, outputTags);
		ASSERT(retProtect, ntags, ret, __LINE__);
		for(i=0; i < retProtect&&i < ntags; i++)
		{
			ASSERT_STR(outputTags->keys[i], keys[i], ret, i);
			ASSERT_STR(outputTags->vals[i], vals[i], ret, i);
		}
		
		kvl_clear(outputTags);
		retProtect = helper_get_tags(testFile, outputTags);
		ASSERT(retProtect, TRUE, ret, __LINE__);
		ASSERT(outputTags->count, ntags, ret, __LINE__);
		for(i=0; i < retProtect&&i < ntags; i++)
		{
			ASSERT_STR(outputTags->keys[i], keys[i], ret, i);
			ASSERT_STR(outputTags->vals[i], vals[i], ret, i);
		}
		kvl_free(inputTags);
		kvl_free(outputTags);
	}
	NXL_FREE(testFile);
	return ret;
}
#endif