/*
	Library:	libpart.dll
*/
#ifndef NXL_LIBPART_DLL_H_INCLUDED
#define NXL_LIBPART_DLL_H_INCLUDED

#include "libsyss.h"

typedef const void *PartFileStream_PTR;	// class UGS::Part::PartFileStream * __ptr64
typedef const void *SaveManager_PTR;	// class UGS::Part::SaveManager * __ptr64

/* typedef for 21 funcs */
/*
	UGS::Part::SaveManager::AddAssociatedFile
	ApiName:	SaveManager_AddAssociatedFile
	FullName:	?AddAssociatedFile@SaveManager@Part@UGS@@QEBAXIAEBVUString@3@_NAEBW4Type@IFileType@Core@Model@DataManagement@3@0@Z
	UnDecorated:	public: void __cdecl UGS::Part::SaveManager::AddAssociatedFile(unsigned int,class UGS::UString const & __ptr64,bool,enum UGS::DataManagement::Model::Core::IFileType::Type const & __ptr64,class UGS::UString const & __ptr64)const __ptr64
	Package = UGS::Part::SaveManager
	Versions(#Ordinal):
		1980.1700.0.0 (#5792)
*/
#define SaveManager_AddAssociatedFile_FULLNAME "?AddAssociatedFile@SaveManager@Part@UGS@@QEBAXIAEBVUString@3@_NAEBW4Type@IFileType@Core@Model@DataManagement@3@0@Z"
typedef void (*pfSaveManager_AddAssociatedFile)(SaveManager_PTR obj	// class UGS::Part::SaveManager * __ptr64
	,unsigned int p1	// unsigned int
	,UString_REF p2	// class UGS::UString const & __ptr64
	,BOOL p3	// bool
	,CPP_ENUM const * p4	// enum UGS::DataManagement::Model::Core::IFileType::Type const & __ptr64
	,UString_REF p5	// class UGS::UString const & __ptr64
	);

/*
	BASE_open_part
	ApiName:	BASE_open_part
	FullName:	?BASE_open_part@@YAHPEBDPEAIPEAUUF_PART_load_status_s@@@Z
	UnDecorated:	int __cdecl BASE_open_part(char const * __ptr64,unsigned int * __ptr64,struct UF_PART_load_status_s * __ptr64)
	Package = BASE
	Versions(#Ordinal):
		1980.1700.0.0 (#6224)
*/
#define BASE_open_part_FULLNAME "?BASE_open_part@@YAHPEBDPEAIPEAUUF_PART_load_status_s@@@Z"
typedef int (*pfBASE_open_part)(char const * p1	// char const * __ptr64
	,unsigned int * p2	// unsigned int * __ptr64
	,CPP_PTR p3	// struct UF_PART_load_status_s * __ptr64
	);

/*
	BASE_part_save_as
	ApiName:	BASE_part_save_as
	FullName:	?BASE_part_save_as@@YAXIPEBDPEAH1_N1@Z
	UnDecorated:	void __cdecl BASE_part_save_as(unsigned int,char const * __ptr64,int * __ptr64,int * __ptr64,bool,int * __ptr64)
	Package = BASE
	Versions(#Ordinal):
		1980.1700.0.0 (#6233)
*/
#define BASE_part_save_as_FULLNAME "?BASE_part_save_as@@YAXIPEBDPEAH1_N1@Z"
typedef void (*pfBASE_part_save_as)(unsigned int p1	// unsigned int
	,char const * p2	// char const * __ptr64
	,int * p3	// int * __ptr64
	,int * p4	// int * __ptr64
	,BOOL p5	// bool
	,int * p6	// int * __ptr64
	);

