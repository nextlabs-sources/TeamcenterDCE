/*
	Library:	libsyss.dll
*/
#ifndef NXL_LIBSYSS_DLL_H_INCLUDED
#define NXL_LIBSYSS_DLL_H_INCLUDED

typedef const void *UString_PTR;	// class UGS::UString * __ptr64
typedef const void *UString_REF;	// class UGS::UString & __ptr64
typedef const void *UException_PTR;	// class UGS::Error::Exception * __ptr64
typedef const void *UException_REF;	// class UGS::Error::Exception & __ptr64
typedef const void *UTEXT_PTR;	// struct TEXT_s * __ptr64

/* typedef for 40 funcs */
/*
	UGS::System::CFI::ZipFile::AddEntryFile
	ApiName:	ZipFile_AddEntryFile
	FullName:	?AddEntryFile@ZipFile@CFI@System@UGS@@QEAAXAEBVUString@4@0@Z
	UnDecorated:	public: void __cdecl UGS::System::CFI::ZipFile::AddEntryFile(class UGS::UString const & __ptr64,class UGS::UString const & __ptr64) __ptr64
	Package = UGS::System::CFI::ZipFile
	Versions(#Ordinal):
		2007.1700.0.0 (#3024)
*/
#define ZipFile_AddEntryFile_FULLNAME "?AddEntryFile@ZipFile@CFI@System@UGS@@QEAAXAEBVUString@4@0@Z"
typedef void (*pfZipFile_AddEntryFile)(CPP_PTR obj	// class UGS::System::CFI::ZipFile * __ptr64
	,UString_REF p1	// class UGS::UString const & __ptr64
	,UString_REF p2	// class UGS::UString const & __ptr64
	);

/*
	CALLBACK_remove_function
	ApiName:	CALLBACK_remove_function
	FullName:	?CALLBACK_remove_function@@YAXPEAUCALLBACK_registered_fn_s@@@Z
	UnDecorated:	void __cdecl CALLBACK_remove_function(struct CALLBACK_registered_fn_s * __ptr64)
	Package = CALLBACK
	Versions(#Ordinal):
		2007.1700.0.0 (#3237)
*/
#define CALLBACK_remove_function_FULLNAME "?CALLBACK_remove_function@@YAXPEAUCALLBACK_registered_fn_s@@@Z"
typedef void (*pfCALLBACK_remove_function)(CPP_PTR p1	// struct CALLBACK_registered_fn_s * __ptr64
	);
extern void CALLBACK_remove_function(CPP_PTR p1	// struct CALLBACK_registered_fn_s * __ptr64
	);

/*
	CFI_add_argument
	ApiName:	CFI_add_argument
	FullName:	?CFI_add_argument@@YAXPEBD@Z
	UnDecorated:	void __cdecl CFI_add_argument(char const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3241)
*/
#define CFI_add_argument_FULLNAME "?CFI_add_argument@@YAXPEBD@Z"
typedef void (*pfCFI_add_argument)(char const * p1	// char const * __ptr64
	);

/*
	CFI_add_filename_argument
	ApiName:	CFI_add_filename_argument
	FullName:	?CFI_add_filename_argument@@YAXPEBD@Z
	UnDecorated:	void __cdecl CFI_add_filename_argument(char const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3244)
*/
#define CFI_add_filename_argument_FULLNAME "?CFI_add_filename_argument@@YAXPEBD@Z"
typedef void (*pfCFI_add_filename_argument)(char const * p1	// char const * __ptr64
	);

/*
	CFI_add_filename_switch_argument
	ApiName:	CFI_add_filename_switch_argument
	FullName:	?CFI_add_filename_switch_argument@@YAXPEBD0@Z
	UnDecorated:	void __cdecl CFI_add_filename_switch_argument(char const * __ptr64,char const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3245)
*/
#define CFI_add_filename_switch_argument_FULLNAME "?CFI_add_filename_switch_argument@@YAXPEBD0@Z"
typedef void (*pfCFI_add_filename_switch_argument)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	);

