#ifndef __PARTFILE_H__
#define __PARTFILE_H__

#include <string>
#include "JavaEnvironment.h"
#include "JavaClass.h"

using namespace std;
class PartFile
{
private:
	string filePath;
	string metaTag;
	JavaEnvironment *pJavaEnv;
	JNIEnv *env;
public:
	PartFile(string filePath, JavaEnvironment *pJavaEnv, JNIEnv *env);
	void ProcessDecryption();
	void ProcessEncryption();
	string getMetaTag();
	void setMetaTag(string metaTag);
};

#endif
