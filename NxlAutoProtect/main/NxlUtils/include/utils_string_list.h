#ifndef NXL_UTILS_STRING_LIST_H_INCLUDED
#define NXL_UTILS_STRING_LIST_H_INCLUDED
#include "nxl_utils_exports.h"
#include "utils_common.h"

/*	Note:
	1, Use string_list_* to manipulate this structure, don't tempt the member directly
	2, Create a copy of items/items[i] when you need it for external use
*/
typedef struct string_list_s
{
	unsigned int count;
	unsigned int size;
	char **items;
}*string_list_p;
typedef const struct string_list_s *string_list_ro;

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI string_list_p string_list_new();
NXLAPI void string_list_free(string_list_p list);
NXLAPI void string_list_clear(string_list_p list);

//input newItem will be duplicated
//returns index of new item
NXLAPI int string_list_add(string_list_p list, const char *newItem);
NXLAPI BOOL string_list_set(string_list_p list, const char *newItem, int index);
NXLAPI BOOL string_list_removeAt(string_list_p list, int index);

NXLAPI int string_list_index_of(string_list_p list, const char *item);

//return the number of sub string found in <str>
//<refList> need to be allocated before this call
NXLAPI int string_split(const char *str, const char *splitChars, string_list_p refList);

#ifdef __cplusplus
}
#endif

#endif