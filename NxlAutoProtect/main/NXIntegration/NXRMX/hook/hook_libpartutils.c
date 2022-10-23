/*
	Library:	libpartutils.dll
	Version:	10.0.0.24
*/
#include <hook/hook_defs.h>
#include <hook/libpartutils.h>
#include <NXRMX/nx_contexts.h>
#include <NXRMX/nx_utils.h>
#include <utils_string.h>
#include <uf_part.h>
#include <hook/hook_file_closed.h>
#include <NxlUtils/include/utils_windows.h>
#include <NxlUtils/Windows/handle.h>

/*
	ExportUtils_ExportSaveAndUnloadPart
	int __cdecl UGS::Part::ExportUtils::ExportSaveAndUnloadPart(unsigned int * __ptr64,bool)
*/
static pfExportUtils_ExportSaveAndUnloadPart ExportUtils_ExportSaveAndUnloadPart_original;
static pfExportUtils_ExportSaveAndUnloadPart ExportUtils_ExportSaveAndUnloadPart_next;
static int ExportUtils_ExportSaveAndUnloadPart_hooked(CPP_PTR obj, unsigned int * p1, BOOL p2)
{
	int ret = 0;
	std::string exportedFile;
	tag_t exportingPart = NULLTAG;
	std::vector<tag_t> exportingObjects;
	CALL_START("ExportUtils_ExportSaveAndUnloadPart(obj='%p', p1='%p', p2='%d')", obj, p1, p2);
	{
		int nParts = UF_PART_ask_num_parts();
		NXDBG("Loaded %d Parts before exporting", nParts);
		if(nParts > 0)
		{
			//TODO:use more reliable approach to get the exporting part tag
			exportingPart = UF_PART_ask_nth_part(0);
			exportingObjects = selection_ask_objects();
			exportedFile = tag_to_name(exportingPart);
		}
	}
	//if(nx_isugmr())
	{
		nx_on_exporting_objects(exportingObjects, exportingPart);
	}
	CALL_NEXT(ret = ExportUtils_ExportSaveAndUnloadPart_next(obj, p1, p2));
	/*if(!nx_isugmr()
		&& !exportedFile.empty())
	{
		if(exportingObjects.size() > 0)
		{
			nx_on_exported_objects(exportingObjects, exportedFile.c_str());
		}
		else
		{
			//WORKAROUND: unmanaged mode, no object selection is found, i think it's exporting a drawing sheet
			nx_on_exported_all(exportedFile.c_str());
		}
	}*/
	CALL_END("ExportUtils_ExportSaveAndUnloadPart(obj='%p', p1='%p', p2='%d') returns '%d'", obj, p1, p2, ret);
	return ret;
}

