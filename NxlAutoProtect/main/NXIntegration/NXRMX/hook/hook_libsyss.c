/*
	Library:	libsyss.dll
	Version:	10.0.0.24
*/
#include <Windows.h>
#include <Shlwapi.h>
#include <utils_windows.h>
#include <hook/hook_defs.h>
#include <hook/libsyss.h>
#include <hook/libassy.h>
#include <NXRMX/nx_contexts.h>


/*
	CFI_copy_file_0
	int __cdecl CFI_copy_file(char const * __ptr64,char const * __ptr64,int,int,int,int,int * __ptr64)
*/
static pfCFI_copy_file_0 CFI_copy_file_0_original;
static pfCFI_copy_file_0 CFI_copy_file_0_next;
static int CFI_copy_file_0_hooked(char const * p1, char const * p2, int p3, int p4, int p5, int p6, int * p7)
{
	int ret = 0;
	CALL_START("CFI_copy_file_0(p1='%s', p2='%s', p3='%d', p4='%d', p5='%d', p6='%d', p7)", p1, p2, p3, p4, p5, p6);
	CALL_NEXT(ret = CFI_copy_file_0_next(p1, p2, p3, p4, p5, p6, p7));
	if(file_exist(p1))
	{
		nx_on_file_copied(p1, p2, false);
	}
	else
	{
		NXDBG("FileMoved:From='%s' To='%s'", p1, p2);
	}
	CALL_END("CFI_copy_file_0(p1='%s', p2='%s', p3='%d', p4='%d', p5='%d', p6='%d', p7) returns '%d'", p1, p2, p3, p4, p5, p6, ret);
	return ret;
}

/*
	CFI_spawn
	int __cdecl CFI_spawn(char const * __ptr64,struct CFI_spawn_options_s * __ptr64,int * __ptr64,int * __ptr64)
*/
static pfCFI_spawn CFI_spawn_original;
static pfCFI_spawn CFI_spawn_next;
static int CFI_spawn_hooked(char const * p1, CPP_PTR p2, int * p3, int * p4)
{
	int ret = 0;
	CALL_START("CFI_spawn(p1='%s', p2='%p', p3='%d', p4='%d')", p1, p2, *p3, *p4);
	SpawnContext::Current().OnBeforeSpawning(p1);
	CALL_NEXT(ret = CFI_spawn_next(p1, p2, p3, p4));
	SpawnContext::Current().OnAfterSpawn(*p3);
	CALL_END("CFI_spawn(p1='%s', p2='%p', p3='%d', p4='%d') returns '%d'", p1, p2, *p3, *p4, ret);
	return ret;
}

/*
	CFI_add_argument
	void __cdecl CFI_add_argument(char const * __ptr64)
*/
static pfCFI_add_argument CFI_add_argument_original;
static pfCFI_add_argument CFI_add_argument_next;
static void CFI_add_argument_hooked(char const * p1)
{
	CALL_START("CFI_add_argument(p1='%s')", p1);
	CALL_NEXT(CFI_add_argument_next(p1));
	CALL_END("CFI_add_argument(p1='%s')", p1);
}

/*
	CFI_add_filename_argument
	void __cdecl CFI_add_filename_argument(char const * __ptr64)
*/
static pfCFI_add_filename_argument CFI_add_filename_argument_original;
static pfCFI_add_filename_argument CFI_add_filename_argument_next;
static void CFI_add_filename_argument_hooked(char const * p1)
{
	CALL_START("CFI_add_filename_argument(p1='%s')", p1);
	auto revisedPath = SpawnContext::Current().OnAddingFileArgument(p1);
	CALL_NEXT(CFI_add_filename_argument_next(revisedPath));
	CALL_END("CFI_add_filename_argument(revisedPath='%s')", revisedPath);
}
/*
	CFI_add_filename_switch_argument
	void __cdecl CFI_add_filename_switch_argument(char const * __ptr64,char const * __ptr64)
*/
static pfCFI_add_filename_switch_argument CFI_add_filename_switch_argument_original;
static pfCFI_add_filename_switch_argument CFI_add_filename_switch_argument_next;
static void CFI_add_filename_switch_argument_hooked(char const * p1, char const * p2)
{
	CALL_START("CFI_add_filename_switch_argument(p1='%s', p2='%s')", p1, p2);
	CALL_NEXT(CFI_add_filename_switch_argument_next(p1, p2));
	CALL_END("CFI_add_filename_switch_argument(p1='%s', p2='%s')", p1, p2);
}
/*
	CFI_add_switch_argument
	void __cdecl CFI_add_switch_argument(char const * __ptr64,char const * __ptr64)
*/
static pfCFI_add_switch_argument CFI_add_switch_argument_original;
static pfCFI_add_switch_argument CFI_add_switch_argument_next;
static void CFI_add_switch_argument_hooked(char const * p1, char const * p2)
{
	CALL_START("CFI_add_switch_argument(p1='%s', p2='%s')", p1, p2);
	CALL_NEXT(CFI_add_switch_argument_next(p1, p2));
	CALL_END("CFI_add_switch_argument(p1='%s', p2='%s')", p1, p2);
}