/*
	UGS::Part::BasicSaveManager::DoNXSave
	ApiName:	BasicSaveManager_DoNXSave
	FullName:	?DoNXSave@BasicSaveManager@Part@UGS@@AEAA_NPEAVSaveInput@23@@Z
	UnDecorated:	private: bool __cdecl UGS::Part::BasicSaveManager::DoNXSave(class UGS::Part::SaveInput * __ptr64) __ptr64
	Package = UGS::Part::BasicSaveManager
	Versions(#Ordinal):
		1980.1700.0.0 (#8022)
*/
#define BasicSaveManager_DoNXSave_FULLNAME "?DoNXSave@BasicSaveManager@Part@UGS@@AEAA_NPEAVSaveInput@23@@Z"
typedef BOOL (*pfBasicSaveManager_DoNXSave)(CPP_PTR obj	// class UGS::Part::BasicSaveManager * __ptr64
	,CPP_PTR p1	// class UGS::Part::SaveInput * __ptr64
	);

/*
	UGS::Part::PartFileStream::GetOSPath
	ApiName:	PartFileStream_GetOSPath
	FullName:	?GetOSPath@PartFileStream@Part@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Part::PartFileStream::GetOSPath(void)const __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#11122)
*/
#define PartFileStream_GetOSPath_FULLNAME "?GetOSPath@PartFileStream@Part@UGS@@QEBAPEBDXZ"
typedef char const * (*pfPartFileStream_GetOSPath)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);
extern char const * PartFileStream_GetOSPath(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);

/*
	UGS::Part::SaveInput::GetPartTag
	ApiName:	SaveInput_GetPartTag
	FullName:	?GetPartTag@SaveInput@Part@UGS@@UEBAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Part::SaveInput::GetPartTag(void)const __ptr64
	Package = UGS::Part::SaveInput
	Versions(#Ordinal):
		1980.1700.0.0 (#11393)
*/
#define SaveInput_GetPartTag_FULLNAME "?GetPartTag@SaveInput@Part@UGS@@UEBAIXZ"
typedef unsigned int (*pfSaveInput_GetPartTag)(CPP_PTR obj	// class UGS::Part::SaveInput * __ptr64
	);
extern unsigned int SaveInput_GetPartTag(CPP_PTR obj	// class UGS::Part::SaveInput * __ptr64
	);

/*
	UGS::Part::PartFileStream::IsForWrite
	ApiName:	PartFileStream_IsForWrite
	FullName:	?IsForWrite@PartFileStream@Part@UGS@@QEBA_NXZ
	UnDecorated:	public: bool __cdecl UGS::Part::PartFileStream::IsForWrite(void)const __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#13254)
*/
#define PartFileStream_IsForWrite_FULLNAME "?IsForWrite@PartFileStream@Part@UGS@@QEBA_NXZ"
typedef BOOL (*pfPartFileStream_IsForWrite)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);
extern BOOL PartFileStream_IsForWrite(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);

/*
	UGS::Part::PartFileStream::IsReadOnly
	ApiName:	PartFileStream_IsReadOnly
	FullName:	?IsReadOnly@PartFileStream@Part@UGS@@QEBA_NXZ
	UnDecorated:	public: bool __cdecl UGS::Part::PartFileStream::IsReadOnly(void)const __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#13501)
*/
#define PartFileStream_IsReadOnly_FULLNAME "?IsReadOnly@PartFileStream@Part@UGS@@QEBA_NXZ"
typedef BOOL (*pfPartFileStream_IsReadOnly)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);
extern BOOL PartFileStream_IsReadOnly(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);

