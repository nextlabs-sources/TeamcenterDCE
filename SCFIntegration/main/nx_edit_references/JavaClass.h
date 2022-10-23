#ifndef __JAVA_CLASS_H__
#define __JAVA_CLASS_H__

#include <jni.h>
#include "JavaEnvironment.h"
class JavaClass
{
private:
	JavaEnvironment* javaEnv;
	jclass cls;
public:
	JavaClass(JavaEnvironment*javaEnv,const char* wzClass);
	jclass GetClass(){return cls;}
	JNIEnv * GetEnv(){return javaEnv->GetJavaEnv();}

};

#endif