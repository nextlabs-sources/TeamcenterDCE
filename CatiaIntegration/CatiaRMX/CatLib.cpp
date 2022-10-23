#include "stdafx.h"

#include "..\common\HookDefs.h"
#include "CatLib.h"

#define RMX_GET_PROCADRESS(apiName) \
static pf##apiName apiName##_fn = NULL; \
if (apiName##_fn == NULL) { \
	HMODULE hLib = GetModuleHandle(apiName##_LIBNAME); \
	CHK_ERROR_RETURN(hLib == NULL, NULL, L"Failed to get module handle of %s (error: %s)", apiName##_LIBNAME, (LPCWSTR)CSysErrorMessage(GetLastError())); \
	apiName##_fn = (pf##apiName)GetProcAddress(hLib, apiName##_FULLNAME); \
	CHK_ERROR_RETURN(apiName##_fn == NULL, NULL, L"Failed to GetProcAddress %s (error: %s)",_W(_STRINGIZE(apiName)), (LPCWSTR)CSysErrorMessage(GetLastError())); \
}
#define RMX_GET_PROCADRESS_VOID(apiName) \
static pf##apiName apiName##_fn = NULL; \
if (apiName##_fn == NULL) { \
	HMODULE hLib = GetModuleHandle(apiName##_LIBNAME); \
	CHK_ERROR(hLib == NULL, L"Failed to get module handle of %s (error: %s)", apiName##_LIBNAME, (LPCWSTR)CSysErrorMessage(GetLastError())); \
	apiName##_fn = (pf##apiName)GetProcAddress(hLib, apiName##_FULLNAME); \
	CHK_ERROR(apiName##_fn == NULL, L"Failed to GetProcAddress %s (error: %s)",_W(_STRINGIZE(apiName)), (LPCWSTR)CSysErrorMessage(GetLastError())); \
}

#pragma region JS0FM.dll
CPP_PTR CATCommand_GetName(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATCommand_GetName);
	if (CATCommand_GetName_fn != NULL)
		return CATCommand_GetName_fn(pThis);

	return NULL;
}

CPP_PTR CATSysGetLastActiveCommand()
{
	RMX_GET_PROCADRESS(CATSysGetLastActiveCommand);
	if (CATSysGetLastActiveCommand_fn != NULL)
		return CATSysGetLastActiveCommand_fn();

	return NULL;
}
#pragma endregion JS0FM.dll

#pragma region JS0GROUP.dll
char const * CATString_ConvertToChar(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATString_ConvertToChar);
	if (CATString_ConvertToChar_fn != NULL)
		return CATString_ConvertToChar_fn(pThis);

	return NULL;
}

char const* CATUnicodeString_ConvertToChar(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATUnicodeString_ConvertToChar);

	return CATUnicodeString_ConvertToChar_fn(pThis);
}

int CATListValCATUnicodeString_Size(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS_VOID(CATListValCATUnicodeString_Size);
	if (CATListValCATUnicodeString_Size_fn != NULL)
		return CATListValCATUnicodeString_Size_fn(pThis);

	return 0;
}

CPP_PTR CATListValCATUnicodeString_Item(CPP_PTR pThis, int index)
{
	RMX_GET_PROCADRESS(CATListValCATUnicodeString_Item);

	return CATListValCATUnicodeString_Item_fn(pThis, index);
}

int CATListValCATUnicodeString_Locate(CPP_PTR pThis, CPP_PTR s, int index)
{
	RMX_GET_PROCADRESS_VOID(CATListValCATUnicodeString_Locate);
	if (CATListValCATUnicodeString_Locate_fn != NULL)
		return CATListValCATUnicodeString_Locate_fn(pThis, s, index);

	return 0;
}

void CATListValCATUnicodeString_Array(CPP_PTR pThis, CPP_PTR *  pArr)
{
	RMX_GET_PROCADRESS_VOID(CATListValCATUnicodeString_Array);
	if (CATListValCATUnicodeString_Array_fn != NULL)
		CATListValCATUnicodeString_Array_fn(pThis, pArr);
}
#pragma endregion JS0GROUP.dll


#pragma region CATObjectModelerBase.dll
char* CATDocument_storageName(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATDocument_storageName);
	if (CATDocument_storageName_fn != NULL)
		return CATDocument_storageName_fn(pThis);

	return NULL;
}

void CATDocument_SetSaveTime(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS_VOID(CATDocument_SetSaveTime);
	if (CATDocument_SetSaveTime_fn != NULL)
		CATDocument_SetSaveTime_fn(pThis);
}

CPP_PTR CATStorageManager_GetDocument(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATStorageManager_GetDocument);
	if (CATStorageManager_GetDocument_fn != NULL)
		return CATStorageManager_GetDocument_fn(pThis);

	return NULL;
}
int CATListPtrCATDocument_Size(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS_VOID(CATListPtrCATDocument_Size);
	if (CATListPtrCATDocument_Size_fn != NULL)
		return CATListPtrCATDocument_Size_fn(pThis);

	return 0;
}

CPP_PTR CATListPtrCATDocument_Item(CPP_PTR pThis, int index)
{
	RMX_GET_PROCADRESS(CATListPtrCATDocument_Item);
	if (CATListPtrCATDocument_Item_fn != NULL)
		return CATListPtrCATDocument_Item_fn(pThis, index);

	return NULL;
}

#pragma endregion CATObjectModelerBase.dll

#pragma region CATApplicationFrame.dll
CPP_PTR CATFrmEditor_GetDocument(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATFrmEditor_GetDocument);
	if (CATFrmEditor_GetDocument_fn != NULL)
		return CATFrmEditor_GetDocument_fn(pThis);

	return NULL;
}

