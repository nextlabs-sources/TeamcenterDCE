#ifndef __JAVACALLER_H__
#define __JAVACALLER_H__

#include "jni.h"
//	  jboolean z;
//    jbyte    b;
//    jchar    c;
//    jshort   s;
//    jint     i;
//    jlong    j;
//    jfloat   f;
//    jdouble  d;
//    jobject  l;
typedef enum _JTYPE
{
	JBOOLEAN=0,
	JBYTE,
	JCHAR,
	JSHORT,
	JINT,
	JLONG,
	JFLOAT,
	FDOUBLE,
	JOBJECT
}JTYPE;

class JavaCaller
{
private:
	JavaVM* jvm;
	JNIEnv* env;
	jclass jClass;
	jmethodID jMethod;
	void Init(JavaVMInitArgs*pArgs,const char*wzClass);
public:
	JavaCaller::JavaCaller(JavaVMInitArgs* pArgs);
	JavaCaller(JavaVMInitArgs* pArgs,const char* wzClass);
	~JavaCaller(){if(jvm!=NULL)jvm->DestroyJavaVM();}
	jmethodID Associate(const char*wzClass,const char*wzMethod, BOOL isStaticMethod,const char*wzSignature);

	JNIEnv* GetJavaEnv(){return env;};
	virtual BOOL InvokeV(BOOL bStatic,...);
	virtual jvalue Invoke(JTYPE jType,BOOL bStatic,...);



};
#endif
