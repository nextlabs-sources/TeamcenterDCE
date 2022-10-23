#include <stdafx.h>
#include <hook/hook_defs.h>
#include <hook/libpart.h>
#include <hook/libsyss.h>
#include <hook/hook_file_closed.h>
#include <hook/rmc_fixes.h>
#include <utils_string.h>
#include <NXRMX/NXUsageControl.h>
#include <stdio.h>

#include <key_value_list.h>
#include <utils_windows.h>
#include <NxlUtils/NxlPath.hpp>

RPMSession *g_rpmSession = nullptr;

void HandleLastException()
{
	UException_PTR except = Exception_askLastException();
	if(except == NULL) return;
	//log basic exception information
	DBGLOG("==>Type:%d", Exception_askType(except));
	const char *syslogMessage = Exception_askSyslogMessage(except);
	DBGLOG("==>SyslogMessage:%s", syslogMessage);
	int code = Exception_askCode(except);
	DBGLOG("==>Code:%d", code);
}

int nx_get_major_version()
{
	static int majorVersion = 0;
	if (majorVersion == 0)
	{
		module_query_ver("libsyss.dll", &majorVersion, NULL, NULL, NULL);
		if (majorVersion != NX_MAJOR_VER)
		{
			ERRLOG("NX version(%d) is not matched with this library(%d)", majorVersion, NX_MAJOR_VER);
		}
	}
	return majorVersion;
}

void CheckFilePermission(const NxlPath &file)
{
	bool isProtected = false;
	bool inrpmdir = false;
	g_rpmSession->ValidateFileExtension(file.wstr(), &isProtected, &inrpmdir);
	if (isProtected)
	{
		if (!inrpmdir) {
			DBGLOG("'%s' is not in a RPM folder", file);
			g_rpmSession->NotifyUser(L"Opening Protected file from non-RPM folder is not supported", file.wstr(), L"Export");
		}
		else
		{
			std::vector<std::pair<std::string, std::string>> rights;
			rights.push_back(std::make_pair(RIGHT_VIEW, ACTION_VIEW));
			for (auto r = rights.begin(); r != rights.end(); r++)
			{
				bool allow = false;
				g_rpmSession->CheckFileRights(file.wstr(), r->first.c_str(), allow, true);
				if (!allow) {
					DBGLOG("You don't have permission(%s) to %s this file-'%s'", r->first.c_str(), r->second.c_str(), file);
					wchar_t buffer[MAX_PATH];
					wsprintfW(buffer, L"You don't have permission(%S) to %S this file", r->first.c_str(), r->second.c_str());
					g_rpmSession->NotifyUser(buffer, file.wstr(), L"Export");
					break;
				}
			}
		}
	}
}

void on_exported(const std::vector<NxlPath> &srcFiles, const char *file)
{
	if(srcFiles.empty())
		return;
	if(!g_rpmSession->IsNxlProtectionEnabled()) return;
	NxlPath exportedFile(file);
	if(!exportedFile.CheckFileExists())
	{
		DBGLOG("FileNotFound:'%s'", exportedFile);
		return;
	}
	bool isprotected;
	if(g_rpmSession->HelperCheckFileProtection(exportedFile.wstr(), &isprotected)==0
		&& isprotected)
	{
		DBGLOG("AlreadyProtected:'%s'", exportedFile);
		return;
	}
	NxlMetadataCollection mergedTags;
	int nprotectedParts = 0;
	int ipart = 0;
	for (int ifile = 0; ifile < srcFiles.size(); ifile++)
	{
		auto file = srcFiles[ifile];
		ipart++;
		NxlMetadataCollection tags;
		if (g_rpmSession->HelperReadFileTags(file.wstr(), &isprotected, tags) == 0
			&& isprotected)
		{
			nprotectedParts++;
			DBGLOG("[%d/%d]Protected(len=%d):'%s'", ipart, srcFiles.size(), tags.len(), file.tstr());
			mergedTags.join(tags);
		}
		else
		{
			DBGLOG("[%d/%d]Non-Protected:'%s'", ipart, srcFiles.size(), file.tstr());
		}
	}
	DBGLOG("Found tags from %d protected part files", nprotectedParts);
	if(nprotectedParts > 0)
	{
		if(g_rpmSession->ProtectFile(exportedFile.wstr(), mergedTags, true)==0)
		{
			DBGLOG("Protected:'%s'", exportedFile.tstr());
			//TODO:should this file has .nxl extension??
		}
		else
		{
			//TODO:??
			ERRLOG("ProtectFailed:'%s'", exportedFile.tstr());
		}
	}
}
NxlPath stream_get_file_path(PartFileStream_PTR streamObj, const char *partName="", const char *partFile="") {
	const char *osfile = PartFileStream_GetOSPath(streamObj);
	if (osfile != nullptr)
		return NxlPath(osfile);
	NxlPath path(partFile);
	if (path.IsValid()
		&& ASSERT_FILE_EXISTS(path))
	{
		return path;
	}
	return NxlPath(partName);
}
static std::vector<NxlPath> loaded_files;
void on_loaded(std::vector<NxlPath> &list, const char *file)
{
	if(file != NULL)
	{
		NxlPath filePath(file);
		if(ASSERT_FILE_EXISTS(filePath))
		{
			//check duplication
			for (int ifile = 0; ifile < loaded_files.size(); ifile++)
			{
				if (stricmp(loaded_files[ifile].str(), file) == 0) {
					return;
				}
			}
			loaded_files.push_back(filePath);
		}
	}
}

