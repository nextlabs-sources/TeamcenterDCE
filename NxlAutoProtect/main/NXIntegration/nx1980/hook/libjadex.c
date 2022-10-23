/*
	Library:	libjadex.dll
*/
#include <hook/hook_defs.h>
#include "libjadex.h"

/*
	DxfdwgCreator_GetExportFrom
	public: enum JA_DXFDWG_CREATOR_ExportFromOption __cdecl UGS::Dex::DxfdwgCreator::GetExportFrom(void)const __ptr64
*/
static pfDxfdwgCreator_GetExportFrom DxfdwgCreator_GetExportFrom_original;
CPP_ENUM DxfdwgCreator_GetExportFrom(DxfdwgCreator_PTR obj)
{
	CPP_ENUM ret = 0;
	if(DxfdwgCreator_GetExportFrom_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		DxfdwgCreator_GetExportFrom_original = (pfDxfdwgCreator_GetExportFrom)GetProcAddress(m, DxfdwgCreator_GetExportFrom_FULLNAME);
		if(DxfdwgCreator_GetExportFrom_original == NULL)
		{
			ERRLOG("GetProcAddress(" DxfdwgCreator_GetExportFrom_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" DxfdwgCreator_GetExportFrom_FULLNAME ") returns %p)", DxfdwgCreator_GetExportFrom_original);
		}
	}
	CALL_NEXT(ret = DxfdwgCreator_GetExportFrom_original(obj));
	CALL_END("DxfdwgCreator_GetExportFrom(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	IgesCreator_GetExportFrom
	public: enum JA_IGES_CREATOR_export_from_option __cdecl UGS::Dex::IgesCreator::GetExportFrom(void)const __ptr64
*/
static pfIgesCreator_GetExportFrom IgesCreator_GetExportFrom_original;
CPP_ENUM IgesCreator_GetExportFrom(IgesCreator_PTR obj)
{
	CPP_ENUM ret = 0;
	if(IgesCreator_GetExportFrom_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		IgesCreator_GetExportFrom_original = (pfIgesCreator_GetExportFrom)GetProcAddress(m, IgesCreator_GetExportFrom_FULLNAME);
		if(IgesCreator_GetExportFrom_original == NULL)
		{
			ERRLOG("GetProcAddress(" IgesCreator_GetExportFrom_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" IgesCreator_GetExportFrom_FULLNAME ") returns %p)", IgesCreator_GetExportFrom_original);
		}
	}
	CALL_NEXT(ret = IgesCreator_GetExportFrom_original(obj));
	CALL_END("IgesCreator_GetExportFrom(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	NXTo2dCreator_GetExportFrom
	public: enum JA_NXTO2D_CREATOR_ExportFromOption __cdecl UGS::Dex::NXTo2dCreator::GetExportFrom(void)const __ptr64
*/
static pfNXTo2dCreator_GetExportFrom NXTo2dCreator_GetExportFrom_original;
CPP_ENUM NXTo2dCreator_GetExportFrom(NXTo2dCreator_PTR obj)
{
	CPP_ENUM ret = 0;
	if(NXTo2dCreator_GetExportFrom_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		NXTo2dCreator_GetExportFrom_original = (pfNXTo2dCreator_GetExportFrom)GetProcAddress(m, NXTo2dCreator_GetExportFrom_FULLNAME);
		if(NXTo2dCreator_GetExportFrom_original == NULL)
		{
			ERRLOG("GetProcAddress(" NXTo2dCreator_GetExportFrom_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" NXTo2dCreator_GetExportFrom_FULLNAME ") returns %p)", NXTo2dCreator_GetExportFrom_original);
		}
	}
	CALL_NEXT(ret = NXTo2dCreator_GetExportFrom_original(obj));
	CALL_END("NXTo2dCreator_GetExportFrom(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	StepCreator_GetExportFrom
	public: enum JA_STEP_CREATOR_ExportFromOption __cdecl UGS::Dex::StepCreator::GetExportFrom(void)const __ptr64
*/
static pfStepCreator_GetExportFrom StepCreator_GetExportFrom_original;
CPP_ENUM StepCreator_GetExportFrom(StepCreator_PTR obj)
{
	CPP_ENUM ret = 0;
	if(StepCreator_GetExportFrom_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		StepCreator_GetExportFrom_original = (pfStepCreator_GetExportFrom)GetProcAddress(m, StepCreator_GetExportFrom_FULLNAME);
		if(StepCreator_GetExportFrom_original == NULL)
		{
			ERRLOG("GetProcAddress(" StepCreator_GetExportFrom_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" StepCreator_GetExportFrom_FULLNAME ") returns %p)", StepCreator_GetExportFrom_original);
		}
	}
	CALL_NEXT(ret = StepCreator_GetExportFrom_original(obj));
	CALL_END("StepCreator_GetExportFrom(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	WavefrontObjCreator_GetExportFrom
	public: enum JA_WAVEFRONT_OBJ_CREATOR_ExportFromOption __cdecl UGS::Dex::WavefrontObjCreator::GetExportFrom(void)const __ptr64
*/
static pfWavefrontObjCreator_GetExportFrom WavefrontObjCreator_GetExportFrom_original;
CPP_ENUM WavefrontObjCreator_GetExportFrom(CPP_PTR obj)
{
	CPP_ENUM ret = 0;
	if(WavefrontObjCreator_GetExportFrom_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		WavefrontObjCreator_GetExportFrom_original = (pfWavefrontObjCreator_GetExportFrom)GetProcAddress(m, WavefrontObjCreator_GetExportFrom_FULLNAME);
		if(WavefrontObjCreator_GetExportFrom_original == NULL)
		{
			ERRLOG("GetProcAddress(" WavefrontObjCreator_GetExportFrom_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" WavefrontObjCreator_GetExportFrom_FULLNAME ") returns %p)", WavefrontObjCreator_GetExportFrom_original);
		}
	}
	CALL_NEXT(ret = WavefrontObjCreator_GetExportFrom_original(obj));
	CALL_END("WavefrontObjCreator_GetExportFrom(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	BaseCreator_GetInputFileName
	public: char const * __ptr64 __cdecl UGS::Dex::BaseCreator::GetInputFileName(void)const __ptr64
*/
static pfBaseCreator_GetInputFileName BaseCreator_GetInputFileName_original;
char const * BaseCreator_GetInputFileName(CPP_PTR obj)
{
	char const * ret = NULL;
	if(BaseCreator_GetInputFileName_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		BaseCreator_GetInputFileName_original = (pfBaseCreator_GetInputFileName)GetProcAddress(m, BaseCreator_GetInputFileName_FULLNAME);
		if(BaseCreator_GetInputFileName_original == NULL)
		{
			ERRLOG("GetProcAddress(" BaseCreator_GetInputFileName_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" BaseCreator_GetInputFileName_FULLNAME ") returns %p)", BaseCreator_GetInputFileName_original);
		}
	}
	CALL_NEXT(ret = BaseCreator_GetInputFileName_original(obj));
	CALL_END("BaseCreator_GetInputFileName(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	STLCreator_GetObjectList
	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::Dex::STLCreator::GetObjectList(void)const __ptr64
*/
static pfSTLCreator_GetObjectList STLCreator_GetObjectList_original;
SELECT_OBJECT_list_PTR STLCreator_GetObjectList(CPP_PTR obj)
{
	SELECT_OBJECT_list_PTR ret = NULL;
	if(STLCreator_GetObjectList_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		STLCreator_GetObjectList_original = (pfSTLCreator_GetObjectList)GetProcAddress(m, STLCreator_GetObjectList_FULLNAME);
		if(STLCreator_GetObjectList_original == NULL)
		{
			ERRLOG("GetProcAddress(" STLCreator_GetObjectList_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" STLCreator_GetObjectList_FULLNAME ") returns %p)", STLCreator_GetObjectList_original);
		}
	}
	CALL_NEXT(ret = STLCreator_GetObjectList_original(obj));
	CALL_END("STLCreator_GetObjectList(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	DxfdwgCreator_GetObjectSelector
	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::DxfdwgCreator::GetObjectSelector(void)const __ptr64
*/
static pfDxfdwgCreator_GetObjectSelector DxfdwgCreator_GetObjectSelector_original;
ObjectSelector_PTR DxfdwgCreator_GetObjectSelector(DxfdwgCreator_PTR obj)
{
	ObjectSelector_PTR ret = NULL;
	if(DxfdwgCreator_GetObjectSelector_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		DxfdwgCreator_GetObjectSelector_original = (pfDxfdwgCreator_GetObjectSelector)GetProcAddress(m, DxfdwgCreator_GetObjectSelector_FULLNAME);
		if(DxfdwgCreator_GetObjectSelector_original == NULL)
		{
			ERRLOG("GetProcAddress(" DxfdwgCreator_GetObjectSelector_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" DxfdwgCreator_GetObjectSelector_FULLNAME ") returns %p)", DxfdwgCreator_GetObjectSelector_original);
		}
	}
	CALL_NEXT(ret = DxfdwgCreator_GetObjectSelector_original(obj));
	CALL_END("DxfdwgCreator_GetObjectSelector(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	IgesCreator_GetObjectSelector
	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::IgesCreator::GetObjectSelector(void)const __ptr64
*/
static pfIgesCreator_GetObjectSelector IgesCreator_GetObjectSelector_original;
ObjectSelector_PTR IgesCreator_GetObjectSelector(IgesCreator_PTR obj)
{
	ObjectSelector_PTR ret = NULL;
	if(IgesCreator_GetObjectSelector_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		IgesCreator_GetObjectSelector_original = (pfIgesCreator_GetObjectSelector)GetProcAddress(m, IgesCreator_GetObjectSelector_FULLNAME);
		if(IgesCreator_GetObjectSelector_original == NULL)
		{
			ERRLOG("GetProcAddress(" IgesCreator_GetObjectSelector_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" IgesCreator_GetObjectSelector_FULLNAME ") returns %p)", IgesCreator_GetObjectSelector_original);
		}
	}
	CALL_NEXT(ret = IgesCreator_GetObjectSelector_original(obj));
	CALL_END("IgesCreator_GetObjectSelector(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	NXTo2dCreator_GetObjectSelector
	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::NXTo2dCreator::GetObjectSelector(void)const __ptr64
*/
static pfNXTo2dCreator_GetObjectSelector NXTo2dCreator_GetObjectSelector_original;
ObjectSelector_PTR NXTo2dCreator_GetObjectSelector(NXTo2dCreator_PTR obj)
{
	ObjectSelector_PTR ret = NULL;
	if(NXTo2dCreator_GetObjectSelector_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		NXTo2dCreator_GetObjectSelector_original = (pfNXTo2dCreator_GetObjectSelector)GetProcAddress(m, NXTo2dCreator_GetObjectSelector_FULLNAME);
		if(NXTo2dCreator_GetObjectSelector_original == NULL)
		{
			ERRLOG("GetProcAddress(" NXTo2dCreator_GetObjectSelector_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" NXTo2dCreator_GetObjectSelector_FULLNAME ") returns %p)", NXTo2dCreator_GetObjectSelector_original);
		}
	}
	CALL_NEXT(ret = NXTo2dCreator_GetObjectSelector_original(obj));
	CALL_END("NXTo2dCreator_GetObjectSelector(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	StepCreator_GetObjectSelector
	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::StepCreator::GetObjectSelector(void)const __ptr64
*/
static pfStepCreator_GetObjectSelector StepCreator_GetObjectSelector_original;
ObjectSelector_PTR StepCreator_GetObjectSelector(StepCreator_PTR obj)
{
	ObjectSelector_PTR ret = NULL;
	if(StepCreator_GetObjectSelector_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		StepCreator_GetObjectSelector_original = (pfStepCreator_GetObjectSelector)GetProcAddress(m, StepCreator_GetObjectSelector_FULLNAME);
		if(StepCreator_GetObjectSelector_original == NULL)
		{
			ERRLOG("GetProcAddress(" StepCreator_GetObjectSelector_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" StepCreator_GetObjectSelector_FULLNAME ") returns %p)", StepCreator_GetObjectSelector_original);
		}
	}
	CALL_NEXT(ret = StepCreator_GetObjectSelector_original(obj));
	CALL_END("StepCreator_GetObjectSelector(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	WavefrontObjCreator_GetObjectSelector
	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::WavefrontObjCreator::GetObjectSelector(void)const __ptr64
*/
static pfWavefrontObjCreator_GetObjectSelector WavefrontObjCreator_GetObjectSelector_original;
ObjectSelector_PTR WavefrontObjCreator_GetObjectSelector(CPP_PTR obj)
{
	ObjectSelector_PTR ret = NULL;
	if(WavefrontObjCreator_GetObjectSelector_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		WavefrontObjCreator_GetObjectSelector_original = (pfWavefrontObjCreator_GetObjectSelector)GetProcAddress(m, WavefrontObjCreator_GetObjectSelector_FULLNAME);
		if(WavefrontObjCreator_GetObjectSelector_original == NULL)
		{
			ERRLOG("GetProcAddress(" WavefrontObjCreator_GetObjectSelector_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" WavefrontObjCreator_GetObjectSelector_FULLNAME ") returns %p)", WavefrontObjCreator_GetObjectSelector_original);
		}
	}
	CALL_NEXT(ret = WavefrontObjCreator_GetObjectSelector_original(obj));
	CALL_END("WavefrontObjCreator_GetObjectSelector(obj='%p') returns '%p'", obj, ret);
	return ret;
}

/*
	BaseCreator_GetOutputFile
	public: char const * __ptr64 __cdecl UGS::Dex::BaseCreator::GetOutputFile(void)const __ptr64
*/
static pfBaseCreator_GetOutputFile BaseCreator_GetOutputFile_original;
char const * BaseCreator_GetOutputFile(CPP_PTR obj)
{
	char const * ret = NULL;
	if(BaseCreator_GetOutputFile_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		BaseCreator_GetOutputFile_original = (pfBaseCreator_GetOutputFile)GetProcAddress(m, BaseCreator_GetOutputFile_FULLNAME);
		if(BaseCreator_GetOutputFile_original == NULL)
		{
			ERRLOG("GetProcAddress(" BaseCreator_GetOutputFile_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" BaseCreator_GetOutputFile_FULLNAME ") returns %p)", BaseCreator_GetOutputFile_original);
		}
	}
	CALL_NEXT(ret = BaseCreator_GetOutputFile_original(obj));
	CALL_END("BaseCreator_GetOutputFile(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	DxfdwgCreator_GetOutputFileType
	public: enum JA_DXFDWG_CREATOR_OutputFileTypeOption __cdecl UGS::Dex::DxfdwgCreator::GetOutputFileType(void)const __ptr64
*/
static pfDxfdwgCreator_GetOutputFileType DxfdwgCreator_GetOutputFileType_original;
CPP_ENUM DxfdwgCreator_GetOutputFileType(DxfdwgCreator_PTR obj)
{
	CPP_ENUM ret = 0;
	if(DxfdwgCreator_GetOutputFileType_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		DxfdwgCreator_GetOutputFileType_original = (pfDxfdwgCreator_GetOutputFileType)GetProcAddress(m, DxfdwgCreator_GetOutputFileType_FULLNAME);
		if(DxfdwgCreator_GetOutputFileType_original == NULL)
		{
			ERRLOG("GetProcAddress(" DxfdwgCreator_GetOutputFileType_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" DxfdwgCreator_GetOutputFileType_FULLNAME ") returns %p)", DxfdwgCreator_GetOutputFileType_original);
		}
	}
	CALL_NEXT(ret = DxfdwgCreator_GetOutputFileType_original(obj));
	CALL_END("DxfdwgCreator_GetOutputFileType(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	NXTo2dCreator_GetOutputFileType
	public: enum JA_NXTO2D_CREATOR_OutputAsOption __cdecl UGS::Dex::NXTo2dCreator::GetOutputFileType(void)const __ptr64
*/
static pfNXTo2dCreator_GetOutputFileType NXTo2dCreator_GetOutputFileType_original;
CPP_ENUM NXTo2dCreator_GetOutputFileType(NXTo2dCreator_PTR obj)
{
	CPP_ENUM ret = 0;
	if(NXTo2dCreator_GetOutputFileType_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		NXTo2dCreator_GetOutputFileType_original = (pfNXTo2dCreator_GetOutputFileType)GetProcAddress(m, NXTo2dCreator_GetOutputFileType_FULLNAME);
		if(NXTo2dCreator_GetOutputFileType_original == NULL)
		{
			ERRLOG("GetProcAddress(" NXTo2dCreator_GetOutputFileType_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" NXTo2dCreator_GetOutputFileType_FULLNAME ") returns %p)", NXTo2dCreator_GetOutputFileType_original);
		}
	}
	CALL_NEXT(ret = NXTo2dCreator_GetOutputFileType_original(obj));
	CALL_END("NXTo2dCreator_GetOutputFileType(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	ObjectSelector_GetSelectionComp
	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::Dex::ObjectSelector::GetSelectionComp(void)const __ptr64
*/
static pfObjectSelector_GetSelectionComp ObjectSelector_GetSelectionComp_original;
SELECT_OBJECT_list_PTR ObjectSelector_GetSelectionComp(ObjectSelector_PTR obj)
{
	SELECT_OBJECT_list_PTR ret = NULL;
	if(ObjectSelector_GetSelectionComp_original == NULL)
	{
		auto m = GetModuleHandle("libjadex.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libjadex.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		ObjectSelector_GetSelectionComp_original = (pfObjectSelector_GetSelectionComp)GetProcAddress(m, ObjectSelector_GetSelectionComp_FULLNAME);
		if(ObjectSelector_GetSelectionComp_original == NULL)
		{
			ERRLOG("GetProcAddress(" ObjectSelector_GetSelectionComp_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" ObjectSelector_GetSelectionComp_FULLNAME ") returns %p)", ObjectSelector_GetSelectionComp_original);
		}
	}
	CALL_NEXT(ret = ObjectSelector_GetSelectionComp_original(obj));
	CALL_END("ObjectSelector_GetSelectionComp(obj='%p') returns '%p'", obj, ret);
	return ret;
}


