#pragma once
//Header file listing all the supported file types
#include "stdafx.h"
#include "FileUtils.h"
enum class SwSourceFileType {
	SW_SLDPRT=1,
	SW_SLDASM=2,
	SW_SLDDRW=3,
	SW_SLDLFP=4,
	SW_PRTDOT=5,
	SW_ASMDOT=6,
	SW_DRWDOT=7,
	SW_PRT=8,
	SW_ASM=9,	
	SW_DRW=10,
	SW_SLDALPRT=11,
	SW_SLDALASM=12
};


enum class SwFileHandlerMethod {
	USE_NORMAL = 1,
	USE_CREATE_HOOK = 2,
	USE_COPY_HOOK = 3,
	USE_PROCESS_HOOK = 4
};

using exportFileHandlerMap = map<wstring, SwFileHandlerMethod> ;

class SwFileTypes {

private:
	static const exportFileHandlerMap exportFileSLDPRT;
	static const exportFileHandlerMap exportFileSLDASM;
	static const exportFileHandlerMap exportFileSLDDRW;

public:
	bool static IsSupportedFileFormat(wstring fromFileType, wstring toFileType, SwFileHandlerMethod &handlerType);
	bool static isNativeFileFormat(wstring fileType);
	bool static IsAutoRecoverFile(const wstring &fileName) {
		return !_wcsicmp(FileUtils::GetFileExt(fileName).c_str(), L".SWAR") ? true : false;
	}
	bool static IsJTFile(const wstring &fileName) {
		return !_wcsicmp(FileUtils::GetFileExt(fileName).c_str(), L".jt") ? true : false;
	}

};