/*
	CFI_add_switch_argument
	ApiName:	CFI_add_switch_argument
	FullName:	?CFI_add_switch_argument@@YAXPEBD0@Z
	UnDecorated:	void __cdecl CFI_add_switch_argument(char const * __ptr64,char const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3247)
*/
#define CFI_add_switch_argument_FULLNAME "?CFI_add_switch_argument@@YAXPEBD0@Z"
typedef void (*pfCFI_add_switch_argument)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	);

/*
	CFI_copy_file
	ApiName:	CFI_copy_file_0
	FullName:	?CFI_copy_file@@YAHPEBD0HHHHPEAH@Z
	UnDecorated:	int __cdecl CFI_copy_file(char const * __ptr64,char const * __ptr64,int,int,int,int,int * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3291)
*/
#define CFI_copy_file_0_FULLNAME "?CFI_copy_file@@YAHPEBD0HHHHPEAH@Z"
typedef int (*pfCFI_copy_file_0)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	,int p3	// int
	,int p4	// int
	,int p5	// int
	,int p6	// int
	,int * p7	// int * __ptr64
	);

/*
	CFI_copy_file
	ApiName:	CFI_copy_file_1
	FullName:	?CFI_copy_file@@YAHPEBD0HHHPEAH@Z
	UnDecorated:	int __cdecl CFI_copy_file(char const * __ptr64,char const * __ptr64,int,int,int,int * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3292)
*/
#define CFI_copy_file_1_FULLNAME "?CFI_copy_file@@YAHPEBD0HHHPEAH@Z"
typedef int (*pfCFI_copy_file_1)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	,int p3	// int
	,int p4	// int
	,int p5	// int
	,int * p6	// int * __ptr64
	);

/*
	CFI_is_storage_file
	ApiName:	CFI_is_storage_file
	FullName:	?CFI_is_storage_file@@YA_NPEBD@Z
	UnDecorated:	bool __cdecl CFI_is_storage_file(char const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3362)
*/
#define CFI_is_storage_file_FULLNAME "?CFI_is_storage_file@@YA_NPEBD@Z"
typedef BOOL (*pfCFI_is_storage_file)(char const * p1	// char const * __ptr64
	);

/*
	CFI_open_storage_file
	ApiName:	CFI_open_storage_file_0
	FullName:	?CFI_open_storage_file@@YAPEAVStorageNode@CFI@System@UGS@@PEBD0@Z
	UnDecorated:	class UGS::System::CFI::StorageNode * __ptr64 __cdecl CFI_open_storage_file(char const * __ptr64,char const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3380)
*/
#define CFI_open_storage_file_0_FULLNAME "?CFI_open_storage_file@@YAPEAVStorageNode@CFI@System@UGS@@PEBD0@Z"
typedef CPP_PTR (*pfCFI_open_storage_file_0)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	);

/*
	CFI_open_storage_file
	ApiName:	CFI_open_storage_file_1
	FullName:	?CFI_open_storage_file@@YAPEAVStorageNode@CFI@System@UGS@@PEBD0PEBUCFI_file_usage_hints_t@@@Z
	UnDecorated:	class UGS::System::CFI::StorageNode * __ptr64 __cdecl CFI_open_storage_file(char const * __ptr64,char const * __ptr64,struct CFI_file_usage_hints_t const * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3381)
*/
#define CFI_open_storage_file_1_FULLNAME "?CFI_open_storage_file@@YAPEAVStorageNode@CFI@System@UGS@@PEBD0PEBUCFI_file_usage_hints_t@@@Z"
typedef CPP_PTR (*pfCFI_open_storage_file_1)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	,CPP_PTR p3	// struct CFI_file_usage_hints_t const * __ptr64
	);

/*
	CFI_open_storage_file
	ApiName:	CFI_open_storage_file_2
	FullName:	?CFI_open_storage_file@@YAPEAVStorageNode@CFI@System@UGS@@PEBD0PEBUCFI_file_usage_hints_t@@H@Z
	UnDecorated:	class UGS::System::CFI::StorageNode * __ptr64 __cdecl CFI_open_storage_file(char const * __ptr64,char const * __ptr64,struct CFI_file_usage_hints_t const * __ptr64,int)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3382)
*/
#define CFI_open_storage_file_2_FULLNAME "?CFI_open_storage_file@@YAPEAVStorageNode@CFI@System@UGS@@PEBD0PEBUCFI_file_usage_hints_t@@H@Z"
typedef CPP_PTR (*pfCFI_open_storage_file_2)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	,CPP_PTR p3	// struct CFI_file_usage_hints_t const * __ptr64
	,int p4	// int
	);

