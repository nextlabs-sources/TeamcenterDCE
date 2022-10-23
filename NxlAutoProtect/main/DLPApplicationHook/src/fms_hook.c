#include "utils_log.h"
#include "utils_jni.h"
#include "utils_string.h"
#include "utils_rmc.h"
#include "utils_mem.h"
#include "dlp_utils.h"

#define NXL_EXT ".nxl"

JNIEXPORT jint JNICALL Java_com_teamcenter_fms_external_hook_DLPApiHook_DLPApplicationHook  (JNIEnv *env, jobject obj, jstring jsFileName, jchar jClassCode)
{
	//print arguments
	const char *fileName = (*env)->GetStringUTFChars(env, jsFileName, NULL);
	char code = (char)jClassCode;
	SCOPE_START;
	DBGLOG("Env=%d FileName=%s JCode='%c'(%d) Code='%c'(%d)", env, fileName, jClassCode, jClassCode, code, code);
	DTLLOG("Context Jobject Class Name=%s", JNI_get_class_name(env, obj));
	if(file_is_protected(fileName))
	{
		//
		DBGLOG("==>Protected");
		if(RMC_is_installed())
		{
			//rename file if doesn't have .nxl ext
			if(!file_is_nxl(fileName))
			{
				//rename file
				char nxlFile[MAX_PATH];
				sprintf_s(nxlFile, MAX_PATH, "%s"NXL_EXT, fileName);
				DBGLOG("Renaming %s to %s", fileName, nxlFile);
				if(!FILE_RENAME(fileName, nxlFile))
				{
					ERRLOG("==>Failed(%d)", error);
				}
			}
		}
		else
		{
			ERRLOG("RMC is required to open this file");
		}
	}
CLEAN_UP:
	//release file name
	(*env)->ReleaseStringUTFChars(env, jsFileName, fileName);
	SCOPE_END;
	return 0;
}