CPP_PTR CATFrmWindow_GetEditor(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATFrmWindow_GetEditor);
	if (CATFrmWindow_GetEditor_fn != NULL)
		return CATFrmWindow_GetEditor_fn(pThis);

	return NULL;
}

int CATFrmWindow_GetWindowMDIState(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATFrmWindow_GetWindowMDIState);
	if (CATFrmWindow_GetWindowMDIState_fn != NULL)
		return CATFrmWindow_GetWindowMDIState_fn(pThis);

	return -1;
}
/// <summary>
/// Returns the list of windows
/// </summary>
/// <param name="pThis"></param>
/// <param name="iWinType">Specify the type of windows to retrieve. 0: Default to list all. 1: Restored windows. 2: Minimized windows</param>
/// <param name="iSort">Sort the return list. 0: Default value w/o sort. 1: Ascend(Newest in the last). 2: Descend</param>
/// <returns></returns>
CPP_PTR CATFrmLayout_GetWindowList(CPP_PTR pThis, int iWinType, int iSort)
{
	RMX_GET_PROCADRESS(CATFrmLayout_GetWindowList);
	if (CATFrmLayout_GetWindowList_fn != NULL)
		return CATFrmLayout_GetWindowList_fn(pThis, iWinType, iSort);

	return NULL;
}

CPP_PTR CATFrmLayout_GetCurrentWindow(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATFrmLayout_GetCurrentWindow);
	if (CATFrmLayout_GetCurrentWindow_fn != NULL)
		return CATFrmLayout_GetCurrentWindow_fn(pThis);

	return NULL;
}

int CATListPtrCATFrmWindow_size(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATListPtrCATFrmWindow_size);
	if (CATListPtrCATFrmWindow_size_fn != NULL)
		return CATListPtrCATFrmWindow_size_fn(pThis);

	return 0;
}

CPP_PTR CATListPtrCATFrmWindow_elem(CPP_PTR pThis, int index)
{
	RMX_GET_PROCADRESS(CATListPtrCATFrmWindow_elem);
	if (CATListPtrCATFrmWindow_elem_fn != NULL)
		return CATListPtrCATFrmWindow_elem_fn(pThis, index);

	return NULL;
}

char const* CATCommandHeader_GetID(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATCommandHeader_GetID);
	if (CATCommandHeader_GetID_fn != NULL)
		return CATCommandHeader_GetID_fn(pThis);

	return NULL;
}

CPP_PTR CATCommandHeader_GetName(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATCommandHeader_GetName);
	if (CATCommandHeader_GetName_fn != NULL)
		return CATCommandHeader_GetName_fn(pThis);

	return NULL;
}

CPP_PTR CATCommandHeader_GetClass(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATCommandHeader_GetClass);
	if (CATCommandHeader_GetClass_fn != NULL)
		return CATCommandHeader_GetClass_fn(pThis);

	return NULL;
}

CPP_PTR CATCommandHeader_GetClsidName(CPP_PTR pThis)
{
	RMX_GET_PROCADRESS(CATCommandHeader_GetClsidName);
	if (CATCommandHeader_GetClsidName_fn != NULL)
		return CATCommandHeader_GetClsidName_fn(pThis);

	return NULL;
}
#pragma endregion CATApplicationFrame.dll