/*
	BASE_open_part
	int __cdecl BASE_open_part(char const * __ptr64,unsigned int * __ptr64,struct UF_PART_load_status_s * __ptr64)
*/
static pfBASE_open_part BASE_open_part_original;
static pfBASE_open_part BASE_open_part_next;
static int BASE_open_part_hooked(char const * p1, unsigned int * p2, CPP_PTR p3)
{
	int ret = 0;
	CALL_START("BASE_open_part(p1='%s', *p2='%u', p3='%p')", p1, *p2, p3);
	CALL_NEXT(ret = BASE_open_part_next(p1, p2, p3));
	on_loaded(loaded_files, p1);
	CALL_END("BASE_open_part(p1='%s', *p2='%u', p3='%p') returns '%d'", p1, *p2, p3, ret);
	return ret;
}

#if NX_MAJOR_VER>=9 && NX_MAJOR_VER<=11
/*
	PartFileStream_Open_1
	public: void __cdecl UGS::Part::PartFileStream::Open(char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool,bool * __ptr64,bool) __ptr64
*/
static pfPartFileStream_Open_1 PartFileStream_Open_1_original;
static pfPartFileStream_Open_1 PartFileStream_Open_1_next;
static void PartFileStream_Open_1_hooked(PartFileStream_PTR obj, char const * p2, CPP_PTR p3, CPP_ENUM p4, BOOL p5, BOOL * p6, BOOL p7)
{
	CALL_START("PartFileStream_Open_1(obj='%p', p2='%s', p3='%p', p4='%p', p5='%d', p6='%d', p7='%d')", obj, p2, p3, p4, p5, *p6, p7);
	NxlPath prtFile = stream_get_file_path(obj, p2);
	//validate file name and check file permission
	CheckFilePermission(prtFile);
	
	bool loaded = true;
	HOOK_CALL_NATIVE2(PartFileStream_Open_1_next(obj, p2, p3, p4, p5, p6, p7), loaded = false);
	if (loaded) {
		prtFile = stream_get_file_path(obj, prtFile.str());
		on_loaded(loaded_files, prtFile.str());
	}
	CALL_END("PartFileStream_Open_1(obj='%p', p2='%s', p3='%p', p4='%p', p5='%d', p6='%d', p7='%d')", obj, p2, p3, p4, p5, *p6, p7);
}
#elif NX_MAJOR_VER >= 12
/*
PartFileStream_Open_0
private: void __cdecl UGS::Part::PartFileStream::Open(class UGS::PART_part * __ptr64,char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool,enum UGS::System::Pfedit::StorageElements) __ptr64
*/
static pfPartFileStream_Open_0 PartFileStream_Open_0_original;
static pfPartFileStream_Open_0 PartFileStream_Open_0_next;
static void PartFileStream_Open_0_hooked(PartFileStream_PTR obj, CPP_PTR p1, char const * p2, char const * p3, CPP_PTR p4, CPP_ENUM p5, BOOL p6, CPP_ENUM p7)
{
	CALL_START("PartFileStream_Open_0(obj='%p', p1='%p', p2='%s', p3='%s', p4='%p', p5='%d', p6='%d', p7='%d')", obj, p1, p2, p3, p4, p5, p6, p7);
	NxlPath prtFile = stream_get_file_path(obj, p2, p3);
	//validate file name and check file permission
	CheckFilePermission(prtFile);

	bool loaded = true;
	HOOK_CALL_NATIVE2(PartFileStream_Open_0_next(obj, p1, p2, p3, p4, p5, p6, p7), loaded = false);
	if (loaded) {
		prtFile = stream_get_file_path(obj, prtFile.str());
		on_loaded(loaded_files, prtFile.str());
	}
	CALL_END("PartFileStream_Open_0(obj='%p', p1='%p', p2='%s', p3='%s', p4='%p', p5='%d', p6='%d', p7='%d')", obj, p1, p2, p3, p4, p5, p6, p7);
}
#endif

