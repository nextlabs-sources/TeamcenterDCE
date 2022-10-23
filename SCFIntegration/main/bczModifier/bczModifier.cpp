#include "stdafx.h"
#include <Windows.h>
#include "log.h"
#include "util.h"
#include "JavaEnvironment.h"
#include "JavaClass.h"
#include "JavaMethod.h"
#include <string>
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	// convert wchar_t to string
	wstring ws( argv[1] );
	string s( (const char*)&ws[0], sizeof(wchar_t) / sizeof(char) * ws.size() );
	std::string bczFilePath = "";
	for( size_t i = 0; i < s.length(); i++ )
	{
		if( i % 2 == 0 )
			bczFilePath += s[i];
	}
	
	// Logfile will only enable if system environment variable "NXL_SCF_LOG" is set
	char* buffer = nullptr;
	size_t bufferSize = 0;
	if (_dupenv_s(&buffer, &bufferSize, "NXL_SCF_LOG") == 0 && buffer != nullptr) {
		SYSLOG("Log File is enable");
		init_bcz_modifier_log();
	}

	// Get RMX_ROOT
	string rmxRoot;
	buffer = nullptr;
	bufferSize = 0;
	if (_dupenv_s(&buffer, &bufferSize, "RMX_ROOT") == 0 && buffer != nullptr)
	{
		SYSLOG("RMX_ROOT=%s", buffer);
		rmxRoot = buffer;
		free(buffer);
	} else {
		ERRLOG("RMX_ROOT is not defined");
		throw "RMX_ROOT is not defined";
	}

	SYSLOG("Processing following Briefcase file: %s", bczFilePath.c_str());

	JavaVMInitArgs args;
	args.ignoreUnrecognized = JNI_FALSE;
	args.version = JNI_VERSION_1_6;
	JavaVMOption options[1];
	args.nOptions = sizeof(options)/sizeof(JavaVMOption);

	string classpathString = "-Djava.class.path=";
	string jarDir = rmxRoot + "\\BBIntegration\\";
	classpathString += jarDir + "bbExtension.jar;";
	classpathString += jarDir + "log4j-api-2.10.0.jar;";
	classpathString += jarDir + "log4j-core-2.10.0.jar;";

	char *classpathCStr = new char[classpathString.length() + 1];
	strcpy_s(classpathCStr, classpathString.length() + 1, classpathString.c_str());
	
	SYSLOG("Java ClassPath: %s", classpathCStr);
	options[0].optionString = classpathCStr;
	args.options = options;

	JavaEnvironment *pJavaEnv = new JavaEnvironment(&args);
	JNIEnv *env = pJavaEnv->GetJavaEnv();

	JavaClass *pZipTest = new JavaClass(pJavaEnv,"com/nextlabs/drm/bczhelper/BczHelper");
	JavaMethod *pRename = new JavaMethod(pZipTest, "RenameToNxl","(Ljava/lang/String;)Z", TRUE);
	jstring intputFilePath = env->NewStringUTF(bczFilePath.c_str());
	jvalue jval = pRename->Invoke(JOBJECT, intputFilePath);
	if (jval.z == TRUE)
		SYSLOG("Rename operation success!");
	else
		ERRLOG("Rename operation failure!");

	return 0;
}
