#include "BBRMX.h"
#include "RMXInstance.h"
#include "RMXUtils.h"
#include <SDLInc.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <inttypes.h>
#include <sstream>

ISDRmUser* pDRmUser = nullptr;
ISDRmcInstance* pDRmInstance = nullptr;

JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_loadDRmUser(JNIEnv *env, jobject jobj) {
	bool result = CRMXInstance::GetInstance().Initialize();

	if (result) {
		pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
		return JNI_TRUE;
	}
	else {
		return JNI_FALSE;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_loadDRmInstance(JNIEnv *env, jobject jobj) {
	bool result = CRMXInstance::GetInstance().Initialize();

	if (result) {
		pDRmInstance = CRMXInstance::GetInstance().GetDRmcInstance();

		return JNI_TRUE;
	}
	else {
		return JNI_FALSE;
	}
}

bool isRPMDirStatus(DWORD sdrDirStatus) {
	return (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR || sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
}

bool isRPMDirectory(std::wstring nxlFilePath) {
	uint32_t dirStatus;
	SDRmRPMFolderOption option;
	std::wstring tags;
	pDRmInstance->IsRPMFolder(nxlFilePath, &dirStatus, &option, tags);
	bool isRPMDir = isRPMDirStatus(dirStatus);

	return isRPMDir;
}

std::string getUser_MembershipID() {
	bool adhoc;
	bool workspace = false;
	int heartbeat;
	int sysProjectId;
	std::string sysProjectTenantId, tenantId;

	if (pDRmUser->GetTenantPreference(&adhoc, &workspace, &heartbeat, &sysProjectId, sysProjectTenantId, tenantId) != 0) {
		return std::string();
	}

	std::string midSysProj = pDRmUser->GetMembershipID(sysProjectId);
	if (midSysProj.empty())
		return pDRmUser->GetMembershipID(sysProjectTenantId);

	return midSysProj;
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_addTrustedProcess(JNIEnv *env, jobject jobj, jlong processId) {
	long procId = (long)processId;
	SDWLResult result = pDRmInstance->RPMAddTrustedProcess(procId, LOGIN_PASSCODE);
	std::wstringstream debugStream;

	if (result.GetCode() == 0) {
		debugStream << L"successfully added trusted process id: " << procId << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return JNI_TRUE;
	}
	else {
		debugStream << L"failed to add trusted process id: " << procId << L" err: " << result.GetCode() << L"-" << result.GetMsg().c_str() << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return JNI_FALSE;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_addRPMDir(JNIEnv *env, jobject jobj, jstring folderPath) {
	const char *raw = env->GetStringUTFChars(folderPath, NULL);
	std::string tmpPath(raw);
	std::wstring targetFolderPath = RMXUtils::s2ws(tmpPath);
	std::wstringstream debugStream;

	if (isRPMDirectory(targetFolderPath))
		return JNI_TRUE;

	SDWLResult result = pDRmInstance->AddRPMDir(targetFolderPath, RPMFOLDER_OVERWRITE | RPMFOLDER_API);

	if (result.GetCode() == 0) {
		debugStream << L"successfully added RPM folder: " << targetFolderPath << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return JNI_TRUE;
	}
	else {
		debugStream << L"failed to add RPM folder: " << targetFolderPath << L" err: " << result.GetCode() << L"-" << result.GetMsg().c_str() << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return JNI_FALSE;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_protectFile(JNIEnv *env, jobject jobj, jstring inFilePath, jstring outFolderPath, jstring tags, jboolean withNxlExt) {
	const char *raw1 = env->GetStringUTFChars(inFilePath, NULL);
	std::string tmpPath1(raw1);
	std::wstring plainFilePath = RMXUtils::s2ws(tmpPath1);
	std::wstring plainFilePathNxlExt = RMXUtils::s2ws(tmpPath1);

	const char *raw2 = env->GetStringUTFChars(outFolderPath, NULL);
	std::string tmpPath2(raw2);
	std::wstring nxlFilePath = RMXUtils::s2ws(tmpPath2);

	const char *tagString = env->GetStringUTFChars(tags, NULL);

	bool withNxlExtension = (bool)withNxlExt;

	bool isRPMDir = isRPMDirectory(nxlFilePath);
	
	// empty data
	std::vector<SDRmFileRight> rights;
	SDR_WATERMARK_INFO watermark;
	SDR_Expiration expire;

	SDWLResult result = pDRmUser->ProtectFile(plainFilePath, nxlFilePath, rights, watermark, expire, tagString, getUser_MembershipID());
	std::wstringstream debugStream;

	if (result.GetCode() == 0) {
		debugStream << L"successfully protected nxlFile: " << nxlFilePath.c_str() << std::endl;
		
		if (pDRmInstance->RPMDeleteFile(plainFilePath.c_str())) {
			// rename file from name with timestamp to orginal file name with nxl extension

			if (withNxlExtension)
				plainFilePathNxlExt += L".nxl";

			
			if (MoveFile(nxlFilePath.c_str(), plainFilePathNxlExt.c_str()) == 1)
				debugStream << L"successfully renamed nxlFile from " << nxlFilePath.c_str() << L" to " << plainFilePathNxlExt.c_str() << std::endl;
			else 
				debugStream << L"failed to rename nxlFile from " << nxlFilePath.c_str() << L" to " << plainFilePathNxlExt.c_str() << std::endl;

			OutputDebugStringW(debugStream.str().c_str());
			debugStream.str(L"");
			debugStream.flush();
		}

		return JNI_TRUE;
	} 
	else {
		debugStream << L"failed to protect nxlFile: " << nxlFilePath.c_str() << L" err: " << result.GetCode() << L"-" << result.GetMsg().c_str() << std::endl;
		OutputDebugStringW(debugStream.str().c_str());
		debugStream.str(L"");
		debugStream.flush();

		return JNI_FALSE;
	}
}

JNIEXPORT jboolean JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_isFileProtected(JNIEnv *env, jobject jobj, jstring filePath) {
	const char *raw = env->GetStringUTFChars(filePath, NULL);
	std::string tmpPath(raw);
	std::wstring nxlFilePath = RMXUtils::s2ws(tmpPath);

	if (pDRmUser->IsFileProtected(nxlFilePath)) {
		return JNI_TRUE;
	}
	else {
		return JNI_FALSE;
	}
}

jstring wsz2jstr(JNIEnv *env, std::wstring cstr) {
	int len = cstr.size();
	jchar* raw = new jchar[len];

	memcpy(raw, cstr.c_str(), len * sizeof(wchar_t));
	jstring result = env->NewString(raw, len);

	delete[] raw;

	return result;
}

JNIEXPORT jstring JNICALL Java_com_nextlabs_bbrmx_BBRMXInstance_readNxlTags(JNIEnv *env, jobject jobj, jstring inFilePath) {
	const char *raw = env->GetStringUTFChars(inFilePath, NULL);
	std::string tmpPath(raw);
	std::wstring nxlFilePath = RMXUtils::s2ws(tmpPath);

	std::wstringstream debugStream;

	std::wstring tags(L"");
	pDRmInstance->RPMReadFileTags(nxlFilePath, tags);

	debugStream << L"Tags: " << tags.c_str() << std::endl;
	debugStream.str(L"");
	debugStream.flush();

	return wsz2jstr(env, tags);
}
