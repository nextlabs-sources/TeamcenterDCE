/*
	Library:	libpartdisp.dll
*/
#ifndef NXL_LIBPARTDISP_DLL_H_INCLUDED
#define NXL_LIBPARTDISP_DLL_H_INCLUDED

#include "libsyss.h"
#include <uf_cgm_types.h>

typedef const void *PrintPDFBuilder_PTR;	// class UGS::Gateway::PrintPDFBuilder * __ptr64
typedef const void *SELECT_OBJECT_list_PTR;	// class UGS::SELECT_OBJECT_list * __ptr64

/* typedef for 29 funcs */
/*
	CGME_create_cgm
	ApiName:	CGME_create_cgm
	FullName:	?CGME_create_cgm@@YAXW4UF_CGM_export_reason_e@@IPEAUCGM_options_s@@PEAUCGM_custom_colors_s@@PEAUCGME_custom_widths_s@@P6AXPEAUCGM_file_s@@IPEAX@Z5_NPEBD@Z
	UnDecorated:	void __cdecl CGME_create_cgm(enum UF_CGM_export_reason_e,unsigned int,struct CGM_options_s * __ptr64,struct CGM_custom_colors_s * __ptr64,struct CGME_custom_widths_s * __ptr64,void (__cdecl*)(struct CGM_file_s * __ptr64,unsigned int,void * __ptr64),void * __ptr64,bool,char const * __ptr64)
	Package = CGME
	Versions(#Ordinal):
		2007.1700.0.0 (#213)
*/
#define CGME_create_cgm_FULLNAME "?CGME_create_cgm@@YAXW4UF_CGM_export_reason_e@@IPEAUCGM_options_s@@PEAUCGM_custom_colors_s@@PEAUCGME_custom_widths_s@@P6AXPEAUCGM_file_s@@IPEAX@Z5_NPEBD@Z"
typedef void (*pfCGME_create_cgm)(UF_CGM_export_reason_t p1	// enum UF_CGM_export_reason_e
	,unsigned int p2	// unsigned int
	,UF_CGM_export_options_p_t p3	// struct CGM_options_s * __ptr64
	,UF_CGM_custom_colors_p_t p4	// struct CGM_custom_colors_s * __ptr64
	,UF_CGM_custom_widths_p_t p5	// struct CGME_custom_widths_s * __ptr64
	,CALLBACK_PTR p6	// void (__cdecl*)(struct CGM_file_s * __ptr64,unsigned int,void * __ptr64)
	,void * p7	// void * __ptr64
	,BOOL p8	// bool
	,char const * p9	// char const * __ptr64
	);