/*
	PFEDIT_edit_part_file
	int __cdecl PFEDIT_edit_part_file(struct PFEDIT_options_s * __ptr64,char const * __ptr64,char const * __ptr64)
*/
static pfPFEDIT_edit_part_file PFEDIT_edit_part_file_original;
static pfPFEDIT_edit_part_file PFEDIT_edit_part_file_next;
static void PFEDIT_edit_part_file_hooked(CPP_PTR p1, char const * p2, char const * p3)
{
	CALL_START("PFEDIT_edit_part_file(p2='%s', p3='%s')", p2, p3);
	CALL_NEXT(PFEDIT_edit_part_file_next(p1, p2, p3));
	nx_on_file_copied(p2, p3, true);
	CALL_END("PFEDIT_edit_part_file(p2='%s', p3='%s')", p2, p3);
}

static BOOL libsyss_hooked = FALSE;
void libsyss_hook_internal(HMODULE module)
{
	if(!libsyss_hooked && module != NULL)
	{
		//INIT_API_CPP(UString_utf8_str, module);

		//file copy
		HOOK_CPP(CFI_copy_file_0, module);
		HOOK_CPP(CFI_spawn, module);
		HOOK_CPP(CFI_add_argument, module);
		HOOK_CPP(CFI_add_filename_argument, module);
		HOOK_CPP(CFI_add_filename_switch_argument, module);
		HOOK_CPP(CFI_add_switch_argument, module);

		//import into teamcenter
		HOOK_CPP(PFEDIT_edit_part_file, module);
		libsyss_hooked = TRUE;
	}
}

void libsyss_hook()
{
	libsyss_hook_internal(GetModuleHandle("libsyss.dll"));
}
static std::vector<NxlPath> s_partFiles;
NxlPath fix_path(const char* path)
{
	NxlPath file(path);
	if (string_ends_with(file.str(), ".nxl.prt", FALSE))
	{
		//remove last extension
		NxlPath revisedName = file.WithoutExtension();
		DBGLOG("FilePathRevised:'%s'", revisedName.tstr());
		if (std::find(s_partFiles.begin(), s_partFiles.end(), revisedName) == s_partFiles.end())
			s_partFiles.push_back(revisedName);
		return revisedName;
	}
	else if (!ASSERT_FILE_EXISTS(file))
	{
		//remove last extension
		NxlPath revisedName = file.WithoutExtension();
		if (ASSERT_FILE_EXISTS(revisedName))
		{
			DBGLOG("FilePathRevised:'%s'", revisedName.tstr());
			if (std::find(s_partFiles.begin(), s_partFiles.end(), revisedName) == s_partFiles.end())
				s_partFiles.push_back(revisedName);
			return revisedName;
		}
	}
	if (std::find(s_partFiles.begin(), s_partFiles.end(), file) == s_partFiles.end())
		s_partFiles.push_back(file);
	return file;
}

void ReportAssemblyException(const char *assemblyName) {
	std::stringstream ss;
	int nNoViewRight = 0;
	UException_PTR except = Exception_askLastException();
	if (except == NULL) return;
	//log basic exception information
	NXDBG("==>Type:%d", Exception_askType(except));
	const char* syslogMessage = Exception_askSyslogMessage(except);
	NXDBG("==>SyslogMessage:%s", syslogMessage);
	int code = Exception_askCode(except);
	NXDBG("==>Code:%d", code);
	ss << "Failed to load assembly-" << name_to_display(assemblyName) << "\n";
	ss << "Error:" << syslogMessage << "\n";
	ss << "Code:" << code << "\n";
	NXDBG("Trying to load %d files", s_partFiles.size());
	for (auto iFile = s_partFiles.begin(); iFile != s_partFiles.end(); iFile++)
	{
		bool hasViewRight = false;
		//report exception to user
		if (iFile->CheckFileExists()
			&& g_rpmSession->CheckFileRights(iFile->wchars(), "RIGHT_VIEW", hasViewRight) == 0
			&& !hasViewRight) {
			nNoViewRight++;
		}
	}
	if (nNoViewRight > 0) {
		ss << "Reason:You don't have View right on some of parts which is NextLabs protected\n";
	}
	nx_show_listing_window(ss.str().c_str());
}

