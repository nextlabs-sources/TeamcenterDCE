/*
	Library:	libpartutils.dll
*/
#ifndef NXL_LIBPARTUTILS_DLL_H_INCLUDED
#define NXL_LIBPARTUTILS_DLL_H_INCLUDED

#include "libsyss.h"

/* typedef for 5 funcs */
/*
	UGS::Part::ExportUtils::ExportSaveAndUnloadPart
	ApiName:	ExportUtils_ExportSaveAndUnloadPart
	FullName:	?ExportSaveAndUnloadPart@ExportUtils@Part@UGS@@YAHPEAI_N@Z
	UnDecorated:	int __cdecl UGS::Part::ExportUtils::ExportSaveAndUnloadPart(unsigned int * __ptr64,bool)
	Package = UGS::Part::ExportUtils
	Versions(#Ordinal):
		2007.1700.0.0 (#2692)
*/
#define ExportUtils_ExportSaveAndUnloadPart_FULLNAME "?ExportSaveAndUnloadPart@ExportUtils@Part@UGS@@YAHPEAI_N@Z"
typedef int (*pfExportUtils_ExportSaveAndUnloadPart)(CPP_PTR obj	// class UGS::Part::ExportUtils * __ptr64
	,unsigned int * p1	// unsigned int * __ptr64
	,BOOL p2	// bool
	);

/*
	PVTRANS_UG_translate_assembly
	ApiName:	UG_translate_assembly
	FullName:	?PVTRANS_UG_translate_assembly@@YAHIPEBD00PEAXPEAUPVTRANS_export_nonJT_options_s@@@Z
	UnDecorated:	int __cdecl PVTRANS_UG_translate_assembly(unsigned int,char const * __ptr64,char const * __ptr64,char const * __ptr64,void * __ptr64,struct PVTRANS_export_nonJT_options_s * __ptr64)
	Package = PVTRANS_UG
	Versions(#Ordinal):
		2007.1700.0.0 (#5245)
*/
#define UG_translate_assembly_FULLNAME "?PVTRANS_UG_translate_assembly@@YAHIPEBD00PEAXPEAUPVTRANS_export_nonJT_options_s@@@Z"
typedef int (*pfUG_translate_assembly)(unsigned int p1	// unsigned int
	,char const * p2	// char const * __ptr64
	,char const * p3	// char const * __ptr64
	,char const * p4	// char const * __ptr64
	,void * p5	// void * __ptr64
	,CPP_PTR p6	// struct PVTRANS_export_nonJT_options_s * __ptr64
	);

/*
	PVTRANS_UG_translate_refset
	ApiName:	UG_translate_refset_0
	FullName:	?PVTRANS_UG_translate_refset@@YAHIAEBVUString@UGS@@HPEAIPEAHPEBDAEAV12@3PEAX_N@Z
	UnDecorated:	int __cdecl PVTRANS_UG_translate_refset(unsigned int,class UGS::UString const & __ptr64,int,unsigned int * __ptr64,int * __ptr64,char const * __ptr64,class UGS::UString & __ptr64,char const * __ptr64,void * __ptr64,bool)
	Package = PVTRANS_UG
	Versions(#Ordinal):
		2007.1700.0.0 (#5250)
*/
#define UG_translate_refset_0_FULLNAME "?PVTRANS_UG_translate_refset@@YAHIAEBVUString@UGS@@HPEAIPEAHPEBDAEAV12@3PEAX_N@Z"
typedef int (*pfUG_translate_refset_0)(unsigned int p1	// unsigned int
	,UString_REF p2	// class UGS::UString const & __ptr64
	,int p3	// int
	,unsigned int * p4	// unsigned int * __ptr64
	,int * p5	// int * __ptr64
	,char const * p6	// char const * __ptr64
	,UString_REF p7	// class UGS::UString & __ptr64
	,char const * p8	// char const * __ptr64
	,void * p9	// void * __ptr64
	,BOOL p10	// bool
	);

/*
	PVTRANS_UG_translate_refset
	ApiName:	UG_translate_refset_1
	FullName:	?PVTRANS_UG_translate_refset@@YAHIPEBDHPEAIPEAH0AEAVUString@UGS@@0PEAX_N@Z
	UnDecorated:	int __cdecl PVTRANS_UG_translate_refset(unsigned int,char const * __ptr64,int,unsigned int * __ptr64,int * __ptr64,char const * __ptr64,class UGS::UString & __ptr64,char const * __ptr64,void * __ptr64,bool)
	Package = PVTRANS_UG
	Versions(#Ordinal):
		2007.1700.0.0 (#5251)
*/
#define UG_translate_refset_1_FULLNAME "?PVTRANS_UG_translate_refset@@YAHIPEBDHPEAIPEAH0AEAVUString@UGS@@0PEAX_N@Z"
typedef int (*pfUG_translate_refset_1)(unsigned int p1	// unsigned int
	,char const * p2	// char const * __ptr64
	,int p3	// int
	,unsigned int * p4	// unsigned int * __ptr64
	,int * p5	// int * __ptr64
	,char const * p6	// char const * __ptr64
	,UString_REF p7	// class UGS::UString & __ptr64
	,char const * p8	// char const * __ptr64
	,void * p9	// void * __ptr64
	,BOOL p10	// bool
	);

/*
	PX_export
	ApiName:	PX_export
	FullName:	?PX_export@@YAXPEAIHPEAD_N2H22@Z
	UnDecorated:	void __cdecl PX_export(unsigned int * __ptr64,int,char * __ptr64,bool,bool,int,bool,bool)
	Package = PX
	Versions(#Ordinal):
		2007.1700.0.0 (#5357)
*/
#define PX_export_FULLNAME "?PX_export@@YAXPEAIHPEAD_N2H22@Z"
typedef void (*pfPX_export)(unsigned int * p1	// unsigned int * __ptr64
	,int p2	// int
	,char * p3	// char * __ptr64
	,BOOL p4	// bool
	,BOOL p5	// bool
	,int p6	// int
	,BOOL p7	// bool
	,BOOL p8	// bool
	);


#endif
