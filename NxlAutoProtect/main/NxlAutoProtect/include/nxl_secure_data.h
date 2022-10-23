#pragma once
#include <tc_utils.h>

//logical is_secure_data_enabled(tag_info_ro tagInfo);
logical dataset_need_secure(tag_info_ro tagInfo, logical *oIsDefault);
tag_t secure_dataset(tag_info_ro tagInfo, char context[200]);
