// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

// Import Catia Automation tlb
#import "InfTypeLib.tlb" rename_namespace("CAT") raw_interfaces_only, no_auto_exclude, auto_rename, exclude("CopyFile") exclude("CreateFile") exclude("DeleteFile")
#import "KweTypeLib.tlb" rename_namespace("CAT")
#import "PSTypeLib.tlb" rename_namespace("CAT")
#import "MecModTypeLib.tlb" rename_namespace("CAT")
#import "PartTypeLib.tlb" rename_namespace("CAT") raw_interfaces_only, no_auto_exclude, auto_rename
#import "CATSmarTeamIntegTypeLib.tlb" rename_namespace("CAT") raw_interfaces_only, no_auto_exclude, auto_rename

#include <string>
#include <map>
#include <set>
using namespace std;

// Common RMX headers
#include "..\common\RMXLogInc.h"
#include <RMXUtils.h>
#include <PathEx.h>
#include "..\common\CommonTypes.h"
#include <SysErrorMessage.h>
#include <StringHlp.h>

// Catia RMX headers
#include "resource.h"
#include "CatRMXDefs.h"

using namespace RMXCore;