void ReportException(const NxlPath &file) {
	char message[1024];
	bool hasViewRight = false;
	UException_PTR except = Exception_askLastException();
	if (except == NULL) return;
	//log basic exception information
	NXDBG("==>Type:%d", Exception_askType(except));
	const char* syslogMessage = Exception_askSyslogMessage(except);
	NXDBG("==>SyslogMessage:%s", syslogMessage);
	int code = Exception_askCode(except);
	NXDBG("==>Code:%d", code);
	//report exception to user
	if (file.CheckFileExists()
		&& g_rpmSession->CheckFileRights(file.wchars(), "RIGHT_VIEW", hasViewRight) == 0
		&& !hasViewRight) {
		sprintf(message, "You don't have view right on file-'%s'\n\n", file.str());
	}
	else
	{
		sprintf(message, "Failed to read file-'%s'\n\tError-'%s'\n\n", file.str(), syslogMessage);
	}
	nx_show_listing_window(message);
}

/*
CFI_is_storage_file
bool __cdecl CFI_is_storage_file(char const * __ptr64)
*/
static pfCFI_is_storage_file CFI_is_storage_file_original;
static pfCFI_is_storage_file CFI_is_storage_file_next;
static BOOL CFI_is_storage_file_hooked(char const * p1)
{
	BOOL ret = FALSE;
	CALL_START("CFI_is_storage_file(p1='%s')", p1);
	NxlPath file = fix_path(p1);
	CALL_NEXT(ret = CFI_is_storage_file_next(file.str()));
	CALL_END("CFI_is_storage_file(p1='%s') returns '%d'", p1, ret);
	return ret;
}
/*
OPL_load_occurrence_data
int __cdecl OPL_load_occurrence_data(char const * __ptr64,class UGS::OCC_part * __ptr64 * __ptr64,enum PART_units * __ptr64)
*/
static pfOPL_load_occurrence_data OPL_load_occurrence_data_original;
static pfOPL_load_occurrence_data OPL_load_occurrence_data_next;
static int OPL_load_occurrence_data_hooked(char const * p1, CPP_PTR p2, CPP_ENUM * p3)
{
	int ret = 0;
	CALL_START("OPL_load_occurrence_data(p1='%s', p2='%p', p3='%p')", p1, p2, p3);
	NxlPath file = fix_path(p1);
	HOOK_CALL_NATIVE2(ret = OPL_load_occurrence_data_next(file.str(), p2, p3), ReportException(file));
	CALL_END("OPL_load_occurrence_data(p1='%s', p2='%p', p3='%p') returns '%d'", p1, p2, p3, ret);
	return ret;
}

/*
PFEDIT_open_storage_file_0
struct PFEDIT_storage_s * __ptr64 __cdecl PFEDIT_open_storage_file(char const * __ptr64,int,int)
*/
static pfPFEDIT_open_storage_file_0 PFEDIT_open_storage_file_0_original;
static pfPFEDIT_open_storage_file_0 PFEDIT_open_storage_file_0_next;
static CPP_PTR PFEDIT_open_storage_file_0_hooked(char const * p1, int p2, int p3)
{
	CPP_PTR ret = NULL;
	CALL_START("PFEDIT_open_storage_file_0(p1='%s', p2='%d', p3='%d')", p1, p2, p3);
	NxlPath file = fix_path(p1);
	HOOK_CALL_NATIVE2(ret = PFEDIT_open_storage_file_0_next(file.str(), p2, p3), ReportException(file));
	CALL_END("PFEDIT_open_storage_file_0(p1='%s', p2='%d', p3='%d') returns '%p'", p1, p2, p3, ret);
	return ret;
}