/*
	CFI_spawn
	ApiName:	CFI_spawn
	FullName:	?CFI_spawn@@YAHPEBDPEAUCFI_spawn_options_s@@PEAH2@Z
	UnDecorated:	int __cdecl CFI_spawn(char const * __ptr64,struct CFI_spawn_options_s * __ptr64,int * __ptr64,int * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3426)
*/
#define CFI_spawn_FULLNAME "?CFI_spawn@@YAHPEBDPEAUCFI_spawn_options_s@@PEAH2@Z"
typedef int (*pfCFI_spawn)(char const * p1	// char const * __ptr64
	,CPP_PTR p2	// struct CFI_spawn_options_s * __ptr64
	,int * p3	// int * __ptr64
	,int * p4	// int * __ptr64
	);

/*
	CFI_udsetspec
	ApiName:	CFI_udsetspec
	FullName:	?CFI_udsetspec@@YAHPEBDIIPEAPEAD1@Z
	UnDecorated:	int __cdecl CFI_udsetspec(char const * __ptr64,unsigned int,unsigned int,char * __ptr64 * __ptr64,char * __ptr64 * __ptr64)
	Package = CFI
	Versions(#Ordinal):
		2007.1700.0.0 (#3432)
*/
#define CFI_udsetspec_FULLNAME "?CFI_udsetspec@@YAHPEBDIIPEAPEAD1@Z"
typedef int (*pfCFI_udsetspec)(char const * p1	// char const * __ptr64
	,unsigned int p2	// unsigned int
	,unsigned int p3	// unsigned int
	,char ** p4	// char * __ptr64 * __ptr64
	,char ** p5	// char * __ptr64 * __ptr64
	);

