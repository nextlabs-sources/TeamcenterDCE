#ifndef __JAVA_METHOD_H__
#define __JAVA_METHOD_H__

#include <jni.h>
#include "JavaClass.h"
#include <Windows.h>
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

class JavaMethod
{
private:
	jmethodID mid;
	JavaClass* pClass;
	BOOL isStatic;
	void Init(JavaClass*pClass,const char* wzMethod,const char*wzSig,BOOL isStatic);
	BOOL InvokeV_Inner(JavaClass* pClass,...);
	
public:
	JavaMethod(JavaClass*pClass,const char* wzMethod,const char*wzSig,BOOL isStatic);
	BOOL InvokeV();
	BOOL InvokeV(jobject obj,...);

	jvalue Invoke(JTYPE jType,...);
	jvalue Invoke(jobject obj,JTYPE jType,...);
};
#endif
