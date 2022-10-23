#include "stdafx.h"
#include "JavaClass.h"
#include "JavaEnvironment.h"
#include "log.h"
JavaClass::JavaClass(JavaEnvironment*javaEnv,const char* wzClass)
{
	cls=NULL;
	this->javaEnv=javaEnv;

	JNIEnv* env=javaEnv->GetJavaEnv();
	cls = env->FindClass(wzClass);
	if(cls==NULL)
	{
		if (env->ExceptionOccurred())
		{
			env->ExceptionDescribe();
			env->ExceptionClear();
		}
		ERRLOG("Failed to find Java class %s",wzClass);
	}
}
