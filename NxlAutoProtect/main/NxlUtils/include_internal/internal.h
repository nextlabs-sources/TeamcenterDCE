#ifndef NXL_UTILS_INTERNAL_STRING_LIST_H_INCLUDED
#define NXL_UTILS_INTERNAL_STRING_LIST_H_INCLUDED
#include <utils_string_list.h>
#include <key_value_list.h>
#include <utils_common.h>

char *string_ncopy(const char *str, const int nchar);

int string_list_add_internal(string_list_p list, char *newItem);
BOOL string_list_set_internal(string_list_p list, char *newItem, int index);
void string_list_ensure_internal(string_list_p list, unsigned int targetSize);

int kvl_set_internal(kvl_p list, char *key, char *val);
#endif