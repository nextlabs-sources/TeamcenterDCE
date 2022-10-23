// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Shlwapi.h>

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

#include <SDWL/SDLAPI.h>

#ifdef UNICODE
typedef std::wostringstream tostringstream;
typedef std::wstring tstring;
#else
typedef std::ostringstream tostringstream;
typedef std::string tstring;
#endif


#define ENUM_TO_STRING_CASE(e) case e: return TEXT(#e)
#define FLAG_ENUMS_TO_STRING(value, ss, e) if(value & e){ ss << TEXT(#e) << TEXT(";"); }

inline tstring RPMDirRelations_tostring(unsigned int relations) {
	tostringstream ss;
	FLAG_ENUMS_TO_STRING(relations, ss, RPM_SAFEDIRRELATION_SAFE_DIR);
	FLAG_ENUMS_TO_STRING(relations, ss, RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR);
	FLAG_ENUMS_TO_STRING(relations, ss, RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
	return ss.str();
}
inline tstring FileRight_tostring(SDRmFileRight right) {
	switch (right)
	{
		ENUM_TO_STRING_CASE(RIGHT_VIEW);
		ENUM_TO_STRING_CASE(RIGHT_EDIT);
		ENUM_TO_STRING_CASE(RIGHT_PRINT);
		ENUM_TO_STRING_CASE(RIGHT_CLIPBOARD);
		ENUM_TO_STRING_CASE(RIGHT_SAVEAS);
		ENUM_TO_STRING_CASE(RIGHT_DECRYPT);
		ENUM_TO_STRING_CASE(RIGHT_SCREENCAPTURE);
		ENUM_TO_STRING_CASE(RIGHT_SEND);
		ENUM_TO_STRING_CASE(RIGHT_CLASSIFY);
		ENUM_TO_STRING_CASE(RIGHT_SHARE);
		ENUM_TO_STRING_CASE(RIGHT_DOWNLOAD);
		ENUM_TO_STRING_CASE(RIGHT_WATERMARK);
	default:
		return TEXT("UNKNOWN_RIGHT");
	}
}
inline tstring FileRights_tostring(SDRmFileRight rights) {
	tostringstream ss;
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_VIEW);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_EDIT);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_PRINT);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_CLIPBOARD);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_SAVEAS);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_DECRYPT);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_SCREENCAPTURE);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_SEND);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_CLASSIFY);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_SHARE);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_DOWNLOAD);
	FLAG_ENUMS_TO_STRING(rights, ss, RIGHT_WATERMARK);
	return ss.str();
}

#include <NxlUtils/NxlPath.hpp>