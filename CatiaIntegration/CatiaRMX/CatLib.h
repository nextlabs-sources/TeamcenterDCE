#pragma once

extern "C" {

#pragma region JS0FM.dll
	//
	// class CATString & CATCommand::GetName(void)
	//
	#define CATCommand_GetName_LIBNAME  L"JS0FM.dll"
	#define CATCommand_GetName_FULLNAME  "?GetName@CATCommand@@UEAAAEAVCATString@@XZ"
	typedef CPP_REF(*pfCATCommand_GetName)(CPP_PTR pThis);
	CPP_PTR CATCommand_GetName(CPP_PTR pThis);

	//
	// class CATCommand * CATSysGetLastActiveCommand(void)
	//
#define CATSysGetLastActiveCommand_LIBNAME  L"JS0FM.dll"
#define CATSysGetLastActiveCommand_FULLNAME  "?CATSysGetLastActiveCommand@@YAPEAVCATCommand@@XZ"
	typedef CPP_REF(*pfCATSysGetLastActiveCommand)();
	CPP_PTR CATSysGetLastActiveCommand();
#pragma endregion JS0FM.dll

#pragma region JS0GROUP.dll
	//
	//char const * CATString::ConvertToChar(void)
	// 
	#define CATString_ConvertToChar_LIBNAME L"JS0GROUP.dll" 
	#define CATString_ConvertToChar_FULLNAME  "?ConvertToChar@CATString@@QEBAPEBDXZ"
	typedef char const *(*pfCATString_ConvertToChar)(CPP_PTR pThis);
	char const * CATString_ConvertToChar(CPP_PTR pThis);

	//
	// char const * CATUnicodeString::ConvertToChar(void)
	//
	#define CATUnicodeString_ConvertToChar_LIBNAME L"JS0GROUP.dll" 
	#define CATUnicodeString_ConvertToChar_FULLNAME  "?ConvertToChar@CATUnicodeString@@QEBAPEBDXZ"
	typedef char const* (*pfCATUnicodeString_ConvertToChar)(CPP_PTR pThis);
	char const* CATUnicodeString_ConvertToChar(CPP_PTR pThis);

	//
	// int CATListValCATUnicodeString::Size(void)
	//
#define CATListValCATUnicodeString_Size_LIBNAME L"JS0GROUP.dll" 
#define CATListValCATUnicodeString_Size_FULLNAME  "?Size@CATListValCATString@@QEBAHXZ"
	typedef int (*pfCATListValCATUnicodeString_Size)(CPP_PTR pThis);
	int CATListValCATUnicodeString_Size(CPP_PTR pThis);

	//
	//class CATUnicodeString & CATListValCATUnicodeString::operator[](int)
	//class CATUnicodeString const& CATListValCATUnicodeString::operator[](int)
#define CATListValCATUnicodeString_Item_LIBNAME L"JS0GROUP.dll" 
#define CATListValCATUnicodeString_Item_FULLNAME  "??ACATListValCATUnicodeString@@QEAAAEAVCATUnicodeString@@H@Z"
	typedef CPP_PTR (*pfCATListValCATUnicodeString_Item)(CPP_PTR, int);
	CPP_PTR CATListValCATUnicodeString_Item(CPP_PTR pThis, int);

	//
	//int CATListValCATUnicodeString::Locate(class CATUnicodeString const &,int)
	//
#define CATListValCATUnicodeString_Locate_LIBNAME L"JS0GROUP.dll" 
#define CATListValCATUnicodeString_Locate_FULLNAME  "?Locate@CATListValCATUnicodeString@@QEBAHAEBVCATUnicodeString@@H@Z"
	typedef int(*pfCATListValCATUnicodeString_Locate)(CPP_PTR, CPP_PTR, int);
	int CATListValCATUnicodeString_Locate(CPP_PTR pThis, CPP_PTR s, int index);
	//
// void CATListValCATUnicodeString::Array(class CATUnicodeString * *)
//
#define CATListValCATUnicodeString_Array_LIBNAME L"CATObjectModelerBase.DLL" 
#define CATListValCATUnicodeString_Array_FULLNAME  "?Array@CATListValCATUnicodeString@@QEBAXPEAPEAVCATUnicodeString@@@Z"
	typedef void(*pfCATListValCATUnicodeString_Array)(CPP_PTR, CPP_PTR *);
	void CATListValCATUnicodeString_Array(CPP_PTR, CPP_PTR*);

#pragma endregion JS0GROUP.dll

#pragma region CATObjectModelerBase.dll
	//
	// void CATDocument::storageName(char *)
	//
	#define CATDocument_storageName_LIBNAME L"CATObjectModelerBase.DLL" 
	#define CATDocument_storageName_FULLNAME  "?storageName@CATDocument@@UEBAPEADXZ"
	typedef char* (*pfCATDocument_storageName)(CPP_PTR);
	char* CATDocument_storageName(CPP_PTR);

	//
	// void CATDocument::SetSaveTime(void)
	// 
	#define CATDocument_SetSaveTime_LIBNAME L"CATObjectModelerBase.DLL" 
	#define CATDocument_SetSaveTime_FULLNAME  "?SetSaveTime@CATDocument@@QEAAXXZ"
	typedef void (*pfCATDocument_SetSaveTime)(CPP_PTR);
	void CATDocument_SetSaveTime(CPP_PTR);

	//
	// class CATDocument* CATStorageManager::GetDocument(void)
	//
	#define CATStorageManager_GetDocument_LIBNAME L"CATObjectModelerBase.DLL" 
	#define CATStorageManager_GetDocument_FULLNAME  "?GetDocument@CATStorageManager@@QEAAPEAVCATDocument@@XZ"
	typedef CPP_PTR(*pfCATStorageManager_GetDocument)(CPP_PTR);
	CPP_PTR CATStorageManager_GetDocument(CPP_PTR);

	//
	// int CATListPtrCATDocument::Size(void)
	//
#define CATListPtrCATDocument_Size_LIBNAME L"CATObjectModelerBase.DLL" 
#define CATListPtrCATDocument_Size_FULLNAME  "?Size@CATListPtrCATDocument@@QEBAHXZ"
	typedef int(*pfCATListPtrCATDocument_Size)(CPP_PTR);
	int CATListPtrCATDocument_Size(CPP_PTR);

	//class CATDocument*& CATListPtrCATDocument::operator[](int)
	//class CATDocument* CATListPtrCATDocument::operator[](int)
#define CATListPtrCATDocument_Item_LIBNAME L"CATObjectModelerBase.DLL" 
#define CATListPtrCATDocument_Item_FULLNAME  "??ACATListPtrCATDocument@@QEBAPEAVCATDocument@@H@Z"
	typedef CPP_PTR(*pfCATListPtrCATDocument_Item)(CPP_PTR, int);
	CPP_PTR CATListPtrCATDocument_Item(CPP_PTR, int);


#pragma endregion CATObjectModelerBase.dll

#pragma region CATApplicationFrame.dll
	//
	// class CATDocument * CATFrmEditor::GetDocument(void)
	//
	#define CATFrmEditor_GetDocument_LIBNAME L"CATApplicationFrame.dll"
	#define CATFrmEditor_GetDocument_FULLNAME  "?GetDocument@CATFrmEditor@@UEAAPEAVCATDocument@@XZ"
	typedef CPP_PTR(*pfCATFrmEditor_GetDocument)(CPP_PTR);
	CPP_PTR CATFrmEditor_GetDocument(CPP_PTR);

	//
	// class CATFrmEditor * CATFrmWindow::GetEditor(void)
	//
#define CATFrmWindow_GetEditor_LIBNAME L"CATApplicationFrame.dll"
#define CATFrmWindow_GetEditor_FULLNAME  "?GetEditor@CATFrmWindow@@UEAAPEAVCATFrmEditor@@XZ"
	typedef CPP_PTR(*pfCATFrmWindow_GetEditor)(CPP_PTR);
	CPP_PTR CATFrmWindow_GetEditor(CPP_PTR);


	//
	// class CATListPtrCATFrmWindow& CATFrmLayout::GetWindowList(int, int)
	//
#define CATFrmLayout_GetWindowList_LIBNAME L"CATApplicationFrame.dll"
#define CATFrmLayout_GetWindowList_FULLNAME  "?GetWindowList@CATFrmLayout@@UEAAAEAVCATListPtrCATFrmWindow@@HH@Z"
	typedef CPP_PTR(*pfCATFrmLayout_GetWindowList)(CPP_PTR, int, int);
	CPP_PTR CATFrmLayout_GetWindowList(CPP_PTR, int, int);

	//
	// class CATFrmWindow * CATFrmLayout::GetCurrentWindow(void)
	// 
#define CATFrmLayout_GetCurrentWindow_LIBNAME L"CATApplicationFrame.dll"
#define CATFrmLayout_GetCurrentWindow_FULLNAME  "?GetCurrentWindow@CATFrmLayout@@UEAAPEAVCATFrmWindow@@XZ"
	typedef CPP_PTR(*pfCATFrmLayout_GetCurrentWindow)(CPP_PTR);
	CPP_PTR CATFrmLayout_GetCurrentWindow(CPP_PTR);

	//
	// int CATListPtrCATFrmWindow::Size(void)
	// 
#define CATListPtrCATFrmWindow_size_LIBNAME L"CATApplicationFrame.dll"
#define CATListPtrCATFrmWindow_size_FULLNAME  "?Size@CATListPtrCATFrmWindow@@QEBAHXZ"
	typedef int(*pfCATListPtrCATFrmWindow_size)(CPP_PTR);
	int CATListPtrCATFrmWindow_size(CPP_PTR);

	//
	// class CATFrmWindow * CATListPtrCATFrmWindow::operator[](int)
	//

#define CATListPtrCATFrmWindow_elem_LIBNAME L"CATApplicationFrame.dll"
#define CATListPtrCATFrmWindow_elem_FULLNAME  "??ACATListPtrCATFrmWindow@@QEBAPEAVCATFrmWindow@@H@Z"
	typedef CPP_PTR(*pfCATListPtrCATFrmWindow_elem)(CPP_PTR, int);
	CPP_PTR CATListPtrCATFrmWindow_elem(CPP_PTR, int);

	//
	// enum CATFrmWindowState CATFrmWindow::GetWindowMDIState(void
	// 

#define CATFrmWindow_GetWindowMDIState_LIBNAME L"CATApplicationFrame.dll"
#define CATFrmWindow_GetWindowMDIState_FULLNAME  "?GetWindowMDIState@CATFrmWindow@@QEAA?AW4CATFrmWindowState@@XZ"
	typedef int(*pfCATFrmWindow_GetWindowMDIState)(CPP_PTR);
	int CATFrmWindow_GetWindowMDIState(CPP_PTR);

	// 
	// char const * CATCommandHeader::GetID(void)
	// 
#define CATCommandHeader_GetID_LIBNAME L"CATApplicationFrame.dll"
#define CATCommandHeader_GetID_FULLNAME  "?GetID@CATCommandHeader@@QEAAPEBDXZ"
	typedef char const* (*pfCATCommandHeader_GetID)(CPP_PTR);
	char const* CATCommandHeader_GetID(CPP_PTR);

	//
	// class CATString & CATCommandHeader::GetName(void)
	// 
#define CATCommandHeader_GetName_LIBNAME L"CATApplicationFrame.dll"
#define CATCommandHeader_GetName_FULLNAME  "?GetName@CATCommandHeader@@UEAAAEAVCATString@@XZ"
	typedef CPP_PTR(*pfCATCommandHeader_GetName)(CPP_PTR);
	CPP_PTR CATCommandHeader_GetName(CPP_PTR);

	//class CATString& CATCommandHeader::GetClass(void)
	// class CATString& CATCommandHeader::GetClsidName(void)
#define CATCommandHeader_GetClass_LIBNAME L"CATApplicationFrame.dll"
#define CATCommandHeader_GetClass_FULLNAME  "?GetClass@CATCommandHeader@@UEAAAEAVCATString@@XZ"
	typedef CPP_PTR(*pfCATCommandHeader_GetClass)(CPP_PTR);
	CPP_PTR CATCommandHeader_GetClass(CPP_PTR);

#define CATCommandHeader_GetClsidName_LIBNAME L"CATApplicationFrame.dll"
#define CATCommandHeader_GetClsidName_FULLNAME  "?GetClsidName@CATCommandHeader@@UEAAAEAVCATString@@XZ"
	typedef CPP_PTR(*pfCATCommandHeader_GetClsidName)(CPP_PTR);
	CPP_PTR CATCommandHeader_GetClsidName(CPP_PTR);
#pragma endregion CATApplicationFrame.dll

}