/*
PFEDIT_open_storage_file_1
struct PFEDIT_storage_s * __ptr64 __cdecl PFEDIT_open_storage_file(char const * __ptr64,int,int,struct CFI_file_usage_hints_t const * __ptr64)
*/
static pfPFEDIT_open_storage_file_1 PFEDIT_open_storage_file_1_original;
static pfPFEDIT_open_storage_file_1 PFEDIT_open_storage_file_1_next;
static CPP_PTR PFEDIT_open_storage_file_1_hooked(char const * p1, int p2, int p3, CPP_PTR p4)
{
	CPP_PTR ret = NULL;
	CALL_START("PFEDIT_open_storage_file_1(p1='%s', p2='%d', p3='%d', p4='%p')", p1, p2, p3, p4);
	NxlPath file = fix_path(p1);
	HOOK_CALL_NATIVE2(ret = PFEDIT_open_storage_file_1_next(file.str(), p2, p3, p4), ReportException(file));
	CALL_END("PFEDIT_open_storage_file_1(p1='%s', p2='%d', p3='%d', p4='%p') returns '%p'", p1, p2, p3, p4, ret);
	return ret;
}
#if NX_MAJOR_VER >= 12
/*
PFEDIT_open_storage_file
struct PFEDIT_storage_s * __ptr64 __cdecl PFEDIT_open_storage_file(char const * __ptr64,int,int,struct CFI_file_usage_hints_t const * __ptr64,enum UGS::System::Pfedit::StorageElements)
*/
static pfPFEDIT_open_storage_file_2 PFEDIT_open_storage_file_2_original;
static pfPFEDIT_open_storage_file_2 PFEDIT_open_storage_file_2_next;
static CPP_PTR PFEDIT_open_storage_file_2_hooked(char const * p1, int p2, int p3, CPP_PTR p4, CPP_ENUM p5)
{
	CPP_PTR ret = NULL;
	CALL_START("PFEDIT_open_storage_file_2(p1='%s', p2='%d', p3='%d', p4='%p', p5='%d')", p1, p2, p3, p4, p5);
	NxlPath file = fix_path(p1);
	HOOK_CALL_NATIVE2(ret = PFEDIT_open_storage_file_2_next(file.str(), p2, p3, p4, p5), ReportException(file));
	CALL_END("PFEDIT_open_storage_file_2(p1='%s', p2='%d', p3='%d', p4='%p', p5='%d') returns '%p'", p1, p2, p3, p4, p5, ret);
	return ret;
}
#endif
/*
ASSY_clone_add_assembly
int __cdecl ASSY_clone_add_assembly(class UGS::ASSY_clone * __ptr64,char const * __ptr64,struct UF_PART_load_status_s * __ptr64)
*/
static pfASSY_clone_add_assembly ASSY_clone_add_assembly_original;
static pfASSY_clone_add_assembly ASSY_clone_add_assembly_next;
static int ASSY_clone_add_assembly_hooked(CPP_PTR p1, char const * p2, CPP_PTR p3)
{
	int ret = 0;
	CALL_START("ASSY_clone_add_assembly(p1='%p', p2='%s', p3='%p')", p1, p2, p3);
	HOOK_API("libsyss.dll", CFI_is_storage_file);
	HOOK_API("libsyss.dll", OPL_load_occurrence_data);
	HOOK_API("libsyss.dll", PFEDIT_open_storage_file_0);
	HOOK_API("libsyss.dll", PFEDIT_open_storage_file_1);
	HOOK_API("libsyss.dll", PFEDIT_open_storage_file_2);
	s_partFiles.clear();
	HOOK_CALL_NATIVE2(ret = ASSY_clone_add_assembly_next(p1, p2, p3), ReportAssemblyException(p2));
	s_partFiles.clear();
	UNHOOK(CFI_is_storage_file);
	UNHOOK(OPL_load_occurrence_data);
	UNHOOK(PFEDIT_open_storage_file_0);
	UNHOOK(PFEDIT_open_storage_file_1);
	UNHOOK(PFEDIT_open_storage_file_2);
	CALL_END("ASSY_clone_add_assembly(p1='%p', p2='%s', p3='%p') returns '%d'", p1, p2, p3, ret);
	return ret;
}
void fix_export_outside_teamcenter()
{
	//this hook is used to fix an issue that happened during exporting protected assembly outside teamcenter
	//While user is trying to add a part/assembly in order to export outside teamcenter
	//if this part file is protected and has .nxl extension in FCC folder(e.g. fmsr_abcd.nxl)
	//NX will fail because it's try to searching the file which name is fmsr_abcd.nxl.prt
	//This issue will only happen on the file protected by dispatcher client
#if NX_MAJOR_VER <= 11
		HOOK_API("libsyss.dll", PFEDIT_open_part911);//file_reading//TODO:NX12/1872
#elif NX_MAJOR_VER >= 12
		HOOK_API("libassy.dll", ASSY_clone_add_assembly);
#endif
}

