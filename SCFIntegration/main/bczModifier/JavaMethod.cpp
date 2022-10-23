#include "stdafx.h"
#include "JavaMethod.h"
#include "JavaClass.h"
#include "log.h"

void JavaMethod::Init(JavaClass* pClass, const char* wzMethod, const char *wzSig, BOOL isStatic)
{
	mid = NULL;
	this->pClass = pClass;
	this->isStatic = isStatic;

	jclass cls = pClass->GetClass();
	JNIEnv *env = pClass->GetEnv();
	if(cls != NULL)
	{
		if(isStatic == TRUE)
			mid = env->GetStaticMethodID(cls, wzMethod, wzSig);
		else
			mid = env->GetMethodID(cls, wzMethod, wzSig);
	}
}

JavaMethod::JavaMethod(JavaClass* pClass, const char* wzMethod, const char* wzSig, BOOL isStatic)
{
	Init(pClass, wzMethod, wzSig, isStatic);
}

jvalue JavaMethod::Invoke(jobject obj, JTYPE jType, ...)
{
	va_list list;
	va_start(list, jType);
	jvalue retValue;
	memset(&retValue, 0, sizeof(retValue));
	JNIEnv *env = this->pClass->GetEnv();
	jclass jClass = pClass->GetClass();
	jmethodID jMethod = this->mid;

	if(isStatic == TRUE)
		DBGLOG("Static method can't be called with object instance");
	else
	{
		switch(jType){
		case JBOOLEAN:
			retValue.z = env->CallBooleanMethodV(obj, jMethod, list);
			break;
		case JBYTE:
			retValue.b = env->CallByteMethodV(obj, jMethod, list);
			break;
		case JCHAR:
			retValue.c = env->CallCharMethodV(obj, jMethod, list);
			break;
		case JSHORT:
			retValue.s = env->CallShortMethodV(obj, jMethod, list);
			break;
		case JINT:
			retValue.i = env->CallIntMethodV(obj, jMethod, list);
			break;
		case JLONG:
			retValue.j = env->CallLongMethodV(obj, jMethod, list);
			break;
		case JFLOAT:
			retValue.f = env->CallFloatMethodV(obj, jMethod, list);
			break;
		case FDOUBLE:
			retValue.d = env->CallDoubleMethodV(obj, jMethod, list);
			break;
		case JOBJECT:
			retValue.l = env->CallObjectMethodV(obj, jMethod, list);
			break;
		}
	}
	return retValue;
}
jvalue JavaMethod::Invoke(JTYPE jType,...)
{
	va_list list;
	va_start(list, jType);
	jvalue retValue;
	JNIEnv *env = this->pClass->GetEnv();
	jclass jClass = pClass->GetClass();
	jmethodID jMethod = this->mid;
	if(isStatic == TRUE)
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
		DBGLOG("Non-Static method can't be called with class");
	va_end(list);
	return retValue;
}

BOOL JavaMethod::InvokeV_Inner(JavaClass*pClass,...)
{
	va_list list;
	va_start(list, pClass);
	JNIEnv *env = this->pClass->GetEnv();
	jclass jClass = pClass->GetClass();
	jmethodID jMethod = this->mid;

	if(isStatic == TRUE)
		env->CallStaticVoidMethodV(jClass, jMethod, list);
	else
		DBGLOG("Non-Static method can't be called with class"); //env->CallVoidMethodV(jClass,jMethod,list);
	va_end(list);
	return TRUE;
}

BOOL JavaMethod::InvokeV()
{
	return this->InvokeV_Inner(pClass);
}

BOOL JavaMethod::InvokeV(jobject obj,...)
{
	va_list list;
	va_start(list, obj);
	JNIEnv *env = this->pClass->GetEnv();
	jclass jClass = pClass->GetClass();
	jmethodID jMethod = this->mid;

	if(isStatic == TRUE)
		DBGLOG("Static method can't be called with object instance");
	else
		env->CallVoidMethodV(obj, jMethod, list);
	va_end(list);
	return TRUE;
}
