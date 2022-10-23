/*
	Library:	libjafreeformsurfaces.dll
*/
#ifndef NXL_LIBJAFREEFORMSURFACES_DLL_H_INCLUDED
#define NXL_LIBJAFREEFORMSURFACES_DLL_H_INCLUDED

#include "libpartdisp.h"

typedef const void *SubdivisionGeometryBuilder_PTR;	// class UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder * __ptr64

/* typedef for 2 funcs */
/*
	UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder::GetFeatureSet
	ApiName:	ExportSubdivisionGeometryBuilder_GetFeatureSet
	FullName:	?GetFeatureSet@ExportSubdivisionGeometryBuilder@ModlSubdivJA@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ
	UnDecorated:	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder::GetFeatureSet(void)const __ptr64
	Package = UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder
	Versions(#Ordinal):
		12.0.2.9 (#826)
*/
#define ExportSubdivisionGeometryBuilder_GetFeatureSet_FULLNAME "?GetFeatureSet@ExportSubdivisionGeometryBuilder@ModlSubdivJA@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ"
typedef SELECT_OBJECT_list_PTR (*pfExportSubdivisionGeometryBuilder_GetFeatureSet)(SubdivisionGeometryBuilder_PTR obj	// class UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder * __ptr64
	);
extern SELECT_OBJECT_list_PTR ExportSubdivisionGeometryBuilder_GetFeatureSet(SubdivisionGeometryBuilder_PTR obj	// class UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder * __ptr64
	);

/*
	UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder::commit
	ApiName:	ExportSubdivisionGeometryBuilder_commit
	FullName:	?commit@ExportSubdivisionGeometryBuilder@ModlSubdivJA@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder::commit(void) __ptr64
	Package = UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder
	Versions(#Ordinal):
		12.0.2.9 (#2135)
*/
#define ExportSubdivisionGeometryBuilder_commit_FULLNAME "?commit@ExportSubdivisionGeometryBuilder@ModlSubdivJA@UGS@@UEAAIXZ"
typedef unsigned int (*pfExportSubdivisionGeometryBuilder_commit)(SubdivisionGeometryBuilder_PTR obj	// class UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder * __ptr64
	);


#endif
