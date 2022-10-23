#include "test.h"
#include "utils_jni.h"
#include <utils_system.h>
#include <utils_string.h>

result_t test_jni()
{
	int jni;
	char jpcSDK[MAX_PATH];
	result_t ret;
	JNIEnv *env;
	INIT(ret);
	
#if defined(WNT)
	get_module_dir(TARGETFILENAME, jpcSDK, MAX_PATH);
#elif defined(__linux__)
	get_lib_dir((void*)test_jni, jpcSDK, MAX_PATH);
#endif
	strcat_s(jpcSDK, MAX_PATH, PATH_DELIMITER);
	strcat_s(jpcSDK, MAX_PATH, "nlJavaSDK2.jar");

	jni = JNI_init(&env, jpcSDK);
	ASSERT(jni, JNI_OK, ret, __LINE__);
	
	JNI_detach();
	PASS(ret);
	
	jni = JNI_init(&env, jpcSDK);
	ASSERT(jni, JNI_OK, ret, __LINE__);

	
	JNI_detach();
	PASS(ret);

	return ret;
}