/*
	UGS::Part::PartFileStream::Open
	ApiName:	PartFileStream_Open_0
	FullName:	?Open@PartFileStream@Part@UGS@@AEAAXPEAVPART_part@3@PEBD1PEAVPathCache@23@W4PART_open_mode_t@@_NW4StorageElements@Pfedit@System@3@@Z
	UnDecorated:	private: void __cdecl UGS::Part::PartFileStream::Open(class UGS::PART_part * __ptr64,char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool,enum UGS::System::Pfedit::StorageElements) __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#14203)
*/
#define PartFileStream_Open_0_FULLNAME "?Open@PartFileStream@Part@UGS@@AEAAXPEAVPART_part@3@PEBD1PEAVPathCache@23@W4PART_open_mode_t@@_NW4StorageElements@Pfedit@System@3@@Z"
typedef void (*pfPartFileStream_Open_0)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	,CPP_PTR p1	// class UGS::PART_part * __ptr64
	,char const * p2	// char const * __ptr64
	,char const * p3	// char const * __ptr64
	,CPP_PTR p4	// class UGS::Part::PathCache * __ptr64
	,CPP_ENUM p5	// enum PART_open_mode_t
	,BOOL p6	// bool
	,CPP_ENUM p7	// enum UGS::System::Pfedit::StorageElements
	);

/*
	UGS::Part::PartFileStream::Open
	ApiName:	PartFileStream_Open_1
	FullName:	?Open@PartFileStream@Part@UGS@@QEAAXPEAVPART_part@3@PEBD1PEAVPathCache@23@W4PART_open_mode_t@@_N@Z
	UnDecorated:	public: void __cdecl UGS::Part::PartFileStream::Open(class UGS::PART_part * __ptr64,char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool) __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#14204)
*/
#define PartFileStream_Open_1_FULLNAME "?Open@PartFileStream@Part@UGS@@QEAAXPEAVPART_part@3@PEBD1PEAVPathCache@23@W4PART_open_mode_t@@_N@Z"
typedef void (*pfPartFileStream_Open_1)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	,CPP_PTR p1	// class UGS::PART_part * __ptr64
	,char const * p2	// char const * __ptr64
	,char const * p3	// char const * __ptr64
	,CPP_PTR p4	// class UGS::Part::PathCache * __ptr64
	,CPP_ENUM p5	// enum PART_open_mode_t
	,BOOL p6	// bool
	);

/*
	UGS::Part::PartFileStream::Open
	ApiName:	PartFileStream_Open_2
	FullName:	?Open@PartFileStream@Part@UGS@@QEAAXPEBD0PEAVPathCache@23@W4PART_open_mode_t@@_N@Z
	UnDecorated:	public: void __cdecl UGS::Part::PartFileStream::Open(char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool) __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#14205)
*/
#define PartFileStream_Open_2_FULLNAME "?Open@PartFileStream@Part@UGS@@QEAAXPEBD0PEAVPathCache@23@W4PART_open_mode_t@@_N@Z"
typedef void (*pfPartFileStream_Open_2)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	,char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	,CPP_PTR p3	// class UGS::Part::PathCache * __ptr64
	,CPP_ENUM p4	// enum PART_open_mode_t
	,BOOL p5	// bool
	);

/*
	PART__load_foreign
	ApiName:	PART_load_foreign
	FullName:	?PART__load_foreign@@YA_NPEBDH_NPEAUPROGRESS_step_s@@PEAUUF_PART_load_status_s@@PEAI@Z
	UnDecorated:	bool __cdecl PART__load_foreign(char const * __ptr64,int,bool,struct PROGRESS_step_s * __ptr64,struct UF_PART_load_status_s * __ptr64,unsigned int * __ptr64)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#14309)
*/
#define PART_load_foreign_FULLNAME "?PART__load_foreign@@YA_NPEBDH_NPEAUPROGRESS_step_s@@PEAUUF_PART_load_status_s@@PEAI@Z"
typedef BOOL (*pfPART_load_foreign)(char const * p1	// char const * __ptr64
	,int p2	// int
	,BOOL p3	// bool
	,CPP_PTR p4	// struct PROGRESS_step_s * __ptr64
	,CPP_PTR p5	// struct UF_PART_load_status_s * __ptr64
	,unsigned int * p6	// unsigned int * __ptr64
	);