bool RegisterPartOpen() {
#if NX_MAJOR_VER >= 9 && NX_MAJOR_VER <=11
	HOOK_API("libpart.dll", PartFileStream_Open_1);
	HOOK_API("libpart.dll", BASE_open_part);//for debug only
#elif NX_MAJOR_VER >= 12 
	HOOK_API("libpart.dll", PartFileStream_Open_0);
	HOOK_API("libpart.dll", BASE_open_part);//for debug only
#endif
	return true;
}
bool UnregisterPartOpen() {
#if NX_MAJOR_VER >= 9 && NX_MAJOR_VER <=11
	UNHOOK(BASE_open_part);
	UNHOOK(PartFileStream_Open_1);
#elif NX_MAJOR_VER >= 12
	UNHOOK(BASE_open_part);
	UNHOOK(PartFileStream_Open_0);
#endif
	return true;
}
/*
	iges.exe
*/
static void iges_on_FileClosed(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if(isWrite
		&& string_ends_with(file, ".igs", FALSE))
	{
		DBGLOG("Exported:'%s'(Size=%lld) From %d part file(s)", file, fileSize, loaded_files.size());
		on_exported(loaded_files, file);
	}
}
BOOL iges_install()
{
	//iges.exe loads input part file by BASE_open_part()
	//write .igs file from main()
	 loaded_files.clear();
	 RegisterPartOpen();
	 register_FileClosedHandler(iges_on_FileClosed);
	 //install rmc fix
	 //fix_QueryDirectoryFile_install();
	 return TRUE;
}
void iges_uninstall()
{
	unregister_FileClosedHandler(iges_on_FileClosed);
	UnregisterPartOpen();
	//fix_QueryDirectoryFile_uninstall();
	loaded_files.clear();
}

/*
	step203ug.exe
	step214ug.exe
*/
static void step_on_FileClosed(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if(isWrite
		&& (string_ends_with(file, ".%%%", FALSE)
			|| string_ends_with(file, ".stp", FALSE)))
	{
		DBGLOG("Exported:'%s'(Size=%lld) From %d part file(s)", file, fileSize, loaded_files.size());
		on_exported(loaded_files, file);
	}
}
BOOL step_install()
{
	//step203ug.exe/step214ug.exe loads input part file by BASE_open_part()
	//write content into temp file(.%%%) which will be renamed to .stp
	loaded_files.clear();
	RegisterPartOpen();
	//install rmc fix
	//fix_QueryDirectoryFile_install();
	//hook MoveFileExW to redirect sourceFile.%%% to .%%%.nxl
	fix_rpm_movefile_install();
	register_FileClosedHandler(step_on_FileClosed);
	return TRUE;
}
void step_uninstall()
{
	unregister_FileClosedHandler(step_on_FileClosed);
	UnregisterPartOpen();
	//fix_QueryDirectoryFile_uninstall();
	fix_rpm_movefile_uninstall();
	loaded_files.clear();
}

/*ugto2d.exe*/
/*
	PART_export_part
	int __cdecl PART_export_part(char const * __ptr64,int,unsigned int * __ptr64,struct PART_export_options_s * __ptr64)
*/
static pfPART_export_part PART_export_part_original;
static pfPART_export_part PART_export_part_next;
unsigned int PART_export_part_hooked(char const * p1, int p2, unsigned int * p3, CPP_PTR p4)
{
	unsigned int ret = 0;
	CALL_START("PART_export_part(p1='%s', p2='%d', *p3='%u', p4=%p)", p1, p2, *p3, p4);
	CALL_NEXT(ret = PART_export_part_next(p1, p2, p3, p4));
	on_exported(loaded_files, p1);
	CALL_END("PART_export_part(p1='%s', p2='%d', *p3='%u', p4=%p) returns %u", p1, p2, *p3, p4, ret);
	return ret;
}

/*
	BASE_part_save_as
	void __cdecl BASE_part_save_as(unsigned int,char const * __ptr64,int * __ptr64,int * __ptr64,bool,int * __ptr64)
*/
static pfBASE_part_save_as BASE_part_save_as_original;
static pfBASE_part_save_as BASE_part_save_as_next;
static void BASE_part_save_as_hooked(unsigned int p1, char const * p2, int * p3, int * p4, BOOL p5, int * p6)
{
	CALL_START("BASE_part_save_as(p1='%u', p2='%s', p3, p4, p5='%d', p6)", p1, p2, p5);
	CALL_NEXT(BASE_part_save_as_next(p1, p2, p3, p4, p5, p6));
	CALL_END("BASE_part_save_as(p1='%u', p2='%s', p3, p4, p5='%d', p6)", p1, p2, p5);
}

