/*
	Library:	libpartdisp.dll
*/
#include <hook/hook_defs.h>
#include "libpartdisp.h"

/*
	PrintPDFBuilder_GetAction
	public: virtual enum JA_PRINT_PDFBUILDER_action_option __cdecl UGS::Gateway::PrintPDFBuilder::GetAction(void)const __ptr64
*/
static pfPrintPDFBuilder_GetAction PrintPDFBuilder_GetAction_original;
CPP_ENUM PrintPDFBuilder_GetAction(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetAction_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetAction_original = (pfPrintPDFBuilder_GetAction)GetProcAddress(m, PrintPDFBuilder_GetAction_FULLNAME);
		if(PrintPDFBuilder_GetAction_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetAction_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetAction_FULLNAME ") returns %p)", PrintPDFBuilder_GetAction_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetAction_original(obj));
	CALL_END("PrintPDFBuilder_GetAction(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetAddWatermark
	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetAddWatermark(void)const __ptr64
*/
static pfPrintPDFBuilder_GetAddWatermark PrintPDFBuilder_GetAddWatermark_original;
BOOL PrintPDFBuilder_GetAddWatermark(PrintPDFBuilder_PTR obj)
{
	BOOL ret = FALSE;
	if(PrintPDFBuilder_GetAddWatermark_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetAddWatermark_original = (pfPrintPDFBuilder_GetAddWatermark)GetProcAddress(m, PrintPDFBuilder_GetAddWatermark_FULLNAME);
		if(PrintPDFBuilder_GetAddWatermark_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetAddWatermark_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetAddWatermark_FULLNAME ") returns %p)", PrintPDFBuilder_GetAddWatermark_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetAddWatermark_original(obj));
	CALL_END("PrintPDFBuilder_GetAddWatermark(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetAppend
	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetAppend(void)const __ptr64
*/
static pfPrintPDFBuilder_GetAppend PrintPDFBuilder_GetAppend_original;
BOOL PrintPDFBuilder_GetAppend(PrintPDFBuilder_PTR obj)
{
	BOOL ret = FALSE;
	if(PrintPDFBuilder_GetAppend_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetAppend_original = (pfPrintPDFBuilder_GetAppend)GetProcAddress(m, PrintPDFBuilder_GetAppend_FULLNAME);
		if(PrintPDFBuilder_GetAppend_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetAppend_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetAppend_FULLNAME ") returns %p)", PrintPDFBuilder_GetAppend_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetAppend_original(obj));
	CALL_END("PrintPDFBuilder_GetAppend(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetCDF
	public: virtual class UGS::Gateway::CDF * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetCDF(void) __ptr64
*/
static pfPrintPDFBuilder_GetCDF PrintPDFBuilder_GetCDF_original;
CPP_PTR PrintPDFBuilder_GetCDF(PrintPDFBuilder_PTR obj)
{
	CPP_PTR ret = NULL;
	if(PrintPDFBuilder_GetCDF_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetCDF_original = (pfPrintPDFBuilder_GetCDF)GetProcAddress(m, PrintPDFBuilder_GetCDF_FULLNAME);
		if(PrintPDFBuilder_GetCDF_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetCDF_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetCDF_FULLNAME ") returns %p)", PrintPDFBuilder_GetCDF_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetCDF_original(obj));
	CALL_END("PrintPDFBuilder_GetCDF(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetColors
	public: virtual enum JA_PRINT_PDFBUILDER_color __cdecl UGS::Gateway::PrintPDFBuilder::GetColors(void)const __ptr64
*/
static pfPrintPDFBuilder_GetColors PrintPDFBuilder_GetColors_original;
CPP_ENUM PrintPDFBuilder_GetColors(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetColors_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetColors_original = (pfPrintPDFBuilder_GetColors)GetProcAddress(m, PrintPDFBuilder_GetColors_FULLNAME);
		if(PrintPDFBuilder_GetColors_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetColors_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetColors_FULLNAME ") returns %p)", PrintPDFBuilder_GetColors_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetColors_original(obj));
	CALL_END("PrintPDFBuilder_GetColors(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetDatasetName
	public: virtual struct TEXT_s const * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetDatasetName(void)const __ptr64
*/
static pfPrintPDFBuilder_GetDatasetName PrintPDFBuilder_GetDatasetName_original;
UTEXT_PTR PrintPDFBuilder_GetDatasetName(PrintPDFBuilder_PTR obj)
{
	UTEXT_PTR ret = NULL;
	if(PrintPDFBuilder_GetDatasetName_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetDatasetName_original = (pfPrintPDFBuilder_GetDatasetName)GetProcAddress(m, PrintPDFBuilder_GetDatasetName_FULLNAME);
		if(PrintPDFBuilder_GetDatasetName_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetDatasetName_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetDatasetName_FULLNAME ") returns %p)", PrintPDFBuilder_GetDatasetName_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetDatasetName_original(obj));
	CALL_END("PrintPDFBuilder_GetDatasetName(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetDatasets
	public: void __cdecl UGS::Gateway::PrintPDFBuilder::GetDatasets(int * __ptr64,char * __ptr64 * __ptr64 * __ptr64)const __ptr64
*/
static pfPrintPDFBuilder_GetDatasets PrintPDFBuilder_GetDatasets_original;
void PrintPDFBuilder_GetDatasets(PrintPDFBuilder_PTR obj, int * p1, char *** p2)
{
	if(PrintPDFBuilder_GetDatasets_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		PrintPDFBuilder_GetDatasets_original = (pfPrintPDFBuilder_GetDatasets)GetProcAddress(m, PrintPDFBuilder_GetDatasets_FULLNAME);
		if(PrintPDFBuilder_GetDatasets_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetDatasets_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetDatasets_FULLNAME ") returns %p)", PrintPDFBuilder_GetDatasets_original);
		}
	}
	CALL_NEXT(PrintPDFBuilder_GetDatasets_original(obj, p1, p2));
	CALL_END("PrintPDFBuilder_GetDatasets(obj='%p', *p1='%d', p2)", obj, *p1);
}

/*
	PrintPDFBuilder_GetFilename
	public: virtual struct TEXT_s const * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetFilename(void)const __ptr64
*/
static pfPrintPDFBuilder_GetFilename PrintPDFBuilder_GetFilename_original;
UTEXT_PTR PrintPDFBuilder_GetFilename(PrintPDFBuilder_PTR obj)
{
	UTEXT_PTR ret = NULL;
	if(PrintPDFBuilder_GetFilename_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetFilename_original = (pfPrintPDFBuilder_GetFilename)GetProcAddress(m, PrintPDFBuilder_GetFilename_FULLNAME);
		if(PrintPDFBuilder_GetFilename_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetFilename_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetFilename_FULLNAME ") returns %p)", PrintPDFBuilder_GetFilename_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetFilename_original(obj));
	CALL_END("PrintPDFBuilder_GetFilename(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetImageResolution
	public: virtual enum JA_PRINT_PDFBUILDER_image_resolution_option __cdecl UGS::Gateway::PrintPDFBuilder::GetImageResolution(void)const __ptr64
*/
static pfPrintPDFBuilder_GetImageResolution PrintPDFBuilder_GetImageResolution_original;
CPP_ENUM PrintPDFBuilder_GetImageResolution(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetImageResolution_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetImageResolution_original = (pfPrintPDFBuilder_GetImageResolution)GetProcAddress(m, PrintPDFBuilder_GetImageResolution_FULLNAME);
		if(PrintPDFBuilder_GetImageResolution_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetImageResolution_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetImageResolution_FULLNAME ") returns %p)", PrintPDFBuilder_GetImageResolution_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetImageResolution_original(obj));
	CALL_END("PrintPDFBuilder_GetImageResolution(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetOutputText
	public: virtual enum JA_PRINT_PDFBUILDER_output_text_option __cdecl UGS::Gateway::PrintPDFBuilder::GetOutputText(void)const __ptr64
*/
static pfPrintPDFBuilder_GetOutputText PrintPDFBuilder_GetOutputText_original;
CPP_ENUM PrintPDFBuilder_GetOutputText(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetOutputText_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetOutputText_original = (pfPrintPDFBuilder_GetOutputText)GetProcAddress(m, PrintPDFBuilder_GetOutputText_FULLNAME);
		if(PrintPDFBuilder_GetOutputText_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetOutputText_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetOutputText_FULLNAME ") returns %p)", PrintPDFBuilder_GetOutputText_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetOutputText_original(obj));
	CALL_END("PrintPDFBuilder_GetOutputText(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetRasterImages
	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetRasterImages(void)const __ptr64
*/
static pfPrintPDFBuilder_GetRasterImages PrintPDFBuilder_GetRasterImages_original;
BOOL PrintPDFBuilder_GetRasterImages(PrintPDFBuilder_PTR obj)
{
	BOOL ret = FALSE;
	if(PrintPDFBuilder_GetRasterImages_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetRasterImages_original = (pfPrintPDFBuilder_GetRasterImages)GetProcAddress(m, PrintPDFBuilder_GetRasterImages_FULLNAME);
		if(PrintPDFBuilder_GetRasterImages_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetRasterImages_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetRasterImages_FULLNAME ") returns %p)", PrintPDFBuilder_GetRasterImages_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetRasterImages_original(obj));
	CALL_END("PrintPDFBuilder_GetRasterImages(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetRelation
	public: enum JA_PRINT_PDFBUILDER_relation_option __cdecl UGS::Gateway::PrintPDFBuilder::GetRelation(void)const __ptr64
*/
static pfPrintPDFBuilder_GetRelation PrintPDFBuilder_GetRelation_original;
CPP_ENUM PrintPDFBuilder_GetRelation(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetRelation_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetRelation_original = (pfPrintPDFBuilder_GetRelation)GetProcAddress(m, PrintPDFBuilder_GetRelation_FULLNAME);
		if(PrintPDFBuilder_GetRelation_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetRelation_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetRelation_FULLNAME ") returns %p)", PrintPDFBuilder_GetRelation_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetRelation_original(obj));
	CALL_END("PrintPDFBuilder_GetRelation(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetScale
	public: double __cdecl UGS::Gateway::PrintPDFBuilder::GetScale(void)const __ptr64
*/
static pfPrintPDFBuilder_GetScale PrintPDFBuilder_GetScale_original;
double PrintPDFBuilder_GetScale(PrintPDFBuilder_PTR obj)
{
	double ret = 0;
	if(PrintPDFBuilder_GetScale_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetScale_original = (pfPrintPDFBuilder_GetScale)GetProcAddress(m, PrintPDFBuilder_GetScale_FULLNAME);
		if(PrintPDFBuilder_GetScale_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetScale_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetScale_FULLNAME ") returns %p)", PrintPDFBuilder_GetScale_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetScale_original(obj));
	CALL_END("PrintPDFBuilder_GetScale(obj='%p') returns '%lf'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetShadedGeometry
	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetShadedGeometry(void)const __ptr64
*/
static pfPrintPDFBuilder_GetShadedGeometry PrintPDFBuilder_GetShadedGeometry_original;
BOOL PrintPDFBuilder_GetShadedGeometry(PrintPDFBuilder_PTR obj)
{
	BOOL ret = FALSE;
	if(PrintPDFBuilder_GetShadedGeometry_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetShadedGeometry_original = (pfPrintPDFBuilder_GetShadedGeometry)GetProcAddress(m, PrintPDFBuilder_GetShadedGeometry_FULLNAME);
		if(PrintPDFBuilder_GetShadedGeometry_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetShadedGeometry_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetShadedGeometry_FULLNAME ") returns %p)", PrintPDFBuilder_GetShadedGeometry_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetShadedGeometry_original(obj));
	CALL_END("PrintPDFBuilder_GetShadedGeometry(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetSize
	public: virtual enum JA_PRINT_PDFBUILDER_size_option __cdecl UGS::Gateway::PrintPDFBuilder::GetSize(void)const __ptr64
*/
static pfPrintPDFBuilder_GetSize PrintPDFBuilder_GetSize_original;
CPP_ENUM PrintPDFBuilder_GetSize(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetSize_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetSize_original = (pfPrintPDFBuilder_GetSize)GetProcAddress(m, PrintPDFBuilder_GetSize_FULLNAME);
		if(PrintPDFBuilder_GetSize_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetSize_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetSize_FULLNAME ") returns %p)", PrintPDFBuilder_GetSize_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetSize_original(obj));
	CALL_END("PrintPDFBuilder_GetSize(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetSourceBuilder
	public: class UGS::Gateway::PlotSourceBuilder * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetSourceBuilder(void)const __ptr64
*/
static pfPrintPDFBuilder_GetSourceBuilder PrintPDFBuilder_GetSourceBuilder_original;
CPP_PTR PrintPDFBuilder_GetSourceBuilder(PrintPDFBuilder_PTR obj)
{
	CPP_PTR ret = NULL;
	if(PrintPDFBuilder_GetSourceBuilder_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetSourceBuilder_original = (pfPrintPDFBuilder_GetSourceBuilder)GetProcAddress(m, PrintPDFBuilder_GetSourceBuilder_FULLNAME);
		if(PrintPDFBuilder_GetSourceBuilder_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetSourceBuilder_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetSourceBuilder_FULLNAME ") returns %p)", PrintPDFBuilder_GetSourceBuilder_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetSourceBuilder_original(obj));
	CALL_END("PrintPDFBuilder_GetSourceBuilder(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetUnits
	public: virtual enum JA_PRINT_PDFBUILDER_units_option __cdecl UGS::Gateway::PrintPDFBuilder::GetUnits(void)const __ptr64
*/
static pfPrintPDFBuilder_GetUnits PrintPDFBuilder_GetUnits_original;
CPP_ENUM PrintPDFBuilder_GetUnits(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetUnits_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetUnits_original = (pfPrintPDFBuilder_GetUnits)GetProcAddress(m, PrintPDFBuilder_GetUnits_FULLNAME);
		if(PrintPDFBuilder_GetUnits_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetUnits_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetUnits_FULLNAME ") returns %p)", PrintPDFBuilder_GetUnits_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetUnits_original(obj));
	CALL_END("PrintPDFBuilder_GetUnits(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetWatermark
	public: virtual struct TEXT_s const * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetWatermark(void)const __ptr64
*/
static pfPrintPDFBuilder_GetWatermark PrintPDFBuilder_GetWatermark_original;
UTEXT_PTR PrintPDFBuilder_GetWatermark(PrintPDFBuilder_PTR obj)
{
	UTEXT_PTR ret = NULL;
	if(PrintPDFBuilder_GetWatermark_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetWatermark_original = (pfPrintPDFBuilder_GetWatermark)GetProcAddress(m, PrintPDFBuilder_GetWatermark_FULLNAME);
		if(PrintPDFBuilder_GetWatermark_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetWatermark_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetWatermark_FULLNAME ") returns %p)", PrintPDFBuilder_GetWatermark_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetWatermark_original(obj));
	CALL_END("PrintPDFBuilder_GetWatermark(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetWidthDefinition
	public: virtual class UGS::Gateway::WidthDefinition * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetWidthDefinition(void) __ptr64
*/
static pfPrintPDFBuilder_GetWidthDefinition PrintPDFBuilder_GetWidthDefinition_original;
CPP_PTR PrintPDFBuilder_GetWidthDefinition(PrintPDFBuilder_PTR obj)
{
	CPP_PTR ret = NULL;
	if(PrintPDFBuilder_GetWidthDefinition_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetWidthDefinition_original = (pfPrintPDFBuilder_GetWidthDefinition)GetProcAddress(m, PrintPDFBuilder_GetWidthDefinition_FULLNAME);
		if(PrintPDFBuilder_GetWidthDefinition_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetWidthDefinition_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetWidthDefinition_FULLNAME ") returns %p)", PrintPDFBuilder_GetWidthDefinition_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetWidthDefinition_original(obj));
	CALL_END("PrintPDFBuilder_GetWidthDefinition(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetWidths
	public: virtual enum JA_PRINT_PDFBUILDER_width __cdecl UGS::Gateway::PrintPDFBuilder::GetWidths(void)const __ptr64
*/
static pfPrintPDFBuilder_GetWidths PrintPDFBuilder_GetWidths_original;
CPP_ENUM PrintPDFBuilder_GetWidths(PrintPDFBuilder_PTR obj)
{
	CPP_ENUM ret = 0;
	if(PrintPDFBuilder_GetWidths_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetWidths_original = (pfPrintPDFBuilder_GetWidths)GetProcAddress(m, PrintPDFBuilder_GetWidths_FULLNAME);
		if(PrintPDFBuilder_GetWidths_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetWidths_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetWidths_FULLNAME ") returns %p)", PrintPDFBuilder_GetWidths_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetWidths_original(obj));
	CALL_END("PrintPDFBuilder_GetWidths(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetXDimension
	public: double __cdecl UGS::Gateway::PrintPDFBuilder::GetXDimension(void)const __ptr64
*/
static pfPrintPDFBuilder_GetXDimension PrintPDFBuilder_GetXDimension_original;
double PrintPDFBuilder_GetXDimension(PrintPDFBuilder_PTR obj)
{
	double ret = 0;
	if(PrintPDFBuilder_GetXDimension_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetXDimension_original = (pfPrintPDFBuilder_GetXDimension)GetProcAddress(m, PrintPDFBuilder_GetXDimension_FULLNAME);
		if(PrintPDFBuilder_GetXDimension_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetXDimension_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetXDimension_FULLNAME ") returns %p)", PrintPDFBuilder_GetXDimension_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetXDimension_original(obj));
	CALL_END("PrintPDFBuilder_GetXDimension(obj='%p') returns '%lf'", obj, ret);
	return ret;
}

/*
	PrintPDFBuilder_GetYDimension
	public: double __cdecl UGS::Gateway::PrintPDFBuilder::GetYDimension(void)const __ptr64
*/
static pfPrintPDFBuilder_GetYDimension PrintPDFBuilder_GetYDimension_original;
double PrintPDFBuilder_GetYDimension(PrintPDFBuilder_PTR obj)
{
	double ret = 0;
	if(PrintPDFBuilder_GetYDimension_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PrintPDFBuilder_GetYDimension_original = (pfPrintPDFBuilder_GetYDimension)GetProcAddress(m, PrintPDFBuilder_GetYDimension_FULLNAME);
		if(PrintPDFBuilder_GetYDimension_original == NULL)
		{
			ERRLOG("GetProcAddress(" PrintPDFBuilder_GetYDimension_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PrintPDFBuilder_GetYDimension_FULLNAME ") returns %p)", PrintPDFBuilder_GetYDimension_original);
		}
	}
	CALL_NEXT(ret = PrintPDFBuilder_GetYDimension_original(obj));
	CALL_END("PrintPDFBuilder_GetYDimension(obj='%p') returns '%lf'", obj, ret);
	return ret;
}

/*
	SELECT_OBJECT_list_ask_size
	public: virtual int __cdecl UGS::SELECT_OBJECT_list::ask_size(void)const __ptr64
*/
static pfSELECT_OBJECT_list_ask_size SELECT_OBJECT_list_ask_size_original;
int SELECT_OBJECT_list_ask_size(SELECT_OBJECT_list_PTR obj)
{
	int ret = 0;
	if(SELECT_OBJECT_list_ask_size_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		SELECT_OBJECT_list_ask_size_original = (pfSELECT_OBJECT_list_ask_size)GetProcAddress(m, SELECT_OBJECT_list_ask_size_FULLNAME);
		if(SELECT_OBJECT_list_ask_size_original == NULL)
		{
			ERRLOG("GetProcAddress(" SELECT_OBJECT_list_ask_size_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" SELECT_OBJECT_list_ask_size_FULLNAME ") returns %p)", SELECT_OBJECT_list_ask_size_original);
		}
	}
	CALL_NEXT(ret = SELECT_OBJECT_list_ask_size_original(obj));
	CALL_END("SELECT_OBJECT_list_ask_size(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	SELECT_OBJECT_list_to_tag_array
	public: virtual unsigned int * __ptr64 __cdecl UGS::SELECT_OBJECT_list::to_tag_array(int * __ptr64)const __ptr64
*/
static pfSELECT_OBJECT_list_to_tag_array SELECT_OBJECT_list_to_tag_array_original;
unsigned int * SELECT_OBJECT_list_to_tag_array(SELECT_OBJECT_list_PTR obj, int * p1)
{
	unsigned int * ret = NULL;
	if(SELECT_OBJECT_list_to_tag_array_original == NULL)
	{
		auto m = GetModuleHandle("libpartdisp.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpartdisp.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		SELECT_OBJECT_list_to_tag_array_original = (pfSELECT_OBJECT_list_to_tag_array)GetProcAddress(m, SELECT_OBJECT_list_to_tag_array_FULLNAME);
		if(SELECT_OBJECT_list_to_tag_array_original == NULL)
		{
			ERRLOG("GetProcAddress(" SELECT_OBJECT_list_to_tag_array_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" SELECT_OBJECT_list_to_tag_array_FULLNAME ") returns %p)", SELECT_OBJECT_list_to_tag_array_original);
		}
	}
	CALL_NEXT(ret = SELECT_OBJECT_list_to_tag_array_original(obj, p1));
	CALL_END("SELECT_OBJECT_list_to_tag_array(obj='%p', *p1='%d') returns '%u'", obj, *p1, *ret);
	return ret;
}


