/*
	Library:	libjafreeformsurfaces.dll
*/
#include <hook/hook_defs.h>
#include "libjafreeformsurfaces.h"

/*
	ExportSubdivisionGeometryBuilder_GetFeatureSet
	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder::GetFeatureSet(void)const __ptr64
*/
static pfExportSubdivisionGeometryBuilder_GetFeatureSet ExportSubdivisionGeometryBuilder_GetFeatureSet_original;
SELECT_OBJECT_list_PTR ExportSubdivisionGeometryBuilder_GetFeatureSet(SubdivisionGeometryBuilder_PTR obj)
{
	SELECT_OBJECT_list_PTR ret = NULL;
	if(ExportSubdivisionGeometryBuilder_GetFeatureSet_original == NULL)
	{
		auto m = GetModuleHandle("libjafreeformsurfaces.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjafreeformsurfaces.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		ExportSubdivisionGeometryBuilder_GetFeatureSet_original = (pfExportSubdivisionGeometryBuilder_GetFeatureSet)GetProcAddress(m, ExportSubdivisionGeometryBuilder_GetFeatureSet_FULLNAME);
		if(ExportSubdivisionGeometryBuilder_GetFeatureSet_original == NULL)
		{
			ERRLOG("GetProcAddress(" ExportSubdivisionGeometryBuilder_GetFeatureSet_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" ExportSubdivisionGeometryBuilder_GetFeatureSet_FULLNAME ") returns %p)", ExportSubdivisionGeometryBuilder_GetFeatureSet_original);
		}
	}
	CALL_NEXT(ret = ExportSubdivisionGeometryBuilder_GetFeatureSet_original(obj));
	CALL_END("ExportSubdivisionGeometryBuilder_GetFeatureSet(obj='%p') returns '%p'", obj, ret);
	return ret;
}


