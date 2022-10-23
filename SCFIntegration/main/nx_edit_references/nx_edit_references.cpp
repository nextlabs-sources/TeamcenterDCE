#include "stdafx.h"
#include <Windows.h>
#include "CommandLineHandler.h"
#include "log.h"
#include "util.h"
#include "PartListFile.h"
#include "PartFile.h"
#include "JavaEnvironment.h"
#include "JavaClass.h"
#include "JavaMethod.h"
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	// Logfile will only enable if system environment variable "NXL_SCF_LOG" is set
//	char* buffer = nullptr;
//	size_t bufferSize = 0;
//	//if (_dupenv_s(&buffer, &bufferSize, "NXL_SCF_LOG") == 0 && buffer != nullptr) {
//	//	SYSLOG("Log File is enable");
//	//	init_nx_edit_ref_log();
//	//}
//	init_nx_edit_ref_log();
//
//	// Get RMX_ROOT
//	string rmxRoot;
//	buffer = nullptr;
//	bufferSize = 0;
//	if (_dupenv_s(&buffer, &bufferSize, "RMX_ROOT") == 0 && buffer != nullptr)
//	{
//		SYSLOG("RMX_ROOT=%s", buffer);
//		rmxRoot = buffer;
//		free(buffer);
//	} else {
//		ERRLOG("RMX_ROOT is not defined");
//		throw "RMX_ROOT is not defined";
//	}
//
//	// Build Java Environment
//	JavaVMInitArgs args;
//	args.ignoreUnrecognized = JNI_FALSE;
//	args.version = JNI_VERSION_1_6;
//	JavaVMOption options[1];
//	args.nOptions = sizeof(options)/sizeof(JavaVMOption);
//
//	string classpathString = "-Djava.class.path=";
//	string jarDir = rmxRoot + "\\BBIntegration\\";
//	const int numberOfJars = 15;
//	string jarList[numberOfJars] = {
//		"bbExtension",
//		"bcpkix-jdk15on-1.57",
//		"bcprov-jdk15on-1.57",
//		"com.nextlabs.drm.rmx.configuration",
//		"common-framework",
//		"commons-cli-1.2",
//		"commons-codec-1.10",
//		"commons-lang3-3.5",
//		"crypt",
//		"guava-18.0",
//		"gson-2.3.1",
//		"log4j-api-2.10.0",
//		"log4j-core-2.10.0",
//		"rmjavasdk-ng",
//		"shared"
//	};
//
//	for(int i = 0; i < numberOfJars ; i ++ )
//	{
//		classpathString += jarDir + jarList[i] + ".jar;";
//	}
//
//	char *classpathCString = new char[classpathString.length() + 1];
//	strcpy_s(classpathCString, classpathString.length() + 1, classpathString.c_str());
//
//	SYSLOG("Java_Classpath=%s", classpathCString);
//	options[0].optionString = classpathCString;
//	args.options = options;
//
//	JavaEnvironment *pJavaEnv = new JavaEnvironment(&args);
//	JNIEnv *env = pJavaEnv->GetJavaEnv();
//
//	// Main Process
//	CommandLineHandler *pCmdLineHandler = new CommandLineHandler(argc, argv);
//	pCmdLineHandler->Parse();
//	PrintLogW(L"nx_edit_references!PartList File: %s", pCmdLineHandler->GetPartListFile());
//	SYSLOG("PartList File: %s", char(pCmdLineHandler->GetPartListFile()));
//	vector<PartFile> vecPartFiles;
//	do
//	{
//		if(TRUE==IsFileExistingW(pCmdLineHandler->GetPartListFile()))
//		{
//			PartListFile partListFile(pCmdLineHandler->GetPartListFile());
//			vector<string> partFiles=partListFile.GetLines();
//			size_t partFileCount=partFiles.size();
//			vecPartFiles.clear();
//			for(size_t index=0;index<partFileCount;index++)
//			{
//				string strFile=partFiles[index];
//				SYSLOG("Part File: %s",strFile.c_str());
//				if( TRUE==::IsFileExistingA(strFile.c_str()) && TRUE==::IsNxlFile(strFile.c_str()))
//				{
//					PartFile partFile(strFile.c_str(), pJavaEnv, env);
//					partFile.ProcessDecryption();
//					vecPartFiles.push_back(partFile);
//				}
//			}
//		}
//		else
//			PrintLogW(L"PartList File is not existing at %s",pCmdLineHandler->GetPartListFile());
//	}
//	while(0);
//
//
//	PrintLogW(L"nx_edit_references!The Inner Command Line: %s", pCmdLineHandler->GetInnerCommandLine());
//	SYSLOG("The Inner Command Line: %s", char(pCmdLineHandler->GetInnerCommandLine()));
//	int iCommandRet=_wsystem(pCmdLineHandler->GetInnerCommandLine());
//	SYSLOG("Return %d from nx_edit_references_inner.exe", iCommandRet);
//	if(pCmdLineHandler!=NULL)
//		delete pCmdLineHandler;
//
//
//	SYSLOG("NumberPartFiles=%d", vecPartFiles.size());
//	while(!vecPartFiles.empty())
//	{
//		vecPartFiles.back().ProcessEncryption();
//		vecPartFiles.pop_back();
//	}
//
//	return iCommandRet;
//}

