#pragma once
//#include "com_nextlabs_teamcenter_fms_rmx_RMXInstance.h"
#include "com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance.h"

jbyteArray as_byte_array(JNIEnv *, unsigned char*, int);

unsigned char* as_unsigned_char_array(JNIEnv *, jbyteArray);