/*
	PART__save_as_foreign_part
	ApiName:	PART_save_as_foreign_part
	FullName:	?PART__save_as_foreign_part@@YAHIQEBD@Z
	UnDecorated:	int __cdecl PART__save_as_foreign_part(unsigned int,char const * __ptr64 const)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#14330)
*/
#define PART_save_as_foreign_part_FULLNAME "?PART__save_as_foreign_part@@YAHIQEBD@Z"
typedef int (*pfPART_save_as_foreign_part)(unsigned int p1	// unsigned int
	,char const * p2	// char const * __ptr64 const
	);

/*
	PART_copy_part
	ApiName:	PART_copy_part
	FullName:	?PART_copy_part@@YAIPEBD0HPEAUUF_PART_load_status_s@@PEAVSaveFailure@Part@UGS@@@Z
	UnDecorated:	unsigned int __cdecl PART_copy_part(char const * __ptr64,char const * __ptr64,int,struct UF_PART_load_status_s * __ptr64,class UGS::Part::SaveFailure * __ptr64)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#14711)
*/
#define PART_copy_part_FULLNAME "?PART_copy_part@@YAIPEBD0HPEAUUF_PART_load_status_s@@PEAVSaveFailure@Part@UGS@@@Z"
typedef unsigned int (*pfPART_copy_part)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	,int p3	// int
	,CPP_PTR p4	// struct UF_PART_load_status_s * __ptr64
	,CPP_PTR p5	// class UGS::Part::SaveFailure * __ptr64
	);

/*
	PART_export_part
	ApiName:	PART_export_part
	FullName:	?PART_export_part@@YAHPEBDHPEAIPEAUPART_export_options_s@@@Z
	UnDecorated:	int __cdecl PART_export_part(char const * __ptr64,int,unsigned int * __ptr64,struct PART_export_options_s * __ptr64)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#14795)
*/
#define PART_export_part_FULLNAME "?PART_export_part@@YAHPEBDHPEAIPEAUPART_export_options_s@@@Z"
typedef int (*pfPART_export_part)(char const * p1	// char const * __ptr64
	,int p2	// int
	,unsigned int * p3	// unsigned int * __ptr64
	,CPP_PTR p4	// struct PART_export_options_s * __ptr64
	);

/*
	PART_get_os_date
	ApiName:	PART_get_os_date
	FullName:	?PART_get_os_date@@YAXPEBDHPEAH1@Z
	UnDecorated:	void __cdecl PART_get_os_date(char const * __ptr64,int,int * __ptr64,int * __ptr64)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#14871)
*/
#define PART_get_os_date_FULLNAME "?PART_get_os_date@@YAXPEBDHPEAH1@Z"
typedef void (*pfPART_get_os_date)(char const * p1	// char const * __ptr64
	,int p2	// int
	,int * p3	// int * __ptr64
	,int * p4	// int * __ptr64
	);

/*
	PART_open_ug_partfile
	ApiName:	PART_open_ug_partfile_0
	FullName:	?PART_open_ug_partfile@@YAHPEBDW4PART_open_mode_t@@H_NPEBUCFI_file_usage_hints_t@@PEAVPartFileStream@Part@UGS@@@Z
	UnDecorated:	int __cdecl PART_open_ug_partfile(char const * __ptr64,enum PART_open_mode_t,int,bool,struct CFI_file_usage_hints_t const * __ptr64,class UGS::Part::PartFileStream * __ptr64)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#15190)
*/
#define PART_open_ug_partfile_0_FULLNAME "?PART_open_ug_partfile@@YAHPEBDW4PART_open_mode_t@@H_NPEBUCFI_file_usage_hints_t@@PEAVPartFileStream@Part@UGS@@@Z"
typedef int (*pfPART_open_ug_partfile_0)(char const * p1	// char const * __ptr64
	,CPP_ENUM p2	// enum PART_open_mode_t
	,int p3	// int
	,BOOL p4	// bool
	,CPP_PTR p5	// struct CFI_file_usage_hints_t const * __ptr64
	,PartFileStream_PTR p6	// class UGS::Part::PartFileStream * __ptr64
	);

