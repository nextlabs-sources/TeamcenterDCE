/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance */

#ifndef _Included_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance
#define _Included_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance
 * Method:    loadDRmUser
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance_loadDRmUser
  (JNIEnv *, jobject);

/*
 * Class:     com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance
 * Method:    buildNxlHeader
 * Signature: ([B[J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance_buildNxlHeader
  (JNIEnv *, jobject, jbyteArray, jlongArray);

/*
 * Class:     com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance
 * Method:    decryptPartial
 * Signature: ([B[B[BJJ)J
 */
JNIEXPORT jlong JNICALL Java_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance_decryptPartial
  (JNIEnv *, jobject, jbyteArray, jbyteArray, jbyteArray, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
