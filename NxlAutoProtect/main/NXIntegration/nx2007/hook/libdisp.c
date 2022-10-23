/*
	Library:	libdisp.dll
*/
#include <hook/hook_defs.h>
#include "libdisp.h"

/*
	ImageExportBuilder_GetFileFormat
	public: enum JA_IMAGE_EXPORT_BUILDER_FileFormats __cdecl UGS::Gateway::ImageExportBuilder::GetFileFormat(void)const __ptr64
*/
static pfImageExportBuilder_GetFileFormat ImageExportBuilder_GetFileFormat_original;
CPP_ENUM ImageExportBuilder_GetFileFormat(CPP_PTR obj)
{
	CPP_ENUM ret = 0;
	if(ImageExportBuilder_GetFileFormat_original == NULL)
	{
		auto m = GetModuleHandle("libdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		ImageExportBuilder_GetFileFormat_original = (pfImageExportBuilder_GetFileFormat)GetProcAddress(m, ImageExportBuilder_GetFileFormat_FULLNAME);
		if(ImageExportBuilder_GetFileFormat_original == NULL)
		{
			ERRLOG("GetProcAddress(" ImageExportBuilder_GetFileFormat_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" ImageExportBuilder_GetFileFormat_FULLNAME ") returns %p)", ImageExportBuilder_GetFileFormat_original);
		}
	}
	CALL_NEXT(ret = ImageExportBuilder_GetFileFormat_original(obj));
	CALL_END("ImageExportBuilder_GetFileFormat(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	ImageExportBuilder_GetFileName
	public: char const * __ptr64 __cdecl UGS::Gateway::ImageExportBuilder::GetFileName(void)const __ptr64
*/
static pfImageExportBuilder_GetFileName ImageExportBuilder_GetFileName_original;
char const * ImageExportBuilder_GetFileName(CPP_PTR obj)
{
	char const * ret = NULL;
	if(ImageExportBuilder_GetFileName_original == NULL)
	{
		auto m = GetModuleHandle("libdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		ImageExportBuilder_GetFileName_original = (pfImageExportBuilder_GetFileName)GetProcAddress(m, ImageExportBuilder_GetFileName_FULLNAME);
		if(ImageExportBuilder_GetFileName_original == NULL)
		{
			ERRLOG("GetProcAddress(" ImageExportBuilder_GetFileName_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" ImageExportBuilder_GetFileName_FULLNAME ") returns %p)", ImageExportBuilder_GetFileName_original);
		}
	}
	CALL_NEXT(ret = ImageExportBuilder_GetFileName_original(obj));
	CALL_END("ImageExportBuilder_GetFileName(obj='%p') returns '%s'", obj, ret);
	return ret;
}