/*
	PartFileStream_ReleaseReplaceFile_0
	public: void __cdecl UGS::Part::PartFileStream::ReleaseReplaceFile(int,char const * __ptr64,bool) __ptr64
*/
static pfPartFileStream_ReleaseReplaceFile_0 PartFileStream_ReleaseReplaceFile_0_original;
static pfPartFileStream_ReleaseReplaceFile_0 PartFileStream_ReleaseReplaceFile_0_next;
static void PartFileStream_ReleaseReplaceFile_0_hooked(PartFileStream_PTR obj, int p2, char const * p3, BOOL p4)
{
	CALL_START("PartFileStream_ReleaseReplaceFile_0(obj='%p', p2='%d', p3='%s', p4='%d')", obj, p2, p3, p4);
	BOOL iswrite = PartFileStream_IsForWrite(obj);
	CALL_NEXT(PartFileStream_ReleaseReplaceFile_0_next(obj, p2, p3, p4));
	if(iswrite)
	{
		NxlPath prtFile = stream_get_file_path(obj, p3);
		on_exported(loaded_files, prtFile.str());
		//clear loaded_files??
		loaded_files.clear();
	}
	CALL_END("PartFileStream_ReleaseReplaceFile_0(obj='%p', p2='%d', p3='%s', p4='%d')", obj, p2, p3, p4);
}
BOOL ugto2d_install()
{
	//ugto2d.exe always output one .prt file in following steps:
	//1, the input part file is loaded by BASE_open_part()
	//2, loaded part files will be exported into a temp .prt file(e.g.'C:\Users\tcadmin\AppData\Local\Temp\tmp_ugto2d_00000120_00001.prt') by PART_export_part()
	//3, The temp .prt file is saved as the output file by BASE_part_save_as()
	//all of part file reading and writing will be done by PartFileStream
	loaded_files.clear();
	RegisterPartOpen();
	 HOOK_API("libpart.dll", PartFileStream_ReleaseReplaceFile_0);//protect after part file writing
	 HOOK_API("libpart.dll", PART_export_part);
	 HOOK_API("libpart.dll", BASE_part_save_as);//for debug only
	 //register_FileClosedHandler(HandleCreatedFile);
	 return TRUE;
}
void ugto2d_uninstall()
{
	UnregisterPartOpen();
	UNHOOK(PartFileStream_ReleaseReplaceFile_0);
	UNHOOK(PART_export_part);
	UNHOOK(BASE_part_save_as);
	//unregister_FileClosedHandler(HandleCreatedFile);
	loaded_files.clear();
}

/*dxfdwg.exe*/
static void dxfdwg_on_FileClosed(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if(isWrite
		&& (string_ends_with(file, ".dxf", FALSE)
		||string_ends_with(file, ".dwg", FALSE)))
	{
		DBGLOG("Exported:'%s'(Size=%lld) From %d part file(s)", file, fileSize, loaded_files.size());
		on_exported(loaded_files, file);
	}
}
BOOL dxfdwg_install()
{
	//dxfdwg.exe loads input prt file by BASE_open_part()
	//write the content into .dxf/.dwg file directly
	loaded_files.clear();
	RegisterPartOpen();
	 register_FileClosedHandler(dxfdwg_on_FileClosed);
	 return TRUE;
}
void dxfdwg_uninstall()
{
	UnregisterPartOpen();
	unregister_FileClosedHandler(dxfdwg_on_FileClosed);
	loaded_files.clear();
}
/*wavefrontobj.exe*/
static void wavefront_on_FileClosed(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if (isWrite
		&& (string_ends_with(file, ".obj", FALSE)
			|| string_ends_with(file, ".mtl", FALSE)))
	{
		DBGLOG("Exported:'%s'(Size=%lld) From %d part file(s)", file, fileSize, loaded_files.size());
		on_exported(loaded_files, file);
	}
}
BOOL wavefront_install()
{
	loaded_files.clear();
	RegisterPartOpen();
	register_FileClosedHandler(wavefront_on_FileClosed);

	return TRUE;
}

void wavefront_uninstall()
{
	UnregisterPartOpen();
	unregister_FileClosedHandler(wavefront_on_FileClosed);
	loaded_files.clear();
}