/*
	Library:	libdisp.dll
*/
#ifndef NXL_LIBDISP_DLL_H_INCLUDED
#define NXL_LIBDISP_DLL_H_INCLUDED

/* typedef for 3 funcs */
/*
	UGS::Gateway::ImageExportBuilder::GetFileFormat
	ApiName:	ImageExportBuilder_GetFileFormat
	FullName:	?GetFileFormat@ImageExportBuilder@Gateway@UGS@@QEBA?AW4JA_IMAGE_EXPORT_BUILDER_FileFormats@@XZ
	UnDecorated:	public: enum JA_IMAGE_EXPORT_BUILDER_FileFormats __cdecl UGS::Gateway::ImageExportBuilder::GetFileFormat(void)const __ptr64
	Package = UGS::Gateway::ImageExportBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#7132)
*/
#define ImageExportBuilder_GetFileFormat_FULLNAME "?GetFileFormat@ImageExportBuilder@Gateway@UGS@@QEBA?AW4JA_IMAGE_EXPORT_BUILDER_FileFormats@@XZ"
typedef CPP_ENUM (*pfImageExportBuilder_GetFileFormat)(CPP_PTR obj	// class UGS::Gateway::ImageExportBuilder * __ptr64
	);
extern CPP_ENUM ImageExportBuilder_GetFileFormat(CPP_PTR obj	// class UGS::Gateway::ImageExportBuilder * __ptr64
	);

/*
	UGS::Gateway::ImageExportBuilder::GetFileName
	ApiName:	ImageExportBuilder_GetFileName
	FullName:	?GetFileName@ImageExportBuilder@Gateway@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Gateway::ImageExportBuilder::GetFileName(void)const __ptr64
	Package = UGS::Gateway::ImageExportBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#7136)
*/
#define ImageExportBuilder_GetFileName_FULLNAME "?GetFileName@ImageExportBuilder@Gateway@UGS@@QEBAPEBDXZ"
typedef char const * (*pfImageExportBuilder_GetFileName)(CPP_PTR obj	// class UGS::Gateway::ImageExportBuilder * __ptr64
	);
extern char const * ImageExportBuilder_GetFileName(CPP_PTR obj	// class UGS::Gateway::ImageExportBuilder * __ptr64
	);

/*
	UGS::Gateway::ImageExportBuilder::commit
	ApiName:	ImageExportBuilder_commit
	FullName:	?commit@ImageExportBuilder@Gateway@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Gateway::ImageExportBuilder::commit(void) __ptr64
	Package = UGS::Gateway::ImageExportBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#14939)
*/
#define ImageExportBuilder_commit_FULLNAME "?commit@ImageExportBuilder@Gateway@UGS@@UEAAIXZ"
typedef unsigned int (*pfImageExportBuilder_commit)(CPP_PTR obj	// class UGS::Gateway::ImageExportBuilder * __ptr64
	);


#endif
