#include "stdafx.h"
#include "JavaEnvironment.h"

void JavaEnvironment::Init(JavaVMInitArgs* pArgs)
{
	jvm = NULL;
	env = NULL;
	jint rc = JNI_CreateJavaVM(&jvm, (void **)&env, pArgs);
	if(rc != JNI_OK )
	{
		if (env->ExceptionOccurred())  
		{  
			env->ExceptionDescribe();           
			env->ExceptionClear(); 
       }  
	}
}
JavaEnvironment::~JavaEnvironment()
{
	if(jvm != NULL)
		jvm->DestroyJavaVM();
}
JavaEnvironment::JavaEnvironment(JavaVMInitArgs *pArgs)
{
	Init(pArgs);
}