/*
	UGS::Error::Exception::CanAcknowledge
	ApiName:	Exception_CanAcknowledge
	FullName:	?CanAcknowledge@Exception@Error@UGS@@UEBA_NXZ
	UnDecorated:	public: virtual bool __cdecl UGS::Error::Exception::CanAcknowledge(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#3569)
*/
#define Exception_CanAcknowledge_FULLNAME "?CanAcknowledge@Exception@Error@UGS@@UEBA_NXZ"
typedef BOOL (*pfException_CanAcknowledge)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern BOOL Exception_CanAcknowledge(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	ERROR_ask_fail_message
	ApiName:	ERROR_ask_fail_message
	FullName:	?ERROR_ask_fail_message@@YAPEBDH@Z
	UnDecorated:	char const * __ptr64 __cdecl ERROR_ask_fail_message(int)
	Package = ERROR
	Versions(#Ordinal):
		2007.1700.0.0 (#4165)
*/
#define ERROR_ask_fail_message_FULLNAME "?ERROR_ask_fail_message@@YAPEBDH@Z"
typedef char const * (*pfERROR_ask_fail_message)(int p1	// int
	);
extern char const * ERROR_ask_fail_message(int p1	// int
	);

/*
	OPL_load_occurrence_data
	ApiName:	OPL_load_occurrence_data
	FullName:	?OPL_load_occurrence_data@@YAHPEBDPEAPEAVOCC_part@UGS@@PEAW4PART_units@@@Z
	UnDecorated:	int __cdecl OPL_load_occurrence_data(char const * __ptr64,class UGS::OCC_part * __ptr64 * __ptr64,enum PART_units * __ptr64)
	Package = OPL
	Versions(#Ordinal):
		2007.1700.0.0 (#7279)
*/
#define OPL_load_occurrence_data_FULLNAME "?OPL_load_occurrence_data@@YAHPEBDPEAPEAVOCC_part@UGS@@PEAW4PART_units@@@Z"
typedef int (*pfOPL_load_occurrence_data)(char const * p1	// char const * __ptr64
	,CPP_PTR p2	// class UGS::OCC_part * __ptr64 * __ptr64
	,CPP_ENUM * p3	// enum PART_units * __ptr64
	);

/*
	PFEDIT_edit_part_file
	ApiName:	PFEDIT_edit_part_file
	FullName:	?PFEDIT_edit_part_file@@YAHPEAUPFEDIT_options_s@@PEBD1@Z
	UnDecorated:	int __cdecl PFEDIT_edit_part_file(struct PFEDIT_options_s * __ptr64,char const * __ptr64,char const * __ptr64)
	Package = PFEDIT
	Versions(#Ordinal):
		2007.1700.0.0 (#7443)
*/
#define PFEDIT_edit_part_file_FULLNAME "?PFEDIT_edit_part_file@@YAHPEAUPFEDIT_options_s@@PEBD1@Z"
typedef int (*pfPFEDIT_edit_part_file)(CPP_PTR p1	// struct PFEDIT_options_s * __ptr64
	,char const * p2	// char const * __ptr64
	,char const * p3	// char const * __ptr64
	);

/*
	PFEDIT_open_part
	FullName:	?PFEDIT_open_part@@YAPEAVPFEDIT_stream@Pfedit@System@UGS@@PEBDUPFEDIT_part_open_options_s@@@Z
	UnDecorated:	class UGS::System::Pfedit::PFEDIT_stream * __ptr64 __cdecl PFEDIT_open_part(char const * __ptr64,struct PFEDIT_part_open_options_s)
	Package = PFEDIT
	Versions(#Ordinal):
		2007.1700.0.0 (#7454)
	Comments:
		Arg-1: Type('PFEDIT_part_open_options_s') is C++ Struct instead of pointer
*/

/*
	PFEDIT_open_storage_file
	ApiName:	PFEDIT_open_storage_file_0
	FullName:	?PFEDIT_open_storage_file@@YAPEAUPFEDIT_storage_s@@PEBDHH@Z
	UnDecorated:	struct PFEDIT_storage_s * __ptr64 __cdecl PFEDIT_open_storage_file(char const * __ptr64,int,int)
	Package = PFEDIT
	Versions(#Ordinal):
		2007.1700.0.0 (#7455)
*/
#define PFEDIT_open_storage_file_0_FULLNAME "?PFEDIT_open_storage_file@@YAPEAUPFEDIT_storage_s@@PEBDHH@Z"
typedef CPP_PTR (*pfPFEDIT_open_storage_file_0)(char const * p1	// char const * __ptr64
	,int p2	// int
	,int p3	// int
	);

/*
	PFEDIT_open_storage_file
	ApiName:	PFEDIT_open_storage_file_1
	FullName:	?PFEDIT_open_storage_file@@YAPEAUPFEDIT_storage_s@@PEBDHHPEBUCFI_file_usage_hints_t@@@Z
	UnDecorated:	struct PFEDIT_storage_s * __ptr64 __cdecl PFEDIT_open_storage_file(char const * __ptr64,int,int,struct CFI_file_usage_hints_t const * __ptr64)
	Package = PFEDIT
	Versions(#Ordinal):
		2007.1700.0.0 (#7456)
*/
#define PFEDIT_open_storage_file_1_FULLNAME "?PFEDIT_open_storage_file@@YAPEAUPFEDIT_storage_s@@PEBDHHPEBUCFI_file_usage_hints_t@@@Z"
typedef CPP_PTR (*pfPFEDIT_open_storage_file_1)(char const * p1	// char const * __ptr64
	,int p2	// int
	,int p3	// int
	,CPP_PTR p4	// struct CFI_file_usage_hints_t const * __ptr64
	);

/*
	PFEDIT_open_storage_file
	ApiName:	PFEDIT_open_storage_file_2
	FullName:	?PFEDIT_open_storage_file@@YAPEAUPFEDIT_storage_s@@PEBDHHPEBUCFI_file_usage_hints_t@@W4StorageElements@Pfedit@System@UGS@@@Z
	UnDecorated:	struct PFEDIT_storage_s * __ptr64 __cdecl PFEDIT_open_storage_file(char const * __ptr64,int,int,struct CFI_file_usage_hints_t const * __ptr64,enum UGS::System::Pfedit::StorageElements)
	Package = PFEDIT
	Versions(#Ordinal):
		2007.1700.0.0 (#7457)
*/
#define PFEDIT_open_storage_file_2_FULLNAME "?PFEDIT_open_storage_file@@YAPEAUPFEDIT_storage_s@@PEBDHHPEBUCFI_file_usage_hints_t@@W4StorageElements@Pfedit@System@UGS@@@Z"
typedef CPP_PTR (*pfPFEDIT_open_storage_file_2)(char const * p1	// char const * __ptr64
	,int p2	// int
	,int p3	// int
	,CPP_PTR p4	// struct CFI_file_usage_hints_t const * __ptr64
	,CPP_ENUM p5	// enum UGS::System::Pfedit::StorageElements
	);

/*
	TEXT_create_string
	ApiName:	TEXT_create_string
	FullName:	?TEXT_create_string@@YAPEAUTEXT_s@@PEBD@Z
	UnDecorated:	struct TEXT_s * __ptr64 __cdecl TEXT_create_string(char const * __ptr64)
	Package = TEXT
	Versions(#Ordinal):
		2007.1700.0.0 (#9213)
*/
#define TEXT_create_string_FULLNAME "?TEXT_create_string@@YAPEAUTEXT_s@@PEBD@Z"
typedef UTEXT_PTR (*pfTEXT_create_string)(char const * p1	// char const * __ptr64
	);
extern UTEXT_PTR TEXT_create_string(char const * p1	// char const * __ptr64
	);

/*
	TEXT_free
	ApiName:	TEXT_free
	FullName:	?TEXT_free@@YAXPEAUTEXT_s@@@Z
	UnDecorated:	void __cdecl TEXT_free(struct TEXT_s * __ptr64)
	Package = TEXT
	Versions(#Ordinal):
		2007.1700.0.0 (#9237)
*/
#define TEXT_free_FULLNAME "?TEXT_free@@YAXPEAUTEXT_s@@@Z"
typedef void (*pfTEXT_free)(UTEXT_PTR p1	// struct TEXT_s * __ptr64
	);
extern void TEXT_free(UTEXT_PTR p1	// struct TEXT_s * __ptr64
	);

/*
	TEXT_strcpy
	ApiName:	TEXT_strcpy
	FullName:	?TEXT_strcpy@@YAXPEADPEBUTEXT_s@@@Z
	UnDecorated:	void __cdecl TEXT_strcpy(char * __ptr64,struct TEXT_s const * __ptr64)
	Package = TEXT
	Versions(#Ordinal):
		2007.1700.0.0 (#9287)
*/
#define TEXT_strcpy_FULLNAME "?TEXT_strcpy@@YAXPEADPEBUTEXT_s@@@Z"
typedef void (*pfTEXT_strcpy)(char * p1	// char * __ptr64
	,UTEXT_PTR p2	// struct TEXT_s const * __ptr64
	);
extern void TEXT_strcpy(char * p1	// char * __ptr64
	,UTEXT_PTR p2	// struct TEXT_s const * __ptr64
	);

/*
	TEXT_strlen
	ApiName:	TEXT_strlen
	FullName:	?TEXT_strlen@@YAHPEBUTEXT_s@@@Z
	UnDecorated:	int __cdecl TEXT_strlen(struct TEXT_s const * __ptr64)
	Package = TEXT
	Versions(#Ordinal):
		2007.1700.0.0 (#9290)
*/
#define TEXT_strlen_FULLNAME "?TEXT_strlen@@YAHPEBUTEXT_s@@@Z"
typedef int (*pfTEXT_strlen)(UTEXT_PTR p1	// struct TEXT_s const * __ptr64
	);
extern int TEXT_strlen(UTEXT_PTR p1	// struct TEXT_s const * __ptr64
	);

/*
	UGS::Error::Exception::acknowledge
	ApiName:	Exception_acknowledge
	FullName:	?acknowledge@Exception@Error@UGS@@UEBAXXZ
	UnDecorated:	public: virtual void __cdecl UGS::Error::Exception::acknowledge(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10119)
*/
#define Exception_acknowledge_FULLNAME "?acknowledge@Exception@Error@UGS@@UEBAXXZ"
typedef void (*pfException_acknowledge)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern void Exception_acknowledge(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::UString::append
	ApiName:	UString_append_0
	FullName:	?append@UString@UGS@@QEAAAEAV12@AEBV12@@Z
	UnDecorated:	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(class UGS::UString const & __ptr64) __ptr64
	Package = UGS::UString
	Versions(#Ordinal):
		2007.1700.0.0 (#10136)
*/
#define UString_append_0_FULLNAME "?append@UString@UGS@@QEAAAEAV12@AEBV12@@Z"
typedef UString_REF (*pfUString_append_0)(UString_PTR obj	// class UGS::UString * __ptr64
	,UString_REF p1	// class UGS::UString const & __ptr64
	);
extern UString_REF UString_append_0(UString_PTR obj	// class UGS::UString * __ptr64
	,UString_REF p1	// class UGS::UString const & __ptr64
	);

/*
	UGS::UString::append
	ApiName:	UString_append_1
	FullName:	?append@UString@UGS@@QEAAAEAV12@D@Z
	UnDecorated:	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(char) __ptr64
	Package = UGS::UString
	Versions(#Ordinal):
		2007.1700.0.0 (#10137)
*/
#define UString_append_1_FULLNAME "?append@UString@UGS@@QEAAAEAV12@D@Z"
typedef UString_REF (*pfUString_append_1)(UString_PTR obj	// class UGS::UString * __ptr64
	,char p1	// char
	);
extern UString_REF UString_append_1(UString_PTR obj	// class UGS::UString * __ptr64
	,char p1	// char
	);

/*
	UGS::UString::append
	ApiName:	UString_append_2
	FullName:	?append@UString@UGS@@QEAAAEAV12@G@Z
	UnDecorated:	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(unsigned short) __ptr64
	Package = UGS::UString
	Versions(#Ordinal):
		2007.1700.0.0 (#10138)
*/
#define UString_append_2_FULLNAME "?append@UString@UGS@@QEAAAEAV12@G@Z"
typedef UString_REF (*pfUString_append_2)(UString_PTR obj	// class UGS::UString * __ptr64
	,unsigned short p1	// unsigned short
	);
extern UString_REF UString_append_2(UString_PTR obj	// class UGS::UString * __ptr64
	,unsigned short p1	// unsigned short
	);

/*
	UGS::UString::append
	ApiName:	UString_append_3
	FullName:	?append@UString@UGS@@QEAAAEAV12@PEBD@Z
	UnDecorated:	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(char const * __ptr64) __ptr64
	Package = UGS::UString
	Versions(#Ordinal):
		2007.1700.0.0 (#10139)
*/
#define UString_append_3_FULLNAME "?append@UString@UGS@@QEAAAEAV12@PEBD@Z"
typedef UString_REF (*pfUString_append_3)(UString_PTR obj	// class UGS::UString * __ptr64
	,char const * p1	// char const * __ptr64
	);
extern UString_REF UString_append_3(UString_PTR obj	// class UGS::UString * __ptr64
	,char const * p1	// char const * __ptr64
	);

/*
	UGS::UString::append
	ApiName:	UString_append_4
	FullName:	?append@UString@UGS@@QEAAAEAV12@PEBUTEXT_s@@@Z
	UnDecorated:	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(struct TEXT_s const * __ptr64) __ptr64
	Package = UGS::UString
	Versions(#Ordinal):
		2007.1700.0.0 (#10140)
*/
#define UString_append_4_FULLNAME "?append@UString@UGS@@QEAAAEAV12@PEBUTEXT_s@@@Z"
typedef UString_REF (*pfUString_append_4)(UString_PTR obj	// class UGS::UString * __ptr64
	,UTEXT_PTR p1	// struct TEXT_s const * __ptr64
	);
extern UString_REF UString_append_4(UString_PTR obj	// class UGS::UString * __ptr64
	,UTEXT_PTR p1	// struct TEXT_s const * __ptr64
	);

/*
	UGS::Error::Exception::askCode
	ApiName:	Exception_askCode
	FullName:	?askCode@Exception@Error@UGS@@QEBAHXZ
	UnDecorated:	public: int __cdecl UGS::Error::Exception::askCode(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10184)
*/
#define Exception_askCode_FULLNAME "?askCode@Exception@Error@UGS@@QEBAHXZ"
typedef int (*pfException_askCode)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern int Exception_askCode(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::Error::Exception::askFileName
	ApiName:	Exception_askFileName
	FullName:	?askFileName@Exception@Error@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Error::Exception::askFileName(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10189)
*/
#define Exception_askFileName_FULLNAME "?askFileName@Exception@Error@UGS@@QEBAPEBDXZ"
typedef char const * (*pfException_askFileName)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern char const * Exception_askFileName(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::Error::Exception::askLastException
	ApiName:	Exception_askLastException
	FullName:	?askLastException@Exception@Error@UGS@@SAPEBV123@XZ
	UnDecorated:	public: static class UGS::Error::Exception const * __ptr64 __cdecl UGS::Error::Exception::askLastException(void)
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10190)
*/
#define Exception_askLastException_FULLNAME "?askLastException@Exception@Error@UGS@@SAPEBV123@XZ"
typedef UException_PTR (*pfException_askLastException)();
extern UException_PTR Exception_askLastException();

/*
	UGS::Error::Exception::askLineNumber
	ApiName:	Exception_askLineNumber
	FullName:	?askLineNumber@Exception@Error@UGS@@QEBAHXZ
	UnDecorated:	public: int __cdecl UGS::Error::Exception::askLineNumber(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10191)
*/
#define Exception_askLineNumber_FULLNAME "?askLineNumber@Exception@Error@UGS@@QEBAHXZ"
typedef int (*pfException_askLineNumber)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern int Exception_askLineNumber(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::Error::Exception::askSyslogMessage
	ApiName:	Exception_askSyslogMessage
	FullName:	?askSyslogMessage@Exception@Error@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Error::Exception::askSyslogMessage(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10192)
*/
#define Exception_askSyslogMessage_FULLNAME "?askSyslogMessage@Exception@Error@UGS@@QEBAPEBDXZ"
typedef char const * (*pfException_askSyslogMessage)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern char const * Exception_askSyslogMessage(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::Error::Exception::askType
	ApiName:	Exception_askType
	FullName:	?askType@Exception@Error@UGS@@QEBA?AW4ERROR_exception_type_t@@XZ
	UnDecorated:	public: enum ERROR_exception_type_t __cdecl UGS::Error::Exception::askType(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10193)
*/
#define Exception_askType_FULLNAME "?askType@Exception@Error@UGS@@QEBA?AW4ERROR_exception_type_t@@XZ"
typedef CPP_ENUM (*pfException_askType)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern CPP_ENUM Exception_askType(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::Error::Exception::askUserMessage
	ApiName:	Exception_askUserMessage
	FullName:	?askUserMessage@Exception@Error@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Error::Exception::askUserMessage(void)const __ptr64
	Package = UGS::Error::Exception
	Versions(#Ordinal):
		2007.1700.0.0 (#10194)
*/
#define Exception_askUserMessage_FULLNAME "?askUserMessage@Exception@Error@UGS@@QEBAPEBDXZ"
typedef char const * (*pfException_askUserMessage)(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);
extern char const * Exception_askUserMessage(UException_PTR obj	// class UGS::Error::Exception * __ptr64
	);

/*
	UGS::UString::utf8_str
	ApiName:	UString_utf8_str
	FullName:	?utf8_str@UString@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::UString::utf8_str(void)const __ptr64
	Package = UGS::UString
	Versions(#Ordinal):
		2007.1700.0.0 (#12296)
*/
#define UString_utf8_str_FULLNAME "?utf8_str@UString@UGS@@QEBAPEBDXZ"
typedef char const * (*pfUString_utf8_str)(UString_PTR obj	// class UGS::UString * __ptr64
	);
extern char const * UString_utf8_str(UString_PTR obj	// class UGS::UString * __ptr64
	);


#endif
