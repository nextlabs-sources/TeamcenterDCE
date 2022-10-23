#include "test.h"
#include "utils_common.h"
#include "utils_string.h"
#include <utils_string_list.h>

result_t test_string_isTrue()
{
	result_t ret;
	char *cases[] = {NULL, "true", "TRUE",	"T",	"t",	" TRUE ",	" T ",	"FALSE"};
	BOOL expects[] = {FALSE, TRUE, TRUE,	TRUE,	TRUE,	TRUE,		TRUE,	FALSE};
	int i;
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);
	if(n!=(sizeof(expects)/sizeof(BOOL)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		for(i=0; i<n; i++)
		{
			BOOL actual = string_isTrue(cases[i]);
			ASSERT(actual, expects[i], ret, i);
		}
	}
	return ret;
}

result_t test_string_split()
{
	result_t ret;
	char *cases[] = {NULL, "\rline1", "\rline1\nline2",	"line1\r\nline2",	"line1\r\nline2\r\n",	"line1\r\n\r\nline2\r\n\r\n", "\r\n\r\n\r\r\r\r"};
	char *seperators = "\r\n";
	int expectNums[] = {0, 1, 2,	2,	2,	2,		0};
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);
	if(n!=(sizeof(expectNums)/sizeof(int)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		int i;
		for(i=0; i<n; i++)
		{
			string_list_p lines = string_list_new();
			int num = string_split(cases[i], seperators, lines);
			ASSERT(num, lines->count, ret, i);
			ASSERT(num, expectNums[i], ret, i);
			string_list_free(lines);
		}
	}
	return ret;
}

result_t test_string_toLower()
{
	result_t ret;
	char *cases[] = {NULL, "ABC", "ABc",	"abc",	"ABC.~",	" abc ",	" a*b ",	"aXb"};
	char *expects[] = {NULL, "abc", "abc",	"abc",	"abc.~",	" abc ",	" a*b ",	"axb"};
	int i;
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);
	if(n!=(sizeof(expects)/sizeof(char*)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		for(i=0; i<n; i++)
		{
			char *actual = string_toLower(cases[i]);
			ASSERT_STR(actual, expects[i], ret, i);
			string_free(actual);
		}
	}
	return ret;
}
result_t test_string_isNullOrSpace()
{
	result_t ret;
	char *cases[] = {NULL, "", " ", "	", "A","BC", "AA"};
	BOOL expects[] = {TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE};
	int i;
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);
	if(n!=(sizeof(expects)/sizeof(BOOL)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		for(i=0; i<n; i++)
		{
			BOOL actual = string_isNullOrSpace(cases[i]);
			ASSERT(actual, expects[i], ret, i);
		}
	}
	return ret;
}
//strings related utilities
result_t test_strings_index_of()
{
	result_t ret;
	char *table[] = {NULL, "","A","B", "AB", "ABCDEF","BBB", " "};
	int ntable = sizeof(table)/sizeof(char*);
	char *cases[] = {NULL, "", " ", "A", "C", "AB", "BCD"};
	int expects[] = {0, 1, 7, 2, -1, 4, -1};
	int i;
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);

	if(n!=(sizeof(expects)/sizeof(int)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		for(i=0; i<n; i++)
		{
			int actual = strings_index_of(table, ntable, cases[i]);
			ASSERT(actual, expects[i], ret, i);
		}
	}
	return ret;

}

result_t test_string_starts_with()
{
	result_t ret;
	char *cases[] = {NULL, "ABC","ABC", "ABCEDF", "ABCED"};
	char *prefixes[] = {NULL, "A","B", "ABC", "ABDFG"};
	BOOL expects[] = {TRUE, TRUE, FALSE, TRUE, FALSE};
	int i;
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);
	if(n!=(sizeof(expects)/sizeof(BOOL)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		for(i=0; i<n; i++)
		{
			BOOL actual = string_starts_with(cases[i], prefixes[i]);
			ASSERT(actual, expects[i], ret, i);
		}
	}
	return ret;
}

result_t test_string_ends_with()
{
	result_t ret;
	char *cases[] =		{NULL,	NULL,	NULL,	NULL,	"ABC",	"ABC",	"ABC",	"ABC",	"ABC",	"ABC",	"ABC",	"ABC ",	"ABC ",	"ABC ",	"ABC ",	"ABC ",	"ABC ",	"ABC "};
	char *suffixes[] =	{NULL,	"A",	"",		"a",	NULL,	"A",	"",		"a",	"C",	"c",	" ",	NULL,	"A",	"",		"a",	"C",	"c",	" "};
	BOOL expects[] =	{TRUE, FALSE,	FALSE,	FALSE,	TRUE,	FALSE,	TRUE,	FALSE,	TRUE,	TRUE,	FALSE,	TRUE,	FALSE,	TRUE,	FALSE,	FALSE,	FALSE,	TRUE};
	BOOL expectsCS[] = {TRUE, FALSE,	FALSE,	FALSE,	TRUE,	FALSE,	TRUE,	FALSE,	TRUE,	FALSE,	FALSE,	TRUE,	FALSE,	TRUE,	FALSE,	FALSE,	FALSE,	TRUE};
	int i;
	int n = sizeof(cases)/sizeof(char*);
	INIT(ret);
	if(n!=(sizeof(expects)/sizeof(BOOL)))
	{
		printf("ERROR!!!%s():Count of expected results doesn't match to test cases!\n", __FUNCTION__);
	}
	else
	{
		for(i=0; i<n; i++)
		{
			BOOL actual = string_ends_with(cases[i], suffixes[i], FALSE);
			ASSERT(actual, expects[i], ret, i);
			actual = string_ends_with(cases[i], suffixes[i], TRUE);
			ASSERT(actual, expectsCS[i], ret, i);
		}
	}
	return ret;

}

