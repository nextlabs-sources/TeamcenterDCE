#ifndef NXL_UTILS_JNI_H_INCLUDED
#define NXL_UTILS_JNI_H_INCLUDED
#include <jni.h>
#include "nxl_utils_exports.h"
#include "utils_string_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

NXLAPI int JNI_init2(JNIEnv **env, string_list_ro jars);//0=JNI_OK
NXLAPI int JNI_init(JNIEnv **env, const char *classPath);//0=JNI_OK
NXLAPI void JNI_detach();
//Returns the class name of the input jobject
//Output string is stored in a static array which will be overriden in next call
NXLAPI const char *JNI_get_class_name(JNIEnv *env, jobject obj);

#ifdef __cplusplus
}
#endif

#endif