#ifndef NXL_METADATA_CONFIG_H_INCLUDED
#define NXL_METADATA_CONFIG_H_INCLUDED
#include <key_value_list.h>

//manually load metadata priority configuration
void metadata_priorities_load();
//compare the prioritiy of the two metadata values which has same key
//if(return == FALSE) compare unsuccessful, either the input key values is not valid or PRIORITIES IS NOT DEFINED!
//else if(*cmp > 0) val1 has higher priority than val2
//else if(*cmp < 0) val1 has lower priority than val2
//else if(*cmp == 0) val1 is duplicated with val2
BOOL metadata_compare(const char *key, const char *val1, const char *val2, int *cmp);

BOOL metadata_merge(kvl_p target, kvl_ro source);
#endif