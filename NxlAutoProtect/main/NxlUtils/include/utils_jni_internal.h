#ifndef NXL_UTILS_JNI_INTERNAL_H_INCLUDED
#define NXL_UTILS_JNI_INTERNAL_H_INCLUDED
#include "utils_jni.h"
#include "hashtable.h"
#include "utils_string_list.h"
#include "key_value_list.h"

#define JNI_VERSION JNI_VERSION_1_6

int JNI_getClass_global(jclass *clsId, JNIEnv *env, const char *clsName);//0=JNI_OK
int JNI_getMethod_global(jmethodID *methodID, JNIEnv *env, const jclass cls, const char *name, const char *sig);//0=JNI_OK
int JNI_make_global(jobject *global, JNIEnv *env, jobject obj);
//int jattrs_to_string_list(JNIEnv *env, jobject jattrs, char ***strArray);
//int jattrs_create(JNIEnv *env, hashtable_ro attrs, jobject *jattrs);

#define DELETE_REF(j) if(j!=NULL){(*env)->DeleteLocalRef(env, j);}
#define DELETE_GLOBAL(j) if(j!=NULL){(*env)->DeleteGlobalRef(env, j);j=NULL;}

int JNI_checkException(JNIEnv *env, const char *call, const void *ret, const char *func, const char *file, int line, BOOL checkNull);//0=JNI_OK
string_list_p jattrs_to_strings(JNIEnv *env, jobject jattrs);
int jattrs_create(JNIEnv *env, kvl_ro attrs, jobject *jattrs);
int jbytearray_copy(JNIEnv * env, jbyteArray target, jsize istart, jbyteArray src, jsize ifrom, jsize numToCopy);

#define JNI_CALL(x) JNI_checkException(env, #x, (x), __FUNCTION__, __FILE__, __LINE__, TRUE);
#define JNI_EVAL(x) (JNI_checkException(env, #x, (x), __FUNCTION__, __FILE__, __LINE__, TRUE)==JNI_OK)
#define JNI_CALL2(x) JNI_checkException(env, #x, (x), __FUNCTION__, __FILE__, __LINE__, FALSE)
#define JNI_EVAL2(x) (JNI_checkException(env, #x, (x), __FUNCTION__, __FILE__, __LINE__, FALSE)==JNI_OK)
#define JNI_CALL_void(x) x;JNI_checkException(env, #x, NULL, __FUNCTION__, __FILE__, __LINE__, FALSE);

//static clss/method variable
//Java Exception
extern jclass excepCls;
extern jmethodID excepGetMsgMethod;
extern jmethodID excepToStrMethod;
extern jmethodID excepPrintSTMethod;
#endif