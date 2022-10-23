#ifndef __JAVA_ENVIRONMENT_H__
#define __JAVA_ENVIRONMENT_H__


#ifdef __cplusplus
extern "C"
{
#endif
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


#ifdef __cplusplus
}
#endif


#endif