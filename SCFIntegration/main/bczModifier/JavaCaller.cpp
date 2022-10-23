#include "stdafx.h"
#include <Windows.h>
#include "JavaCaller.h"
#include "log.h"

void JavaCaller::Init(JavaVMInitArgs* pArgs, const char* wzClass)
{
	jvm = NULL;
	env = NULL;
	jClass = NULL;
	jMethod = NULL;
	jint rc = JNI_CreateJavaVM(&jvm, (void **)&env, pArgs);
	if(rc == JNI_OK && wzClass != NULL)
	{
		jClass = env->FindClass(wzClass);
	}
	else
	{
		if (env->ExceptionOccurred())  
		{  
			env->ExceptionDescribe();           
			env->ExceptionClear(); 
       }  
	}
}

JavaCaller::JavaCaller(JavaVMInitArgs* pArgs)
{
	Init(pArgs, NULL);
}

JavaCaller::JavaCaller(JavaVMInitArgs* pArgs, const char* wzClass)
{
	Init(pArgs, wzClass);
}

jmethodID JavaCaller::Associate(const char* wzClass, const char* wzMethod, BOOL isStaticMethod, const char* wzSignature)
{
	if(wzClass == NULL)
		jClass = NULL;
	if(wzMethod == NULL)
		jMethod = NULL;

	if(wzClass == NULL || wzMethod == NULL)
		return NULL;

	jClass = env->FindClass(wzClass);
	if(jClass == NULL)
		return NULL;
	if(isStaticMethod == TRUE)
		jMethod = env->GetStaticMethodID(jClass, wzMethod, wzSignature);
	else
		jMethod = env->GetMethodID(jClass, wzMethod, wzSignature);
	if(jMethod == NULL)
		return NULL;
	return jMethod;

}

jvalue JavaCaller::Invoke(JTYPE jType, BOOL bStatic, ...)
{
	va_list list;
	va_start(list, bStatic);
	jvalue retValue;
	if(bStatic == TRUE)
	{
		switch(jType){
		case JBOOLEAN:
			retValue.z = env->CallStaticBooleanMethodV(jClass, jMethod, list);
			break;
		case JBYTE:
			retValue.b = env->CallStaticByteMethodV(jClass, jMethod, list);
			break;
		case JCHAR:
			retValue.c = env->CallStaticCharMethodV(jClass, jMethod, list);
			break;
		case JSHORT:
			retValue.s = env->CallStaticShortMethodV(jClass, jMethod, list);
			break;
		case JINT:
			retValue.i = env->CallStaticIntMethodV(jClass, jMethod, list);
			break;
		case JLONG:
			retValue.j = env->CallStaticLongMethodV(jClass, jMethod, list);
			break;
		case JFLOAT:
			retValue.f = env->CallStaticFloatMethodV(jClass, jMethod, list);
			break;
		case FDOUBLE:
			retValue.d = env->CallStaticDoubleMethodV(jClass, jMethod, list);
			break;
		case JOBJECT:
			retValue.l = env->CallStaticObjectMethodV(jClass, jMethod, list);
			break;
		}
	}
	else
	{
		switch(jType){
		case JBOOLEAN:
			retValue.z = env->CallBooleanMethodV(jClass, jMethod, list);
			break;
		case JBYTE:
			retValue.b = env->CallByteMethodV(jClass, jMethod, list);
			break;
		case JCHAR:
			retValue.c = env->CallCharMethodV(jClass, jMethod, list);
			break;
		case JSHORT:
			retValue.s = env->CallShortMethodV(jClass, jMethod, list);
			break;
		case JINT:
			retValue.i = env->CallIntMethodV(jClass, jMethod, list);
			break;
		case JLONG:
			retValue.j = env->CallLongMethodV(jClass, jMethod, list);
			break;
		case JFLOAT:
			retValue.f = env->CallFloatMethodV(jClass, jMethod, list);
			break;
		case FDOUBLE:
			retValue.d = env->CallDoubleMethodV(jClass, jMethod, list);
			break;
		case JOBJECT:
			retValue.l = env->CallObjectMethodV(jClass, jMethod, list);
			break;
		}
	}
	va_end(list);
	return retValue;
}

BOOL JavaCaller::InvokeV(BOOL bStatic, ...)
{
	va_list list;
	va_start(list, bStatic);
	if(bStatic == TRUE)
		env->CallStaticVoidMethodV(jClass, jMethod, list);
	else
		env->CallVoidMethodV(jClass, jMethod, list);
	va_end(list);
	return TRUE;
}