/*
	Library:	libjadex.dll
	Version:	10.0.0.24
*/
#include <uf_ugmgr.h>
#include <uf_part.h>
#include <hook/hook_defs.h>
#include <hook/libpart.h>
#include <hook/libjadex.h>
#include <hook/libhidl.h>
#include <hook/hook_file_closed.h>
#include <Shlwapi.h>
#include <utils_windows.h>
#include <NXRMX/nx_contexts.h>
#include <NXRMX/nx_utils.h>
#include <utils_rmc.h>
#include <NxlUtils/nxlrunner.exe.hpp>

typedef enum export_from_e
{
	from_displayed_part = 0,
	from_existing_part = 1,
	from_unknown = 2
} export_from_t;

typedef struct export_context_s
{
	const char *inputFile;
	const char *outputFile;
	int last_spawned_pid;
	export_from_t exportFrom;
	int nSelectedObjs;
	tag_t *selectedObjs;
	pfFileClosedHandler fileClosedHandler;
	BOOL exportAssemblyFromExisting;
}export_context_t;

//handle created dwg file
static const char *s_rootDwgFile = NULL;
static int s_ndwgs = 0;
static void HandleCreatedDWGFile(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if(isWrite
		&& string_ends_with(file, ".dwg", FALSE))
	{
		s_ndwgs++;
		DBGLOG("FinishedWriting[%d]:'%s'(Size=%lld)", s_ndwgs, file, fileSize);
		if(s_rootDwgFile == NULL
			|| _stricmp(s_rootDwgFile, file) != 0)
		{
			nx_on_exported_fuzzy(file);
		}
	}
}