static std::string s_rootJtFile;
static int s_nSubJts = 0;
void HandleCreatedJtFile(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if (isWrite
		&& (string_ends_with(file, ".jt", FALSE)
			|| string_ends_with(file, ".stpx", FALSE)))
	{
		int nopened = 0;
		DBGLOG("FinishedWriting:'%s'(Size=%lld)", file, fileSize);
		//check if any other handle opened on this file
		NxlString filePathWithoutDrive(file + 2);
		std::vector<HANDLE> handles = GetHandlesFromProcess(GetCurrentProcessId());
		for (auto h = handles.begin(); h != handles.end(); h++) {
			std::wstring ntpath;
			if (GetNtPathFromHandle(*h, ntpath) == ERROR_SUCCESS) {
				if (wcswcs(ntpath.c_str(), filePathWithoutDrive.wchars()) != NULL) {
					DWORD grantAccess = 0;
					if (GetGrantAccessFromHandle(*h, &grantAccess) == ERROR_SUCCESS)
					{
						if (grantAccess & FILE_WRITE_DATA) {
							nopened++;
						}
					}
					DBGLOG("[%#X]'%S'(GrantAccess=%#X)", *h, ntpath.c_str(), grantAccess);
				}
				else
				{
					//TRACELOG("[%#X]'%S'", *h, ntpath.c_str());
				}
			}
		}
		DBGLOG("File-'%s' is still opened in %d handles", filePathWithoutDrive.tchars(), nopened);
		if (nopened > 0)
		{
			return;
		}
		if (nx_name_compare(s_rootJtFile.c_str(), file, NULL) == equal_full
			|| string_ends_with(file, ".stpx", FALSE))
		{
			if (s_nSubJts > 0)
			{
				//root file contains the root assembly file only
				nx_on_exported_1to1(UF_PART_ask_display_part(), file);
			}
			else
			{
				//the whole assembly is exported into one file
				nx_on_exported_all(file);
			}
		}
		else
		{
			s_nSubJts++;
			nx_on_exported_fuzzy(file);
		}
	}
}
class ExportJTEvent {
public:
	ExportJTEvent(const char *dir, const char *fileName):_folderPath(dir), _fileName(fileName) {
		_settransient = REG_get_dword(REG_ROOT_KEY, "SetExportFolderAsRPM", 0) > 0;
	}
	void OnBeforeExport()
	{
		if (_settransient)
		{
			//set the export folder as rpm dir
			transient_rpm_dir_add(_folderPath);
		}

		//pre-export
		s_rootJtFile = std::string(_folderPath) + "\\" + _fileName;
		s_nSubJts = 0;
		//monitor the file creation
		nx_begin_export();
		register_FileClosedHandler(HandleCreatedJtFile);
	}
	void OnAfterExport() {

		//post export
		unregister_FileClosedHandler(HandleCreatedJtFile);
		nx_end_export();
		//clean
		s_rootJtFile.clear();
		s_nSubJts = 0;
		if (_settransient)
		{
			//clear transient rpm dir
			transient_rpm_dir_clear();
		}
	}

private:
	bool _settransient;
	const char* _folderPath;
	const char* _fileName;
};
#if NX_MAJOR_VER <= 1980
/*
	UG_translate_assembly
	int __cdecl PVTRANS_UG_translate_assembly(unsigned int,char const * __ptr64,char const * __ptr64,char const * __ptr64,void * __ptr64)
*/
static pfUG_translate_assembly UG_translate_assembly_original;
static pfUG_translate_assembly UG_translate_assembly_next;
static int UG_translate_assembly_hooked(unsigned int p1, char const * p2, char const * p3, char const * p4, void * p5)
{
	int ret = 0;
	CALL_START("UG_translate_assembly(p1='%u', p2='%s', p3='%s', p4='%s', p5)", p1, p2, p3, p4);
	ExportJTEvent exportEvent(p2, p3);
	exportEvent.OnBeforeExport();
	CALL_NEXT(ret = UG_translate_assembly_next(p1, p2, p3, p4, p5));
	exportEvent.OnAfterExport();
	CALL_END("UG_translate_assembly(p1='%u', p2='%s', p3='%s', p4='%s', p5) returns '%d'", p1, p2, p3, p4, ret);
	return ret;
}
#elif NX_MAJOR_VER >= 2007
/*
	UG_translate_assembly
	int __cdecl PVTRANS_UG_translate_assembly(unsigned int,char const * __ptr64,char const * __ptr64,char const * __ptr64,void * __ptr64,struct PVTRANS_export_nonJT_options_s * __ptr64)
*/
static pfUG_translate_assembly UG_translate_assembly_original;
static pfUG_translate_assembly UG_translate_assembly_next;
static int UG_translate_assembly_hooked(unsigned int p1, char const* p2, char const* p3, char const* p4, void* p5, CPP_PTR p6)
{
	int ret = 0;
	CALL_START("UG_translate_assembly(p1='%u', p2='%s', p3='%s', p4='%s', p5=%p, p6=%p)", p1, p2, p3, p4, p5, p6);
	ExportJTEvent exportEvent(p2, p3);
	exportEvent.OnBeforeExport();
	CALL_NEXT(ret = UG_translate_assembly_next(p1, p2, p3, p4, p5, p6));
	exportEvent.OnAfterExport();
	CALL_END("UG_translate_assembly(p1='%u', p2='%s', p3='%s', p4='%s', p5=%p, p6=%p) returns '%d'", p1, p2, p3, p4, p5, p6, ret);
	return ret;
}
#endif

#if NX_MAJOR_VER <= 1980
/*
	PX_export
	void __cdecl PX_export(unsigned int * __ptr64,int,char * __ptr64,bool,bool,int)
*/
static pfPX_export PX_export_original;
static pfPX_export PX_export_next;
static void PX_export_hooked(unsigned int * p1, int p2, char * p3, BOOL p4, BOOL p5, int p6)
{
	CALL_START("PX_export(*p1='%u', p2='%d', p3='%s', p4='%d', p5='%d', p6='%d')", *p1, p2, p3, p4, p5, p6);
	CALL_NEXT(PX_export_next(p1, p2, p3, p4, p5, p6));
	nx_on_exported_objects(tags_make(p1, p2), p3);
	CALL_END("PX_export(*p1='%u', p2='%d', p3='%s', p4='%d', p5='%d', p6='%d')", *p1, p2, p3, p4, p5, p6);
}
#elif NX_MAJOR_VER >= 2007
/*
	PX_export
	void __cdecl PX_export(unsigned int * __ptr64,int,char * __ptr64,bool,bool,int,bool,bool)
*/
static pfPX_export PX_export_original;
static pfPX_export PX_export_next;
static void PX_export_hooked(unsigned int* p1, int p2, char* p3, BOOL p4, BOOL p5, int p6, bool p7, bool p8)
{
	CALL_START("PX_export(*p1='%u', p2='%d', p3='%s', p4='%d', p5='%d', p6='%d', p7=%d p8=%d)", *p1, p2, p3, p4, p5, p6, p7, p8);
	CALL_NEXT(PX_export_next(p1, p2, p3, p4, p5, p6, p7, p8));
	nx_on_exported_objects(tags_make(p1, p2), p3);
	CALL_END("PX_export(*p1='%u', p2='%d', p3='%s', p4='%d', p5='%d', p6='%d', p7=%d p8=%d)", *p1, p2, p3, p4, p5, p6, p7, p8);
}
#endif


#define LIB_PART_UTILS "libpartutils.dll"
void libpartutils_hook()
{
	//File->Export->Part
	HOOK_API(LIB_PART_UTILS, ExportUtils_ExportSaveAndUnloadPart);
	//File->Export->Parasolid
	HOOK_API(LIB_PART_UTILS, PX_export);
	//File->Export->JT
	HOOK_API(LIB_PART_UTILS, UG_translate_assembly);

}

