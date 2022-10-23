/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_nextlabs_bbrmx_BBRMXInstance */

#ifndef _Included_com_nextlabs_bbrmx_BBRMXInstance
#define _Included_com_nextlabs_bbrmx_BBRMXInstance
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    loadDRmUser
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_loadDRmUser
  (JNIEnv *, jobject);

/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    loadDRmInstance
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_loadDRmInstance
  (JNIEnv *, jobject);

/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    addTrustedProcess
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_addTrustedProcess
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    addRPMDir
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_addRPMDir
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    protectFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_protectFile
  (JNIEnv *, jobject, jstring, jstring, jstring, jboolean);

/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    isFileProtected
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_isFileProtected
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_nextlabs_bbrmx_BBRMXInstance
 * Method:    readNxlTags
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_readNxlTags
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif