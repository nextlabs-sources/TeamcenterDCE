#include "stdafx.h"
#include "PartFile.h"
#include "util.h"
#include "JavaMethod.h"
#include "log.h"
#include <string>
#include <fstream>
#include <windows.h>

PartFile::PartFile(string filePath, JavaEnvironment *pJavaEnv, JNIEnv *env)
{
	this->filePath = filePath;
	this->pJavaEnv = pJavaEnv;
	this->env = env;
}

void PartFile::ProcessDecryption() {
	SYSLOG("Performing Decryption before original nx_edit_reference.exe");
	// Rename protected file to .nxl extension first
	string oldname = this->filePath;
	string newname = oldname + ".nxl";

	if (rename(oldname.c_str(), newname.c_str()) == 0)
		SYSLOG("File renamed to protected version: %s", newname.c_str());
	else
		ERRLOG("Failed to rename file to protected version");

	// Start RightsManager. Then read and store MetaData Tag
	JavaClass *pNxlUtil = new JavaClass(pJavaEnv, "com/nextlabs/drm/NxlUtil");
	jstring intputFilePath = env->NewStringUTF(newname.c_str());
	JavaMethod *pReadMetadata = new JavaMethod(pNxlUtil,"readMetadataString","(Ljava/lang/String;)Ljava/lang/String;",TRUE);
	jvalue jval = pReadMetadata->Invoke(JOBJECT, intputFilePath);
	const char* metaString = env->GetStringUTFChars((jstring)jval.l, 0);

	SYSLOG("Metadata Tags: %s", metaString);
	this->setMetaTag(metaString);

	// Perform Decryption. Delete the source file (with .nxl extension)
	JavaMethod *pDecryptMethod = new JavaMethod(pNxlUtil, "decryptFile", "(Ljava/lang/String;)Z", TRUE);
	jvalue decryptionResult = pDecryptMethod->Invoke(JOBJECT, intputFilePath);
	if (decryptionResult.z) {
		SYSLOG("Decryption Success");
		if (remove(newname.c_str()) != 0)
			ERRLOG("Error Deleting file: %s", newname.c_str());
		else
			SYSLOG("Source File (.nxl) successfully deleted");
	}
	else
		SYSLOG("Decryption Failed");
}

void PartFile::ProcessEncryption() {
	SYSLOG("Performing Encryption after original nx_edit_reference.exe finished");

	string oldname = this->filePath;
	string newname = oldname + ".nxl";

	// Start RightsManager and perform encryption
	JavaClass *pNxlUtil = new JavaClass(pJavaEnv, "com/nextlabs/drm/NxlUtil");
	jstring intputFilePath = env->NewStringUTF(oldname.c_str());
	jstring metadataString = env->NewStringUTF(this->getMetaTag().c_str());
	JavaMethod *pEncryptMethod = new JavaMethod(pNxlUtil, "encryptFile", "(Ljava/lang/String;Ljava/lang/String;)Z", TRUE);
	jvalue encryptionResult = pEncryptMethod->Invoke(JOBJECT, intputFilePath, metadataString);

	if (encryptionResult.z == TRUE)
	{
		// After Decryption succeed. There will be 2 files : a.prt and a.prt.nxl.
		SYSLOG("Encryption Success");
		if (std::ifstream(oldname.c_str())) {
			// Need to remove a.prt file (original file)
			if (remove(oldname.c_str()) == 0) {
				SYSLOG("Removed source file");
			}
			// Rename a.prt.nxl to a.prt (content is still protected) to be able to pack to .bcz file
			if (rename(newname.c_str(), oldname.c_str()) == 0) {
				SYSLOG("Renamed encrypted file to non .nxl extension. File is protected but without .nxl extension to be packaged to .bcz");
			}
			// bczModifier will change a.prt back to a.prt.nxl after this (inside bcz file)
		}

	}
	else
		ERRLOG("Encyption Failed");
}

void PartFile::setMetaTag(string metaTag)
{
	this->metaTag = metaTag;
}

string PartFile::getMetaTag()
{
	return this->metaTag;
}