export_context_t before_export(const char *inputFile, const char *outputFile, int exportFrom, ObjectSelector_PTR objSelector)
{
	int nobjs = -1;
	tag_t *objs = NULL;
	pfFileClosedHandler handler = NULL;
	if(objSelector != NULL)
	{
		SELECT_OBJECT_list_PTR selectList = ObjectSelector_GetSelectionComp(objSelector);
		if(selectList != NULL)
		{
			int size = SELECT_OBJECT_list_ask_size(selectList);
			if(size > 0)
			{
				objs = SELECT_OBJECT_list_to_tag_array(selectList, &nobjs);
			}
		}
		//when exporting to .dwg files with following configuration
		//	ExportFrom=DisplayedPart, DataToExport=EntirePart, and the displayed part is modified
		//the export will be done in NX, not in a new created process-dxfdwg.exe
		//in this case, we need monitor the .dwg file creation in NX process
		if(exportFrom == from_displayed_part
			&& nobjs < 0
			//&& TODO:part file is modified
			&& string_ends_with(outputFile, ".dwg", FALSE))
		{
			auto parts = part_list_display_parts();
			for (auto p = parts.begin(); p != parts.end(); p++)
			{
				auto name = tag_to_name(*p);
				if (UF_PART_is_modified(*p))
				{
					DBGLOG("Part-%u('%s') is modified", *p, name.c_str());
					//reset
					s_rootDwgFile = outputFile;
					s_ndwgs = 0;
					//monitor the file creation
					register_FileClosedHandler(HandleCreatedDWGFile);
					handler = HandleCreatedDWGFile;
					break;
				}
				DBGLOG("Part-%u('%s') is NOT modified", *p, name.c_str());
			}
		}
	}
	export_context_t context = {inputFile, outputFile, SpawnContext::Previous().pid(), (export_from_t)exportFrom, nobjs, objs, handler, FALSE};
	nx_begin_export();
	return context;
}
void after_export_stp(export_context_t context) {
	NxlPath outfile(context.outputFile);
	if (ASSERT_FILE_EXISTS(outfile) == false) {
		//bug fix 55146:workaround
		//somehow NX didn't rename xxx.%%% to xxx.stp
		NxlPath tmpfile = outfile.WithoutExtension().Append(".%%%");
		if (ASSERT_FILE_EXISTS(tmpfile)) {
			if (MoveFileW(tmpfile.wstr(), outfile.wstr())) {
				DBGLOG("Renamed '%s' to '%s'", tmpfile.tstr(), outfile.tstr());
			}
			else
			{
				DBGLOG("Failed(%#X) to rename '%s'", GetLastError(), tmpfile.tstr());
			}
		}
	}
}
void after_export(export_context_t context)
{
	std::string inputFilePath;
	char *internalName = NULL;
	DBGLOG("CreatorCommit:InputFile='%s' OutputFile='%s' ExportFrom=%d SelectedObject=%d PreviousSpawnPID=%d LastSpawnPID=%d Injection=%d",
		context.inputFile, context.outputFile, context.exportFrom, context.nSelectedObjs, context.last_spawned_pid, SpawnContext::Previous().pid(), SpawnContext::Previous().injected());
	//remove hook for monitoring dwg file creation
	unregister_FileClosedHandler(context.fileClosedHandler);

	if(SpawnContext::Previous().pid() != context.last_spawned_pid
		&& SpawnContext::Previous().injected())
	{
		//new process is spawned for exporting, the injected minirmx.dll will enforce the protection
		DBGLOG("New Process(%d) is created and injected for enforcement", SpawnContext::Previous().pid());
		nx_end_export();
		return;
	}
	if(nx_isugmr()
		&& NX_EVAL(UF_UGMGR_convert_name_from_cli(context.inputFile, &internalName)))
	{
		inputFilePath = name_to_file(internalName);
		if(inputFilePath.empty())
		{
			//in case part file is not returned, try to search from contexts
			inputFilePath = PartContext::SearchFileByName(internalName);
		}
		DBGLOG("CliName='%s' InternalName='%s' FilePath='%s'", context.inputFile, internalName, inputFilePath.c_str());
		UF_free(internalName);
	}
	else
	{
		inputFilePath = context.inputFile;
	}
	if(SpawnContext::Previous().pid() != context.last_spawned_pid
		&& !SpawnContext::Previous().injected()
		&& file_exist(inputFilePath.c_str()))
	{
		//if new process is created but injected
		SpawnContext::Previous().wait_finish();
	}
	if(file_exist(context.outputFile))
	{
		if(context.exportFrom == from_displayed_part)
		{
			//exporting from current displayed part
			if(context.nSelectedObjs >= 0)
			{
				//Exporting selected object from displayed part
				nx_on_exported_objects(tags_make(context.selectedObjs, context.nSelectedObjs), context.outputFile);
			}
			else
			{
				if(s_ndwgs > 1)
				{
					//if multiple dwg files are created, mean the input outputFile contains the root assembly only
					nx_on_exported_1to1(UF_PART_ask_display_part(), context.outputFile);//root assembly is exported to this file
				}
				else
				{
					//exporting entire of displayed part
					nx_on_exported_all(context.outputFile);
				}
			}
		}
		else
		{
			tag_t rootPart = UF_PART_ask_part_tag(internalName);
			if (context.exportAssemblyFromExisting
				&& rootPart != NULLTAG)
			{
				//use 
				auto parts = assembly_list_parts(rootPart);
				nx_on_exported_parts(parts, context.outputFile);
			}
			else
			{
				//NOTE:based on current observation, exporing existing part should be done in external application
				//this routin should only be hit when injection failed
				nx_on_file_copied(inputFilePath.c_str(), context.outputFile, true);
			}
		}
	}
	else
	{
		ERRLOG("FileNotFound:'%s'", context.outputFile);
	}
	nx_end_export();
}
/*
	DxfdwgCreator_commit
	public: virtual unsigned int __cdecl UGS::Dex::DxfdwgCreator::commit(void) __ptr64
*/
static pfDxfdwgCreator_commit DxfdwgCreator_commit_original;
static pfDxfdwgCreator_commit DxfdwgCreator_commit_next;
static unsigned int DxfdwgCreator_commit_hooked(DxfdwgCreator_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = {0};
	CALL_START("DxfdwgCreator_commit(obj='%p')", obj);
	context = before_export(
#if NX_MAJOR_VER >=9 && NX_MAJOR_VER <=12
		DxfdwgCreator_GetInputFileName(obj),
		DxfdwgCreator_GetOutputFile(obj),
#else
		BaseCreator_GetInputFileName(obj),
		BaseCreator_GetOutputFile(obj),
#endif
		DxfdwgCreator_GetExportFrom(obj),
		DxfdwgCreator_GetObjectSelector(obj));
	CALL_NEXT(ret = DxfdwgCreator_commit_next(obj));
	after_export(context);
	CALL_END("DxfdwgCreator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

/*
	IgesCreator_commit
	public: virtual unsigned int __cdecl UGS::Dex::IgesCreator::commit(void) __ptr64
*/
static pfIgesCreator_commit IgesCreator_commit_original;
static pfIgesCreator_commit IgesCreator_commit_next;
static unsigned int IgesCreator_commit_hooked(IgesCreator_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = {0};
	CALL_START("IgesCreator_commit(obj='%p')", obj);
	context = before_export(
#if NX_MAJOR_VER >=9 && NX_MAJOR_VER <=12
		IgesCreator_GetInputFileName(obj),
		IgesCreator_GetOutputFile(obj),
#else
		BaseCreator_GetInputFileName(obj),
		BaseCreator_GetOutputFile(obj),
#endif
		IgesCreator_GetExportFrom(obj),
		IgesCreator_GetObjectSelector(obj));
	CALL_NEXT(ret = IgesCreator_commit_next(obj));
	after_export(context);
	CALL_END("IgesCreator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

/*
	NXTo2dCreator_commit
	public: virtual unsigned int __cdecl UGS::Dex::NXTo2dCreator::commit(void) __ptr64
*/
static pfNXTo2dCreator_commit NXTo2dCreator_commit_original;
static pfNXTo2dCreator_commit NXTo2dCreator_commit_next;
static unsigned int NXTo2dCreator_commit_hooked(NXTo2dCreator_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = {0};
	CALL_START("NXTo2dCreator_commit(obj='%p')", obj);
	context = before_export(
#if NX_MAJOR_VER >=9 && NX_MAJOR_VER <=12
		NXTo2dCreator_GetInputFile(obj),
		NXTo2dCreator_GetOutputFile(obj),
#else
		BaseCreator_GetInputFileName(obj),
		BaseCreator_GetOutputFile(obj),
#endif
		NXTo2dCreator_GetExportFrom(obj),
		NXTo2dCreator_GetObjectSelector(obj));
	CALL_NEXT(ret = NXTo2dCreator_commit_next(obj));
	after_export(context);
	CALL_END("NXTo2dCreator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

#if NX_MAJOR_VER == 12
/*
	Step203Creator_commit
	public: virtual unsigned int __cdecl UGS::Dex::Step203Creator::commit(void) __ptr64
*/
static pfStep203Creator_commit Step203Creator_commit_original;
static pfStep203Creator_commit Step203Creator_commit_next;
static unsigned int Step203Creator_commit_hooked(Step203Creator_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = {0};
	CALL_START("Step203Creator_commit(obj='%p')", obj);
	context = before_export(
		Step203Creator_GetInputFileName(obj),
		Step203Creator_GetOutputFile(obj),
		Step203Creator_GetExportFrom(obj),
		Step203Creator_GetObjectSelector(obj));
	CALL_NEXT(ret = Step203Creator_commit_next(obj));
	after_export_stp(context);
	after_export(context);
	CALL_END("Step203Creator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

/*
	Step214Creator_commit
	public: virtual unsigned int __cdecl UGS::Dex::Step214Creator::commit(void) __ptr64
*/
static pfStep214Creator_commit Step214Creator_commit_original;
static pfStep214Creator_commit Step214Creator_commit_next;
static unsigned int Step214Creator_commit_hooked(Step214Creator_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = {0};
	CALL_START("Step214Creator_commit(obj='%p')", obj);
	context = before_export(
		Step214Creator_GetInputFileName(obj),
		Step214Creator_GetOutputFile(obj),
		Step214Creator_GetExportFrom(obj),
		Step214Creator_GetObjectSelector(obj));
	CALL_NEXT(ret = Step214Creator_commit_next(obj));
	after_export_stp(context);
	after_export(context);
	CALL_END("Step214Creator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

#endif
/*
	StepCreator_commit
	public: virtual unsigned int __cdecl UGS::Dex::StepCreator::commit(void) __ptr64
*/
static pfStepCreator_commit StepCreator_commit_original;
static pfStepCreator_commit StepCreator_commit_next;
static unsigned int StepCreator_commit_hooked(StepCreator_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = {0};
	CALL_START("StepCreator_commit(obj='%p')", obj);
	context = before_export(
#if NX_MAJOR_VER >=9 && NX_MAJOR_VER <=12
		StepCreator_GetInputFileName(obj),
		StepCreator_GetOutputFile(obj),
#else
		BaseCreator_GetInputFileName(obj),
		BaseCreator_GetOutputFile(obj),
#endif
		StepCreator_GetExportFrom(obj),
		StepCreator_GetObjectSelector(obj));
	CALL_NEXT(ret = StepCreator_commit_next(obj));
	after_export_stp(context);
	after_export(context);
	CALL_END("StepCreator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

#if NX_MAJOR_VER>=1872
/*
WavefrontObjCreator_commit
public: virtual unsigned int __cdecl UGS::Dex::WavefrontObjCreator::commit(void) __ptr64
*/
static pfWavefrontObjCreator_commit WavefrontObjCreator_commit_original;
static pfWavefrontObjCreator_commit WavefrontObjCreator_commit_next;
static unsigned int WavefrontObjCreator_commit_hooked(CPP_PTR obj)
{
	unsigned int ret = 0;
	export_context_t context = { 0 };
	std::string partFile;
	tag_t partTag = NULL_TAG;
	CALL_START("WavefrontObjCreator_commit(obj='%p')", obj);
	context = before_export(
#if NX_MAJOR_VER >= 1953
		BaseCreator_GetInputFileName(obj),
		BaseCreator_GetOutputFile(obj),
#else
		WavefrontObjCreator_GetInputFile(obj),
		WavefrontObjCreator_GetOutputFile(obj),
#endif
		WavefrontObjCreator_GetExportFrom(obj),
		WavefrontObjCreator_GetObjectSelector(obj));
	context.exportAssemblyFromExisting = TRUE;
	if (context.exportFrom == from_existing_part
		&& nx_isugmr()) {
		//workaround fix for bug-60322
		//in managed mode, when export from existing part, the export will failed with exception
		//after failed, the opened pat will be closed
		//at this time, if the part has been loaded into part context, it will be removed in part_event_handler
		//but this part was reopened again without trigger part_event_handler
		//we need resume the status here
		auto inputFile = context.inputFile;
		partTag = UF_PART_ask_part_tag(context.inputFile);
		if (partTag != NULLTAG)
		{
			//this part has been loaded
			auto pContext = PartContext::FindByTag(partTag);
			if (pContext != nullptr) {
				NXDBG("Part has been loaded:" CONTEXT_FMT, CONTEXT_FMT_ARGS(pContext));
				//this protected context need to be cached
				partFile = pContext->full_path();
			}
		}
	}
	CALL_NEXT(ret = WavefrontObjCreator_commit_next(obj));
	if (context.exportFrom == from_existing_part
		&& nx_isugmr()
		&& partTag
		&& !partFile.empty()) {
		auto pContext = PartContext::FindByTag(partTag);
		if (pContext == nullptr)
		{
			PartContext::LoadContext(partTag, tag_to_name(partTag), NxlPath(partFile.c_str()));
			//notify workpart changed event
			overlay_force_refresh();
		}
	}
	after_export(context);
	nx_on_exported_associated(context.outputFile, ".mtl");
	CALL_END("WavefrontObjCreator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}
#endif

#if NX_MAJOR_VER >= 1872
/*
ExportNXToSTL
void __cdecl ExportNXToSTL(struct STLExportParameters const & __ptr64,unsigned int * __ptr64,int * __ptr64,char const * __ptr64,int * __ptr64,char const * __ptr64,struct STLExportErrorEntityStructure * __ptr64)
*/
static pfExportNXToSTL ExportNXToSTL_original;
static pfExportNXToSTL ExportNXToSTL_next;
static void ExportNXToSTL_hooked(CPP_PTR p1, unsigned int * p2, int * p3, char const * p4, int * p5, char const * p6, CPP_PTR p7)
{
	CALL_START("ExportNXToSTL(p1='%p', *p2='%u', *p3='%d', p4='%s', *p5='%d', p6='%s', p7='%p')", p1, *p2, *p3, p4, *p5, p6, p7);
	const char *outputFile = p4;
	std::vector<tag_t> selectedObjs;
	int nobjs = *p3;
	tag_t *objs = p2;
	if (nobjs > 0 && objs != nullptr)
	{
		for (int i = 0; i < nobjs; i++)
		{
			selectedObjs.push_back(objs[i]);
		}
	}
	DBGLOG("Exporting %d objects into file-'%s'", selectedObjs.size(), outputFile);
	CALL_NEXT(ExportNXToSTL_next(p1, p2, p3, p4, p5, p6, p7));
	//
	nx_on_exported_objects(selectedObjs, outputFile);
	CALL_END("ExportNXToSTL(p1='%p', *p2='%u', *p3='%d', p4='%s', *p5='%d', p6='%s', p7='%p')", p1, *p2, *p3, p4, *p5, p6, p7);
}
#endif
#if NX_MAJOR_VER >=11
/*
STLCreator_commit
public: virtual unsigned int __cdecl UGS::Dex::STLCreator::commit(void) __ptr64
*/
static pfSTLCreator_commit STLCreator_commit_original;
static pfSTLCreator_commit STLCreator_commit_next;
static unsigned int STLCreator_commit_hooked(CPP_PTR obj)
{
	unsigned int ret = 0;	export_context_t context = { 0 };
	CALL_START("STLCreator_commit(obj='%p')", obj);
#if NX_MAJOR_VER <= 12
	const char *outputFile = STLCreator_GetOutputFile(obj);
#elif NX_MAJOR_VER >= 1872
	const char* outputFile = BaseCreator_GetOutputFile(obj);
#endif
	std::vector<tag_t> selectedObjs;
	SELECT_OBJECT_list_PTR list = STLCreator_GetObjectList(obj);
	if (list != NULL)
	{
	int nobjs = SELECT_OBJECT_list_ask_size(list);
		if (nobjs > 0)
		{
			tag_t *objs = SELECT_OBJECT_list_to_tag_array(list, &nobjs);
			for (int i = 0; i < nobjs; i++)
			{
				selectedObjs.push_back(objs[i]);
			}
		}
	}
	DBGLOG("Exporting %d objects into file-'%s'", selectedObjs.size(), outputFile);
	CALL_NEXT(ret = STLCreator_commit_next(obj));
	//
	nx_on_exported_objects(selectedObjs, outputFile);
	CALL_END("STLCreator_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}
#endif

#if NX_MAJOR_VER >= 10
/*
ZipFile_AddEntryFile
public: void __cdecl UGS::System::CFI::ZipFile::AddEntryFile(class UGS::UString const & __ptr64,class UGS::UString const & __ptr64) __ptr64
*/
static pfZipFile_AddEntryFile ZipFile_AddEntryFile_original;
static pfZipFile_AddEntryFile ZipFile_AddEntryFile_next;
static void ZipFile_AddEntryFile_hooked(CPP_PTR obj, UString_REF p1, UString_REF p2)
{
	bool isProtected = false;
	UString_REF entryName = p1;
	NxlPath file(UString_utf8_str(p2));
		DWORD pid = GetCurrentProcessId();
	CALL_START("ZipFile_AddEntryFile(obj='%p', UString_utf8_str(p1)='%s', UString_utf8_str(p2)='%s')", obj, UString_utf8_str(p1), file.chars());

	if (g_rpmSession->CheckFileProtection(file.wchars(), &isProtected) == 0
		&& isProtected)
	{
		DBGLOG("File-'%s' is protected!Revoke TRUSTAPP during zipping...", file.chars());
		g_rpmSession->SetTrustedProcess(pid, false);
	}

	if(file_is_protected(file.chars()))
	{
		entryName = UString_append_3(p1, ".nxl");
	}
	CALL_NEXT(ZipFile_AddEntryFile_next(obj, entryName, p2));

	//restore trust app
	NxlRunner nxlrunner;
	nxlrunner.SetTrustApp(pid);
	CALL_END("ZipFile_AddEntryFile(obj='%p', UString_utf8_str(p1)='%s', UString_utf8_str(p2)='%s')", obj, UString_utf8_str(entryName), file.chars());
}
#endif

#if NX_MAJOR_VER >= 1872
/*
BaseCreator_UploadFileToTeamcenter
protected: void __cdecl UGS::Dex::BaseCreator::UploadFileToTeamcenter(void) __ptr64
*/
static pfBaseCreator_UploadFileToTeamcenter BaseCreator_UploadFileToTeamcenter_original;
static pfBaseCreator_UploadFileToTeamcenter BaseCreator_UploadFileToTeamcenter_next;
static void BaseCreator_UploadFileToTeamcenter_hooked(CPP_PTR obj)
{
	CALL_START("BaseCreator_UploadFileToTeamcenter(obj='%p')", obj);
	const char *outputFile = BaseCreator_GetOutputFile(obj);
	bool isprotected = false;
	NxlPath opath(outputFile);
	if (g_rpmSession->CheckFileProtection(opath.wchars(), &isprotected) != 0
		|| !isprotected)
	{
		const char *cliName = BaseCreator_GetInputFileName(obj);
		char *internalName = NULL;
		if (cliName != NULL && strlen(cliName) > 0
			&& NX_EVAL(UF_UGMGR_convert_name_from_cli(cliName, &internalName)))
		{
			DBGLOG("CliName='%s' InternalName='%s'", cliName, internalName);
			tag_t rootPart = UF_PART_ask_part_tag(internalName);
			if (rootPart != NULL)
			{
				if (isprotected = nx_on_exported_parts(assembly_list_parts(rootPart), outputFile))
				{
					if (opath.HasExtension(".obj")) {
						//protect associated mtl file
						nx_on_exported_associated(outputFile, ".mtl");
					}
				}
			}
		}
		UF_free(internalName);
	}

	if (isprotected) {
		//INSTALL hook on UGS::System::CFI::ZipFile::AddEntryFile
		HOOK_API("libsyss.dll", ZipFile_AddEntryFile);
	}
	CALL_NEXT(BaseCreator_UploadFileToTeamcenter_next(obj));

	UNHOOK(ZipFile_AddEntryFile);
	CALL_END("BaseCreator_UploadFileToTeamcenter(obj='%p')", obj);
}
#endif

#define LIB_JADEX_DLL "libjadex.dll"

void libjadex_hook()
{
	auto nxver = nx_get_major_version();
	//File->Export->AUTOCAD DXF/DWG
	HOOK_API(LIB_JADEX_DLL, DxfdwgCreator_commit);

	//File->Export->IGES
	HOOK_API(LIB_JADEX_DLL, IgesCreator_commit);

	//File->Export->2D Exchange
	HOOK_API(LIB_JADEX_DLL, NXTo2dCreator_commit);

#if NX_MAJOR_VER <= 12
		//File->Export->STEP203
		HOOK_API(LIB_JADEX_DLL, Step203Creator_commit);

		//File->Export->STEP214
		HOOK_API(LIB_JADEX_DLL, Step214Creator_commit);
#elif NX_MAJOR_VER >= 1872
		//HOOK_API("libpart.dll", PART_export_part_as_step_file1872);
		HOOK_API(LIB_JADEX_DLL, BaseCreator_UploadFileToTeamcenter);
		HOOK_API(LIB_JADEX_DLL, WavefrontObjCreator_commit);
#endif
#if NX_MAJOR_VER >= 11
		//bug fix 53590: when exporting an unsaved part file into step format,
		//NX 11 introduces a new class UGS::Dex::StepCreator to handle it 
		//while NX 9/10 use UGS::Dex::Step204Creator or Step213Creator
		HOOK_API(LIB_JADEX_DLL, StepCreator_commit);
		//Export STL support

#if NX_MAJOR_VER >= 1872
		if (nx_isugmr())
		{
			HOOK_API("libhidl.dll", ExportNXToSTL);
		}
		else
#endif
		{
			HOOK_API(LIB_JADEX_DLL, STLCreator_commit);
		}
#endif
}
