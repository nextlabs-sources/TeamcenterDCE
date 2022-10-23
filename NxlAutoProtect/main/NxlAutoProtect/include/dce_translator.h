#ifndef NXL_DCE_TRANSLATOR_H_INCLUDED
#define NXL_DCE_TRANSLATOR_H_INCLUDED
#include "tc_utils.h"

//Translation Request Type
typedef enum request_type_e
{
	request_non = 0,
	request_protect = 1,
	request_update = 2,
	request_unprotect = 3,
	//request_reprotect = 4	//TODO:,
	request_refreshduid = 5
}request_type;

typedef enum priority_e
{
	priority_low = 1,
	priority_medium = 2,
	priority_high = 3
}request_priority;

tag_t send_translation_request(tag_info_ro dsInfo, tag_info_ro revInfo, kvl_ro additionalTags, const time_t scheduledTime, request_type requestType, request_priority priority, const char *reason);
#endif //NXL_DCE_TRANSLATOR_H_INCLUDED