/*
	PART_open_ug_partfile
	ApiName:	PART_open_ug_partfile_1
	FullName:	?PART_open_ug_partfile@@YAHPEBDW4PART_open_mode_t@@H_NPEBUCFI_file_usage_hints_t@@PEAVPartFileStream@Part@UGS@@W4StorageElements@Pfedit@System@5@@Z
	UnDecorated:	int __cdecl PART_open_ug_partfile(char const * __ptr64,enum PART_open_mode_t,int,bool,struct CFI_file_usage_hints_t const * __ptr64,class UGS::Part::PartFileStream * __ptr64,enum UGS::System::Pfedit::StorageElements)
	Package = PART
	Versions(#Ordinal):
		1980.1700.0.0 (#15191)
*/
#define PART_open_ug_partfile_1_FULLNAME "?PART_open_ug_partfile@@YAHPEBDW4PART_open_mode_t@@H_NPEBUCFI_file_usage_hints_t@@PEAVPartFileStream@Part@UGS@@W4StorageElements@Pfedit@System@5@@Z"
typedef int (*pfPART_open_ug_partfile_1)(char const * p1	// char const * __ptr64
	,CPP_ENUM p2	// enum PART_open_mode_t
	,int p3	// int
	,BOOL p4	// bool
	,CPP_PTR p5	// struct CFI_file_usage_hints_t const * __ptr64
	,PartFileStream_PTR p6	// class UGS::Part::PartFileStream * __ptr64
	,CPP_ENUM p7	// enum UGS::System::Pfedit::StorageElements
	);

/*
	UGS::Part::PartFileStream::ReleaseReplaceFile
	ApiName:	PartFileStream_ReleaseReplaceFile_0
	FullName:	?ReleaseReplaceFile@PartFileStream@Part@UGS@@QEAAXHPEBD_N@Z
	UnDecorated:	public: void __cdecl UGS::Part::PartFileStream::ReleaseReplaceFile(int,char const * __ptr64,bool) __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#16479)
*/
#define PartFileStream_ReleaseReplaceFile_0_FULLNAME "?ReleaseReplaceFile@PartFileStream@Part@UGS@@QEAAXHPEBD_N@Z"
typedef void (*pfPartFileStream_ReleaseReplaceFile_0)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	,int p1	// int
	,char const * p2	// char const * __ptr64
	,BOOL p3	// bool
	);

/*
	UGS::Part::PartFileStream::ReleaseReplaceFile
	ApiName:	PartFileStream_ReleaseReplaceFile_1
	FullName:	?ReleaseReplaceFile@PartFileStream@Part@UGS@@QEAAX_N@Z
	UnDecorated:	public: void __cdecl UGS::Part::PartFileStream::ReleaseReplaceFile(bool) __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#16480)
*/
#define PartFileStream_ReleaseReplaceFile_1_FULLNAME "?ReleaseReplaceFile@PartFileStream@Part@UGS@@QEAAX_N@Z"
typedef void (*pfPartFileStream_ReleaseReplaceFile_1)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	,BOOL p1	// bool
	);

/*
	UGS::Part::PartFileStream::SetReadOnly
	ApiName:	PartFileStream_SetReadOnly
	FullName:	?SetReadOnly@PartFileStream@Part@UGS@@QEAAXXZ
	UnDecorated:	public: void __cdecl UGS::Part::PartFileStream::SetReadOnly(void) __ptr64
	Package = UGS::Part::PartFileStream
	Versions(#Ordinal):
		1980.1700.0.0 (#18277)
*/
#define PartFileStream_SetReadOnly_FULLNAME "?SetReadOnly@PartFileStream@Part@UGS@@QEAAXXZ"
typedef void (*pfPartFileStream_SetReadOnly)(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);
extern void PartFileStream_SetReadOnly(PartFileStream_PTR obj	// class UGS::Part::PartFileStream * __ptr64
	);


#endif