result_t test_string_list_add(string_list_p list)
{
	result_t ret;
	char *newItems[] = {NULL, "TEST", "", "TEST", "TEST2"};
	int indexes[] = {0, 1, 2, 1, 4};
	int oldCount = list->count;
	int icase;
	int ncase = sizeof(newItems)/sizeof(char*);
	INIT(ret);
	for(icase=0; icase < ncase; icase++)
	{
		int index;
		index = string_list_add(list, newItems[icase]);
		ASSERT(index, icase, ret, icase);
		ASSERT(list->count, oldCount + icase + 1, ret, icase);
		ASSERT_STR(list->items[oldCount+icase], newItems[icase], ret, icase);
		index = string_list_index_of(list, newItems[icase]);
		ASSERT(index, indexes[icase], ret, icase);
	}
	return ret;
}
result_t test_string_list_set(string_list_p list)
{
	result_t ret;
	char *newItems[] = {NULL, "TEST", "", "TEST", "TEST2"};
	int oldCount = list->count;
	int icase;
	int ncase = sizeof(newItems)/sizeof(char*);
	INIT(ret);
	for(icase=0; icase < ncase; icase++)
	{
		string_list_set(list, newItems[icase], 1);
		ASSERT(list->count, oldCount, ret, icase);
		ASSERT_STR(list->items[1], newItems[icase], ret, icase);
	}
	return ret;
}
result_t test_string_list()
{
	result_t ret;
	string_list_p list = NULL;
	INIT(ret);
	list = string_list_new();
	ASSERT(list!=NULL, TRUE, ret, __LINE__);
	SUBRUN(test_string_list_add(list), ret);
	string_list_clear(list);
	ASSERT(list->count, 0, ret, __LINE__);
	ASSERT(list->size, 0, ret, __LINE__);
	ASSERT(list->items, NULL, ret, __LINE__);
	SUBRUN(test_string_list_add(list), ret);
	SUBRUN(test_string_list_set(list), ret);
	string_list_free(list);	PASS(ret);
	return ret;
}

result_t test_string_wildcmp()
{
	result_t ret;
	char inputString[] = "ThisIsAnInputStringForTest";
	char *cases[] = {NULL, ""	//null or empty
		, "ABC", "ThisIs", "ThisIsAnInputStringForTest", "thisisaninputstringfortest", "ThisIsAnInputStringForTestABC"			//no wildcard
		,"?hisIsAn\\InputStringForTest", "Thi\\sIsAnInputStringForTes?", "Th\\isIsAnInpu?StringForTest", "?ThisIsAn\\InputStringForTest", "Th\\isIsAnInputStringForTest?"		//? with escape
		,"?hisIsAnInputStringForTest", "ThisIsAnInputStringForTes?", "ThisIsAnInpu?StringForTest", "?ThisIsAnInputStringForTest", "ThisIsAnInputStringForTest?"		//?
		,"*ThisIsAnInputStringForTest", "*ThisIsAnInputStringForTest*", "ThisIsAnInputStringForTest*"	//* match 0 chars
		,"\\*ThisIsAnInputStringForTest", "ThisIsAnInputStringForTest\\*", "ThisIsAnInputStringForTest\\*"	//escape *
		,"*ForTest", "ThisIs*", "ThisIs*ForTest", "*", "*TesT"	//*
		,"**", "*?*", "ThisIsAnInp??StringForTest"	//combination
	};
	BOOL expects[] = {FALSE, FALSE,	//NULL or empty
		FALSE, FALSE, TRUE, FALSE, FALSE,	//no wildcard
		TRUE, TRUE, TRUE, FALSE, FALSE,	//? with escape
		TRUE, TRUE, TRUE, FALSE, FALSE,	//?
		TRUE, TRUE, TRUE,	//*match 0 chars
		FALSE, FALSE, FALSE, //escape *
		TRUE, TRUE, TRUE, TRUE, FALSE,	//*
		TRUE, TRUE, TRUE	//combination
	};
	int n = sizeof(cases)/sizeof(char*);
	int nexpects = sizeof(expects)/sizeof(BOOL);
	INIT(ret);
	if(n!=nexpects)
	{
		printf("ERROR!!!%s():Count of expected results(%d) doesn't match to test cases(%d)!\n", __FUNCTION__, nexpects, n);
	}
	else
	{
		int i;
		for(i=0; i<n; i++)
		{
			BOOL actual = string_wildcmp(inputString, cases[i]);
			ASSERT(actual, expects[i], ret, i);
		}
	}
	return ret;
}
result_t test_string()
{
	result_t ret;
	INIT(ret);
	SUBRUN(test_string_wildcmp(), ret);
	SUBRUN(test_string_isNullOrSpace(), ret);
	SUBRUN(test_string_split(), ret);
	SUBRUN(test_strings_index_of(), ret);
	SUBRUN(test_string_starts_with(), ret);
	SUBRUN(test_string_ends_with(), ret);
	SUBRUN(test_string_isTrue(), ret);
	SUBRUN(test_string_toLower(), ret);

	return ret;
}