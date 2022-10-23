#ifndef __JAVA_ENVIRONMENT_H__
#define __JAVA_ENVIRONMENT_H__

#include <jni.h>

class JavaEnvironment
{
private:
	JavaVM * jvm;
	JNIEnv * env;
	void Init(JavaVMInitArgs* pArgs);
public:
	JavaEnvironment(JavaVMInitArgs *pArgs);
	~JavaEnvironment();
	JNIEnv* GetJavaEnv(){return env;}

};


#endif