/*
	CGME_export_cgm
	ApiName:	CGME_export_cgm
	FullName:	?CGME_export_cgm@@YAXW4UF_CGM_export_reason_e@@IPEAUCGM_options_s@@_NPEBD@Z
	UnDecorated:	void __cdecl CGME_export_cgm(enum UF_CGM_export_reason_e,unsigned int,struct CGM_options_s * __ptr64,bool,char const * __ptr64)
	Package = CGME
	Versions(#Ordinal):
		2007.1700.0.0 (#214)
*/
#define CGME_export_cgm_FULLNAME "?CGME_export_cgm@@YAXW4UF_CGM_export_reason_e@@IPEAUCGM_options_s@@_NPEBD@Z"
typedef void (*pfCGME_export_cgm)(UF_CGM_export_reason_t p1	// enum UF_CGM_export_reason_e
	,unsigned int p2	// unsigned int
	,UF_CGM_export_options_p_t p3	// struct CGM_options_s * __ptr64
	,BOOL p4	// bool
	,char const * p5	// char const * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetAction
	ApiName:	PrintPDFBuilder_GetAction
	FullName:	?GetAction@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_action_option@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_action_option __cdecl UGS::Gateway::PrintPDFBuilder::GetAction(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#330)
*/
#define PrintPDFBuilder_GetAction_FULLNAME "?GetAction@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_action_option@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetAction)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetAction(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetAddWatermark
	ApiName:	PrintPDFBuilder_GetAddWatermark
	FullName:	?GetAddWatermark@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ
	UnDecorated:	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetAddWatermark(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#334)
*/
#define PrintPDFBuilder_GetAddWatermark_FULLNAME "?GetAddWatermark@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ"
typedef BOOL (*pfPrintPDFBuilder_GetAddWatermark)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern BOOL PrintPDFBuilder_GetAddWatermark(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetAppend
	ApiName:	PrintPDFBuilder_GetAppend
	FullName:	?GetAppend@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ
	UnDecorated:	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetAppend(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#338)
*/
#define PrintPDFBuilder_GetAppend_FULLNAME "?GetAppend@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ"
typedef BOOL (*pfPrintPDFBuilder_GetAppend)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern BOOL PrintPDFBuilder_GetAppend(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetCDF
	ApiName:	PrintPDFBuilder_GetCDF
	FullName:	?GetCDF@PrintPDFBuilder@Gateway@UGS@@UEAAPEAVCDF@23@XZ
	UnDecorated:	public: virtual class UGS::Gateway::CDF * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetCDF(void) __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#345)
*/
#define PrintPDFBuilder_GetCDF_FULLNAME "?GetCDF@PrintPDFBuilder@Gateway@UGS@@UEAAPEAVCDF@23@XZ"
typedef CPP_PTR (*pfPrintPDFBuilder_GetCDF)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_PTR PrintPDFBuilder_GetCDF(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetColors
	ApiName:	PrintPDFBuilder_GetColors
	FullName:	?GetColors@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_color@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_color __cdecl UGS::Gateway::PrintPDFBuilder::GetColors(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#351)
*/
#define PrintPDFBuilder_GetColors_FULLNAME "?GetColors@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_color@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetColors)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetColors(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetDatasetName
	ApiName:	PrintPDFBuilder_GetDatasetName
	FullName:	?GetDatasetName@PrintPDFBuilder@Gateway@UGS@@UEBAPEBUTEXT_s@@XZ
	UnDecorated:	public: virtual struct TEXT_s const * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetDatasetName(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#357)
*/
#define PrintPDFBuilder_GetDatasetName_FULLNAME "?GetDatasetName@PrintPDFBuilder@Gateway@UGS@@UEBAPEBUTEXT_s@@XZ"
typedef UTEXT_PTR (*pfPrintPDFBuilder_GetDatasetName)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern UTEXT_PTR PrintPDFBuilder_GetDatasetName(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetDatasetType
	FullName:	?GetDatasetType@PrintPDFBuilder@Gateway@UGS@@QEBA?AVUString@3@XZ
	UnDecorated:	public: class UGS::UString __cdecl UGS::Gateway::PrintPDFBuilder::GetDatasetType(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#360)
	Comments:
		Return: Type('UGS::UString') is C++ Class instead of pointer
*/

/*
	UGS::Gateway::PrintPDFBuilder::GetDatasets
	ApiName:	PrintPDFBuilder_GetDatasets
	FullName:	?GetDatasets@PrintPDFBuilder@Gateway@UGS@@QEBAXPEAHPEAPEAPEAD@Z
	UnDecorated:	public: void __cdecl UGS::Gateway::PrintPDFBuilder::GetDatasets(int * __ptr64,char * __ptr64 * __ptr64 * __ptr64)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#361)
*/
#define PrintPDFBuilder_GetDatasets_FULLNAME "?GetDatasets@PrintPDFBuilder@Gateway@UGS@@QEBAXPEAHPEAPEAPEAD@Z"
typedef void (*pfPrintPDFBuilder_GetDatasets)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	,int * p1	// int * __ptr64
	,char *** p2	// char * __ptr64 * __ptr64 * __ptr64
	);
extern void PrintPDFBuilder_GetDatasets(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	,int * p1	// int * __ptr64
	,char *** p2	// char * __ptr64 * __ptr64 * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetFilename
	ApiName:	PrintPDFBuilder_GetFilename
	FullName:	?GetFilename@PrintPDFBuilder@Gateway@UGS@@UEBAPEBUTEXT_s@@XZ
	UnDecorated:	public: virtual struct TEXT_s const * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetFilename(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#370)
*/
#define PrintPDFBuilder_GetFilename_FULLNAME "?GetFilename@PrintPDFBuilder@Gateway@UGS@@UEBAPEBUTEXT_s@@XZ"
typedef UTEXT_PTR (*pfPrintPDFBuilder_GetFilename)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern UTEXT_PTR PrintPDFBuilder_GetFilename(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetImageResolution
	ApiName:	PrintPDFBuilder_GetImageResolution
	FullName:	?GetImageResolution@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_image_resolution_option@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_image_resolution_option __cdecl UGS::Gateway::PrintPDFBuilder::GetImageResolution(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#384)
*/
#define PrintPDFBuilder_GetImageResolution_FULLNAME "?GetImageResolution@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_image_resolution_option@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetImageResolution)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetImageResolution(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetNamedReferenceType
	FullName:	?GetNamedReferenceType@PrintPDFBuilder@Gateway@UGS@@QEBA?AVUString@3@XZ
	UnDecorated:	public: class UGS::UString __cdecl UGS::Gateway::PrintPDFBuilder::GetNamedReferenceType(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#396)
	Comments:
		Return: Type('UGS::UString') is C++ Class instead of pointer
*/

/*
	UGS::Gateway::PrintPDFBuilder::GetOutputText
	ApiName:	PrintPDFBuilder_GetOutputText
	FullName:	?GetOutputText@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_output_text_option@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_output_text_option __cdecl UGS::Gateway::PrintPDFBuilder::GetOutputText(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#411)
*/
#define PrintPDFBuilder_GetOutputText_FULLNAME "?GetOutputText@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_output_text_option@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetOutputText)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetOutputText(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetRasterImages
	ApiName:	PrintPDFBuilder_GetRasterImages
	FullName:	?GetRasterImages@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ
	UnDecorated:	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetRasterImages(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#427)
*/
#define PrintPDFBuilder_GetRasterImages_FULLNAME "?GetRasterImages@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ"
typedef BOOL (*pfPrintPDFBuilder_GetRasterImages)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern BOOL PrintPDFBuilder_GetRasterImages(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetRelation
	ApiName:	PrintPDFBuilder_GetRelation
	FullName:	?GetRelation@PrintPDFBuilder@Gateway@UGS@@QEBA?AW4JA_PRINT_PDFBUILDER_relation_option@@XZ
	UnDecorated:	public: enum JA_PRINT_PDFBUILDER_relation_option __cdecl UGS::Gateway::PrintPDFBuilder::GetRelation(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#429)
*/
#define PrintPDFBuilder_GetRelation_FULLNAME "?GetRelation@PrintPDFBuilder@Gateway@UGS@@QEBA?AW4JA_PRINT_PDFBUILDER_relation_option@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetRelation)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetRelation(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetScale
	ApiName:	PrintPDFBuilder_GetScale
	FullName:	?GetScale@PrintPDFBuilder@Gateway@UGS@@QEBANXZ
	UnDecorated:	public: double __cdecl UGS::Gateway::PrintPDFBuilder::GetScale(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#437)
*/
#define PrintPDFBuilder_GetScale_FULLNAME "?GetScale@PrintPDFBuilder@Gateway@UGS@@QEBANXZ"
typedef double (*pfPrintPDFBuilder_GetScale)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern double PrintPDFBuilder_GetScale(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetShadedGeometry
	ApiName:	PrintPDFBuilder_GetShadedGeometry
	FullName:	?GetShadedGeometry@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ
	UnDecorated:	public: bool __cdecl UGS::Gateway::PrintPDFBuilder::GetShadedGeometry(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#442)
*/
#define PrintPDFBuilder_GetShadedGeometry_FULLNAME "?GetShadedGeometry@PrintPDFBuilder@Gateway@UGS@@QEBA_NXZ"
typedef BOOL (*pfPrintPDFBuilder_GetShadedGeometry)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern BOOL PrintPDFBuilder_GetShadedGeometry(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetSize
	ApiName:	PrintPDFBuilder_GetSize
	FullName:	?GetSize@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_size_option@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_size_option __cdecl UGS::Gateway::PrintPDFBuilder::GetSize(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#446)
*/
#define PrintPDFBuilder_GetSize_FULLNAME "?GetSize@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_size_option@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetSize)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetSize(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetSourceBuilder
	ApiName:	PrintPDFBuilder_GetSourceBuilder
	FullName:	?GetSourceBuilder@PrintPDFBuilder@Gateway@UGS@@QEBAPEAVPlotSourceBuilder@23@XZ
	UnDecorated:	public: class UGS::Gateway::PlotSourceBuilder * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetSourceBuilder(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#450)
*/
#define PrintPDFBuilder_GetSourceBuilder_FULLNAME "?GetSourceBuilder@PrintPDFBuilder@Gateway@UGS@@QEBAPEAVPlotSourceBuilder@23@XZ"
typedef CPP_PTR (*pfPrintPDFBuilder_GetSourceBuilder)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_PTR PrintPDFBuilder_GetSourceBuilder(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetUnits
	ApiName:	PrintPDFBuilder_GetUnits
	FullName:	?GetUnits@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_units_option@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_units_option __cdecl UGS::Gateway::PrintPDFBuilder::GetUnits(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#459)
*/
#define PrintPDFBuilder_GetUnits_FULLNAME "?GetUnits@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_units_option@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetUnits)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetUnits(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetWatermark
	ApiName:	PrintPDFBuilder_GetWatermark
	FullName:	?GetWatermark@PrintPDFBuilder@Gateway@UGS@@UEBAPEBUTEXT_s@@XZ
	UnDecorated:	public: virtual struct TEXT_s const * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetWatermark(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#464)
*/
#define PrintPDFBuilder_GetWatermark_FULLNAME "?GetWatermark@PrintPDFBuilder@Gateway@UGS@@UEBAPEBUTEXT_s@@XZ"
typedef UTEXT_PTR (*pfPrintPDFBuilder_GetWatermark)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern UTEXT_PTR PrintPDFBuilder_GetWatermark(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetWidthDefinition
	ApiName:	PrintPDFBuilder_GetWidthDefinition
	FullName:	?GetWidthDefinition@PrintPDFBuilder@Gateway@UGS@@UEAAPEAVWidthDefinition@23@XZ
	UnDecorated:	public: virtual class UGS::Gateway::WidthDefinition * __ptr64 __cdecl UGS::Gateway::PrintPDFBuilder::GetWidthDefinition(void) __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#477)
*/
#define PrintPDFBuilder_GetWidthDefinition_FULLNAME "?GetWidthDefinition@PrintPDFBuilder@Gateway@UGS@@UEAAPEAVWidthDefinition@23@XZ"
typedef CPP_PTR (*pfPrintPDFBuilder_GetWidthDefinition)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_PTR PrintPDFBuilder_GetWidthDefinition(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetWidths
	ApiName:	PrintPDFBuilder_GetWidths
	FullName:	?GetWidths@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_width@@XZ
	UnDecorated:	public: virtual enum JA_PRINT_PDFBUILDER_width __cdecl UGS::Gateway::PrintPDFBuilder::GetWidths(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#481)
*/
#define PrintPDFBuilder_GetWidths_FULLNAME "?GetWidths@PrintPDFBuilder@Gateway@UGS@@UEBA?AW4JA_PRINT_PDFBUILDER_width@@XZ"
typedef CPP_ENUM (*pfPrintPDFBuilder_GetWidths)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern CPP_ENUM PrintPDFBuilder_GetWidths(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetXDimension
	ApiName:	PrintPDFBuilder_GetXDimension
	FullName:	?GetXDimension@PrintPDFBuilder@Gateway@UGS@@QEBANXZ
	UnDecorated:	public: double __cdecl UGS::Gateway::PrintPDFBuilder::GetXDimension(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#483)
*/
#define PrintPDFBuilder_GetXDimension_FULLNAME "?GetXDimension@PrintPDFBuilder@Gateway@UGS@@QEBANXZ"
typedef double (*pfPrintPDFBuilder_GetXDimension)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern double PrintPDFBuilder_GetXDimension(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::GetYDimension
	ApiName:	PrintPDFBuilder_GetYDimension
	FullName:	?GetYDimension@PrintPDFBuilder@Gateway@UGS@@QEBANXZ
	UnDecorated:	public: double __cdecl UGS::Gateway::PrintPDFBuilder::GetYDimension(void)const __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#487)
*/
#define PrintPDFBuilder_GetYDimension_FULLNAME "?GetYDimension@PrintPDFBuilder@Gateway@UGS@@QEBANXZ"
typedef double (*pfPrintPDFBuilder_GetYDimension)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);
extern double PrintPDFBuilder_GetYDimension(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::SELECT_OBJECT_list::ask_size
	ApiName:	SELECT_OBJECT_list_ask_size
	FullName:	?ask_size@SELECT_OBJECT_list@UGS@@UEBAHXZ
	UnDecorated:	public: virtual int __cdecl UGS::SELECT_OBJECT_list::ask_size(void)const __ptr64
	Package = UGS::SELECT_OBJECT_list
	Versions(#Ordinal):
		2007.1700.0.0 (#962)
*/
#define SELECT_OBJECT_list_ask_size_FULLNAME "?ask_size@SELECT_OBJECT_list@UGS@@UEBAHXZ"
typedef int (*pfSELECT_OBJECT_list_ask_size)(SELECT_OBJECT_list_PTR obj	// class UGS::SELECT_OBJECT_list * __ptr64
	);
extern int SELECT_OBJECT_list_ask_size(SELECT_OBJECT_list_PTR obj	// class UGS::SELECT_OBJECT_list * __ptr64
	);

/*
	UGS::Gateway::PrintPDFBuilder::commit
	ApiName:	PrintPDFBuilder_commit
	FullName:	?commit@PrintPDFBuilder@Gateway@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Gateway::PrintPDFBuilder::commit(void) __ptr64
	Package = UGS::Gateway::PrintPDFBuilder
	Versions(#Ordinal):
		2007.1700.0.0 (#1096)
*/
#define PrintPDFBuilder_commit_FULLNAME "?commit@PrintPDFBuilder@Gateway@UGS@@UEAAIXZ"
typedef unsigned int (*pfPrintPDFBuilder_commit)(PrintPDFBuilder_PTR obj	// class UGS::Gateway::PrintPDFBuilder * __ptr64
	);

/*
	UGS::SELECT_OBJECT_list::to_tag_array
	ApiName:	SELECT_OBJECT_list_to_tag_array
	FullName:	?to_tag_array@SELECT_OBJECT_list@UGS@@UEBAPEAIPEAH@Z
	UnDecorated:	public: virtual unsigned int * __ptr64 __cdecl UGS::SELECT_OBJECT_list::to_tag_array(int * __ptr64)const __ptr64
	Package = UGS::SELECT_OBJECT_list
	Versions(#Ordinal):
		2007.1700.0.0 (#1405)
*/
#define SELECT_OBJECT_list_to_tag_array_FULLNAME "?to_tag_array@SELECT_OBJECT_list@UGS@@UEBAPEAIPEAH@Z"
typedef unsigned int * (*pfSELECT_OBJECT_list_to_tag_array)(SELECT_OBJECT_list_PTR obj	// class UGS::SELECT_OBJECT_list * __ptr64
	,int * p1	// int * __ptr64
	);
extern unsigned int * SELECT_OBJECT_list_to_tag_array(SELECT_OBJECT_list_PTR obj	// class UGS::SELECT_OBJECT_list * __ptr64
	,int * p1	// int * __ptr64
	);


#endif