int _tmain(int argc, _TCHAR* argv[])
{
	init_nx_edit_ref_log();
	vector<PartFile> vecPartFiles;
	CommandLineHandler *pCmdLineHandler = new CommandLineHandler(argc,argv);
	pCmdLineHandler->Parse();
	PrintLogW(L"nx_edit_ref!: The Part List File: %s", pCmdLineHandler->GetPartListFile());

	// Creating JVM argument (jdk version, classpath, ...)
	JavaVMInitArgs args;
	args.ignoreUnrecognized = JNI_FALSE;
	args.version = JNI_VERSION_1_6;
	JavaVMOption options[1];
	args.nOptions = sizeof(options)/sizeof(JavaVMOption);
	string optionTempString = "-Djava.class.path=";

	// Get RMX_ROOT env
	string rmx_root;
	char* buf = nullptr;
	size_t sz = 0;
	if (_dupenv_s(&buf, &sz, "RMX_ROOT") == 0 && buf != nullptr)
	{
		DBGLOG("RMX_ROOT = %s", buf);
		rmx_root = buf;
		free(buf);
	} else {
		DBGLOG("RMX_ROOT is not defined");
		throw "RMX_ROOT not defined";
	}

	// RM Java SDK Library (BBIntegration folder) + NextLabs Briefcase Extension Jar
	string jarDir = rmx_root;
	jarDir += "\\BBIntegration\\";
	const int jarNum = 15;
	string jarNames[jarNum] = {
		"bcpkix-jdk15on-1.57",
		"bcprov-jdk15on-1.57",
		"common-framework",
		"commons-cli-1.2",
		"commons-codec-1.10",
		"commons-lang3-3.5",
		"crypt",
		"guava-18.0",
		"gson-2.3.1",
		"json-20180813",
		"log4j-api-2.10.0",
		"log4j-core-2.10.0",
		"rmjavasdk-ng",
		"shared",
		"bbExtension"
	};
	for(int i = 0 ; i < jarNum ; i ++ ) {
		optionTempString += jarDir + jarNames[i] + ".jar;";
	}

	char *cstr = new char[optionTempString.length() + 1];
	strcpy_s(cstr, optionTempString.length() + 1, optionTempString.c_str());

	DBGLOG("Java Class Path: %s", cstr);
	options[0].optionString = cstr;
	args.options = options;

	JavaEnvironment *pJavaEnv = new JavaEnvironment(&args);
	JNIEnv *env = pJavaEnv->GetJavaEnv();

	// Main function
	if(IsFileExistingW(pCmdLineHandler->GetPartListFile()) == TRUE)
	{
		PartListFile partListFile(pCmdLineHandler->GetPartListFile());
		vector<string> partFiles = partListFile.GetLines();
		size_t partFileCount = partFiles.size();
		vecPartFiles.clear();
		for(size_t index = 0; index < partFileCount; index ++)
		{
			string strFile = partFiles[index];
			DBGLOG("Part File: %s", strFile.c_str());
			if( IsFileExistingA(strFile.c_str()) == TRUE && IsNxlFile(strFile.c_str()) == TRUE)
			{
				PartFile partFile(strFile, pJavaEnv, env);
				partFile.ProcessDecryption();
				vecPartFiles.push_back(partFile);
			}
		}
	}
	else {
		PrintLogW(L"nx_edit_ref!: Part List file not existing: %s", pCmdLineHandler->GetPartListFile());
	}

	int iCommandRet=_wsystem(pCmdLineHandler->GetInnerCommandLine());
	DBGLOG("Return %d from nx_edit_references_inner.exe", iCommandRet);

	if(pCmdLineHandler != NULL) {
		delete pCmdLineHandler;
	}

	DBGLOG("Number of Part File processing: %d", vecPartFiles.size());
	while(!vecPartFiles.empty())
	{
		vecPartFiles.back().ProcessEncryption();
		vecPartFiles.pop_back();
	}

	return iCommandRet;
}
