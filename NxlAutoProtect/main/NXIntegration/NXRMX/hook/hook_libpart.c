/*
	Library:	libpart.dll
	Version:	10.0.0.24
*/
#include <Shlwapi.h>
#include <hook/hook_defs.h>
#include <hook/libpart.h>
#include <hook/libsyss.h>
#include <NXRMX/nx_contexts.h>
#include <utils_windows.h>
#include <uf_part.h>
#include <uf_assem.h>
#include <uf_ui_ugopen.h>
#include <uf_ugmgr.h>
#include <hook/hook_file_closed.h>
#include <NXRMX/NXUsageControl.h>

/*
	PartFileStream_ReleaseReplaceFile_0
	public: void __cdecl UGS::Part::PartFileStream::ReleaseReplaceFile(int,char const * __ptr64,bool) __ptr64
*/
static pfPartFileStream_ReleaseReplaceFile_0 PartFileStream_ReleaseReplaceFile_0_original;
static pfPartFileStream_ReleaseReplaceFile_0 PartFileStream_ReleaseReplaceFile_0_next;
static void PartFileStream_ReleaseReplaceFile_0_hooked(PartFileStream_PTR obj, int p2, char const * p3, BOOL p4)
{
	CALL_START("PartFileStream_ReleaseReplaceFile_0(obj='%p', p2='%d', p3='%s', p4='%d')", obj, p2, p3, p4);
	if(obj != NULL)
	{
		BOOL iswrite = PartFileStream_IsForWrite(obj);
		const char *osfile = PartFileStream_GetOSPath(obj);
		tag_t partTag = UF_PART_ask_part_tag(p3);
		DBGLOG("PartFileStream(%p):PartTag=%u", obj, partTag);
		if(osfile != NULL)
		{
			if(iswrite)
			{
				if (!PartFileStream_IsReadOnly(obj))
				{
					nx_on_part_saved(partTag, osfile);
				}
			}
			else if(partTag != NULL_TAG)
			{
				nx_on_part_loaded(partTag, p3, osfile);
			}
		}
	}
	CALL_NEXT(PartFileStream_ReleaseReplaceFile_0_next(obj, p2, p3, p4));

	CALL_END("PartFileStream_ReleaseReplaceFile_0(obj='%p', p2='%d', p3='%s', p4='%d')", obj, p2, p3, p4);
}
static BOOL is_opening = FALSE;
typedef struct open_event_args
{
	BOOL IsFirstOpen;
	const char *OsFile;
	const char *partName;
}OpenEventArgs_t;
static OpenEventArgs_t on_before_opening(PartFileStream_PTR stream, const char *partName, const char *partFile)
{
	OpenEventArgs_t args = {0};
	args.partName = partName;
	if(stream != NULL)
	{
		if (!is_opening)
		{
			//this open is triggered in first time			
			args.OsFile = PartFileStream_GetOSPath(stream);
			is_opening = TRUE;
			args.IsFirstOpen = TRUE;
			if (args.OsFile != NULL)
			{
				nx_on_file_reading(args.OsFile);
			}
			else if (!nx_isugmr())
			{
				//unmanaged mode use part name as file
				nx_on_file_reading(partName);
			}
			else if (partFile != nullptr) {
				nx_on_file_reading(partFile);
			}
		}
	}
	return args;
}
static void on_after_opened(PartFileStream_PTR stream, OpenEventArgs_t args)
{
	if(stream != NULL)
	{
		if(args.IsFirstOpen)
		{
			BOOL iswrite = PartFileStream_IsForWrite(stream);
			tag_t partTag = UF_PART_ask_part_tag(args.partName);
			DBGLOG("PartFileStream(%p): PartTag='%u'", stream, partTag);
			if(args.OsFile == NULL)
			{
				args.OsFile = PartFileStream_GetOSPath(stream);
			}
			if(args.OsFile != NULL)
			{
				//usage control:set part as readonly when user doesn't have edit right
				//TODO:move this to NXUsageControl module
				NxlPath filePath(args.OsFile);
				bool isprotected = false;
				if (g_rpmSession->CheckFileProtection(filePath.wstr(), &isprotected) == 0
					&& isprotected) {
					bool allowed = true;
					g_rpmSession->CheckFileRights(filePath.wstr(), RIGHT_EDIT, allowed);
					if (!allowed)
					{
						PartFileStream_SetReadOnly(stream);
					}
				}
				if(!iswrite)
				{
					if (partTag != NULLTAG)
					{
						nx_on_part_loaded(partTag, args.partName, args.OsFile);
					}
				}
				else if (partTag != NULL_TAG
					&& nxl_protection_is_enabled())
				{
					nx_on_part_saving(partTag, args.OsFile);
				}
			}
			is_opening = FALSE;
		}
	}
}
#define ERROR_PART_FILE_PURGED 641214
static void on_failed_open(OpenEventArgs_t args)
{
	UException_PTR except = Exception_askLastException();
	if(except == NULL) return;

	int code = Exception_askCode(except);
	const char *syslogMessage = Exception_askSyslogMessage(except);
	//TBD:print information in listing window only for known exception?
	if(code != ERROR_PART_FILE_PURGED)
	{
		return;
	}
	//ensure listing window is opened
	logical windowOpening = false;
	if(!NX_EVAL(UF_UI_is_listing_window_open(&windowOpening))
		|| !windowOpening)
	{
		//if eval failed or window is not opening, force open it
		NX_CALL(UF_UI_open_listing_window());
	}
	char messageBuf[1024];
	sprintf(messageBuf, "NextLabs NXIntegration caught an exception during opening following part:\n\t%s\n", args.partName);
	UF_UI_write_listing_window(messageBuf);
	char partNumber[UF_UGMGR_NAME_BUFSIZE] = "";
	if(nx_isugmr())
	{
		char partRev[UF_UGMGR_PARTREV_BUFSIZE ];
		char partFileType[UF_UGMGR_FTYPE_BUFSIZE ];
		char partFileName[UF_UGMGR_FNAME_BUFSIZE ];
		if(NX_EVAL(UF_UGMGR_decode_part_file_name(args.partName, partNumber, partRev, partFileType, partFileName)))
		{
			sprintf(messageBuf, "\tPartNumber : %s\n\tPartRev : %s\n", partNumber, partRev);
			UF_UI_write_listing_window(messageBuf);
		}
	}
	
	switch(code)
	{
		case ERROR_PART_FILE_PURGED:
			UF_UI_write_listing_window("\n[Problem]\nThe part file has been purged from FCC and attempting to redownload the file from Teamcenter failed.\n");
			UF_UI_write_listing_window("It is likely that the corresponding dataset version of the loaded part has been purged in Teamcenter.\n");
			//print solution
			UF_UI_write_listing_window("\n[Solution]\nPlease Close and Reopen the part to load a valid dataset.\nFollowing are the steps for reference:\n");
			sprintf(messageBuf, "\t1.Locate the part in Assembly Navigator(You can search the PartNumber-'%s' in 'Find Component')\n", partNumber);
			UF_UI_write_listing_window(messageBuf);
			UF_UI_write_listing_window("\t2.Right Click on the part, Click 'Close -> Part' to close the part\n");
			UF_UI_write_listing_window("\t3.Right Click on the part, Click 'Open -> Component' to reopen the part\n");
			break;
		default:
			sprintf(messageBuf, "\nException Information:\n\tCode : %d\n\tMessage : %s\n", code, syslogMessage);
			UF_UI_write_listing_window(messageBuf);
			//TODO:add solution
			break;
	}
}

/*from NX 2007, should not hook on PartOpen any more*/
#if NX_MAJOR_VER >= 12
/*
PartFileStream_Open_0
private: void __cdecl UGS::Part::PartFileStream::Open(class UGS::PART_part * __ptr64,char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool,enum UGS::System::Pfedit::StorageElements) __ptr64
*/
static pfPartFileStream_Open_0 PartFileStream_Open_0_original;
static pfPartFileStream_Open_0 PartFileStream_Open_0_next;
static void PartFileStream_Open_0_hooked(PartFileStream_PTR obj, CPP_PTR p1, char const * p2, char const * p3, CPP_PTR p4, CPP_ENUM p5, BOOL p6, CPP_ENUM p7)
{
	OpenEventArgs_t args;
	CALL_START("PartFileStream_Open_0(obj='%p', p1='%p', p2='%s', p3='%s', p4='%p', p5='%d', p6='%d', p7='%d')", obj, p1, p2, p3, p4, p5, p6, p7);
	args = on_before_opening(obj, p2, p3);

	HOOK_CALL_NATIVE2(PartFileStream_Open_0_next(obj, p1, p2, p3, p4, p5, p6, p7), on_failed_open(args));

	on_after_opened(obj, args);
	CALL_END("PartFileStream_Open_0(obj='%p', p1='%p', p2='%s', p3='%s', p4='%p', p5='%d', p6='%d', p7='%d')", obj, p1, p2, p3, p4, p5, p6, p7);
}

/*
PartFileStream_Open_1
public: void __cdecl UGS::Part::PartFileStream::Open(class UGS::PART_part * __ptr64,char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool) __ptr64
*/
static pfPartFileStream_Open_1 PartFileStream_Open_1_original;
static pfPartFileStream_Open_1 PartFileStream_Open_1_next;
static void PartFileStream_Open_1_hooked(PartFileStream_PTR obj, CPP_PTR p1, char const * p2, char const * p3, CPP_PTR p4, CPP_ENUM p5, BOOL p6)
{
	OpenEventArgs_t args;
	CALL_START("PartFileStream_Open_1(obj='%p', p1='%p', p2='%s', p3='%s', p4='%p', p5='%d', p6='%d')", obj, p1, p2, p3, p4, p5, p6);
	args = on_before_opening(obj, p2, p3);

	HOOK_CALL_NATIVE2(PartFileStream_Open_1_next(obj, p1, p2, p3, p4, p5, p6), on_failed_open(args));

	on_after_opened(obj, args);
	CALL_END("PartFileStream_Open_1(obj='%p', p1='%p', p2='%s', p3='%s', p4='%p', p5='%d', p6='%d')", obj, p1, p2, p3, p4, p5, p6);
}

/*
PartFileStream_Open_2
public: void __cdecl UGS::Part::PartFileStream::Open(char const * __ptr64,char const * __ptr64,class UGS::Part::PathCache * __ptr64,enum PART_open_mode_t,bool) __ptr64
*/
static pfPartFileStream_Open_2 PartFileStream_Open_2_original;
static pfPartFileStream_Open_2 PartFileStream_Open_2_next;
static void PartFileStream_Open_2_hooked(PartFileStream_PTR obj, char const * p1, char const * p2, CPP_PTR p3, CPP_ENUM p4, BOOL p5)
{
	OpenEventArgs_t args;
	CALL_START("PartFileStream_Open_2(obj='%p', p1='%s', p2='%s', p3='%p', p4='%d', p5='%d')", obj, p1, p2, p3, p4, p5);
	args = on_before_opening(obj, p2, NULL);

	HOOK_CALL_NATIVE2(PartFileStream_Open_2_next(obj, p1, p2, p3, p4, p5), on_failed_open(args));

	on_after_opened(obj, args);
	CALL_END("PartFileStream_Ope_2(obj='%p', p1='%s', p2='%s', p3='%p', p4='%d', p5='%d')", obj, p1, p2, p3, p4, p5);
}
#endif

/*
	Export Heal Geometry
	PART_copy_part
	unsigned int PART_copy_part(char const *,char const *,int,struct UF_PART_load_status_s *,class UGS::Part::SaveFailure *)
*/
static pfPART_copy_part PART_copy_part_original;
static pfPART_copy_part PART_copy_part_next;
unsigned int PART_copy_part_hooked(char const * p1,char const * p2,int p3,struct UF_PART_load_status_s * p4,void * p5)
{
	//part file-p1 is copying as part file p2(partTag=return)
	const char *filePath = p1;
	unsigned int ret = NULL;
	CALL_START("PART_copy_part(p1='%s', p2='%s', p3='%d', p4='%p', p5='%p')", p1, p2, p3, p4, p5);
	CALL_NEXT(ret = PART_copy_part_next(p1, p2, p3, p4, p5));
	if(nx_isugmr())
	{
		//in managed mode, search file path in context
		filePath = PartContext::SearchFileByName(p1);
	}
	//notify the context that the part-ret is loaded(partName=p1, and partFile=filePath)
	nx_on_part_loaded(ret, p1, filePath);
	CALL_END("PART_copy_part(p1='%s', p2='%s', p3='%d', p4='%p', p5='%p') returns '%u'", p1, p2, p3, p4, p5, ret);
	return ret;
}

/*
	12507:	PART_export_part
	ApiName:	PART_export_part
	FullName:	?PART_export_part@@YAHPEBDHPEAIPEAUPART_export_options_s@@@Z
	UnDecorated:	int __cdecl PART_export_part(char const * __ptr64,int,unsigned int * __ptr64,struct PART_export_options_s * __ptr64)
	Package = PART
*/
static pfPART_export_part PART_export_part_original;
static pfPART_export_part PART_export_part_next;
tag_t PART_export_part_hooked(char const * p1, int p2, unsigned int * p3, CPP_PTR p4)
{
	tag_t ret = NULLTAG;
	CALL_START("PART_export_part(p1='%s', p2='%d', *p3='%u', p4=%p)", p1, p2, *p3, p4);
	CALL_NEXT(ret = PART_export_part_next(p1, p2, p3, p4));
	if(p1 != NULL)
	{
		//find visible children parts in the assembly
		auto rootParts = export_find_source_parts(tags_make(p3, p2));
		std::vector<tag_t> visibleParts;
		std::vector<tag_t> partsToBeExported;
		drawing_list_visible_parts(NULLTAG, visibleParts);
		for (auto iParent = rootParts.begin(); iParent != rootParts.end(); iParent++)
		{
			partsToBeExported.push_back(*iParent);
			NXDBG("ParentPart:%u", *iParent);
			for (auto ipart = visibleParts.begin(); ipart != visibleParts.end();)
			{
				if (*ipart == *iParent)
				{
					ipart = visibleParts.erase(ipart);
					continue;
				}
				tag_t *occs = NULL;
				int noccs = UF_ASSEM_ask_occs_of_part(*iParent, *ipart, &occs);
				UF_free(occs);
				NXDBG("Part(%u) has %d occurrences of Part(%u)", *iParent, noccs, *ipart);
				if (noccs > 0)
				{
					partsToBeExported.push_back(*ipart);
					ipart = visibleParts.erase(ipart);
					continue;
				}
				ipart++;
			}
		}
		const char *ext = PathFindExtension(p1);
		if((ext == NULL || ext[0]=='\0')
			&& !file_exist(p1))
		{
			//append default ext .prt 
			std::string newpath = std::string(p1) + ".prt";
			DBGLOG("Searching:'%s'", newpath.c_str());
			nx_on_exported_parts(partsToBeExported, newpath.c_str());
		}
		else
		{
			nx_on_exported_parts(partsToBeExported, p1);
		}
	}
	CALL_END("PART_export_part(p1='%s', p2='%d', *p3='%u', p4=%p) returns %u", p1, p2, *p3, p4, ret);
	return ret;
}


static const char *s_rootDwgFile = NULL;
void HandleCreatedDWGFile(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if(isWrite
		&& string_ends_with(file, ".dwg", FALSE))
	{
		DBGLOG("FinishedWriting:'%s'(Size=%lld)", file, fileSize);
		if(s_rootDwgFile == NULL
			|| _stricmp(s_rootDwgFile, file) != 0)
		{
			nx_on_exported_fuzzy(file);
		}
	}
}

/*
	PART_load_foreign
	bool __cdecl PART__load_foreign(char const * __ptr64,int,bool,struct PROGRESS_step_s * __ptr64,struct UF_PART_load_status_s * __ptr64,unsigned int * __ptr64)
*/
static pfPART_load_foreign PART_load_foreign_original;
static pfPART_load_foreign PART_load_foreign_next;
static BOOL PART_load_foreign_hooked(char const * p1, int p2, BOOL p3, CPP_PTR p4, CPP_PTR p5, unsigned int * p6)
{
	BOOL ret = FALSE;
	CALL_START("PART_load_foreign(p1='%s', p2='%d', p3='%d', p4='%p', p5='%p', *p6='%u')", p1, p2, p3, p4, p5, *p6);
	CALL_NEXT(ret = PART_load_foreign_next(p1, p2, p3, p4, p5, p6));
	if(*p6 != NULLTAG)
	{
		if (!string_ends_with(p1, ".prt", false))
		{
			//when the foreign file is not prt file, a new file with name .prt will be created in non-protected format, we need reprotect it
			auto loadedParts = assembly_list_parts(*p6);
			NxlPath source(p1);
			bool isSrcProtected = false;
			NxlMetadataCollection srcTags;
			g_rpmSession->HelperReadFileTags(source.wstr(), &isSrcProtected, srcTags);
			DBGLOG("Successfully loaded foreign file-'%s'(protected=%d) into %d parts", p1, isSrcProtected, loadedParts.size());
			for (auto i = loadedParts.begin(); i != loadedParts.end(); i++)
			{
				auto prtFile = tag_to_name(*i);
				DBGLOG("[%u]'%s'", *i, prtFile.c_str());
				if (isSrcProtected)
				{
					//protect the new created file
					NxlPath target(prtFile);
					g_rpmSession->ProtectFile(target.wstr(), srcTags, true);
					//update PartContext
					auto existingContext = PartContext::FindByTag2(*i);
					if (existingContext != nullptr) {
						existingContext->SetProtectionStatus(true, srcTags);
					}
					else
					{
						PartContext::LoadContext(*i, target.string(), target, true, srcTags);
					}
				}
			}
		}
		else
		{
			nx_on_part_loaded(*p6, p1, p1);
		}
	}
	CALL_END("PART_load_foreign(p1='%s', p2='%d', p3='%d', p4='%p', p5='%p', *p6='%u') returns '%d'", p1, p2, p3, p4, p5, *p6, ret);
	return ret;
}

/*
	PART_save_as_foreign_part
	int __cdecl PART__save_as_foreign_part(unsigned int,char const * __ptr64 const)
*/
static pfPART_save_as_foreign_part PART_save_as_foreign_part_original;
static pfPART_save_as_foreign_part PART_save_as_foreign_part_next;
static int PART_save_as_foreign_part_hooked(unsigned int p1, char const * p2)
{
	int ret = 0;
	BOOL isdwg = string_ends_with(p2, ".dwg", FALSE);
	CALL_START("PART_save_as_foreign_part(p1='%u', p2='%s')", p1, p2);

	if(isdwg)
	{
		s_rootDwgFile = p2;
		//monitor the file creation
		register_FileClosedHandler(HandleCreatedDWGFile);
	}

	CALL_NEXT(ret = PART_save_as_foreign_part_next(p1, p2));
	//File->Save->SaveAs->Non PartFormat
	//IGS/STP203/STP214/DXF/DWG/STP will export the whole assembly into one file
	//DWG will export the assembly into multiple files
	//save as to prt file will not come to here, it will be enforced by the hook in PartFileStream
	//TODO:somehow when the work part is not the root part, it always save parts into prt files
	if(isdwg)
	{
		unregister_FileClosedHandler(HandleCreatedDWGFile);
		nx_on_exported_1to1(UF_PART_ask_display_part(), p2);//root assembly is exported to this file
	}
	else
	{
		NxlPath outfile(p2);
		if (ASSERT_FILE_EXISTS(outfile) == false) {
			//bug fix 56698:workaround
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
		nx_on_exported_all(p2);
		if (outfile.HasExtension(".obj")) {
			nx_on_exported_associated(p2, ".mtl");
		}
	}
	CALL_END("PART_save_as_foreign_part(p1='%u', p2='%s') returns '%d'", p1, p2, ret);
	return ret;
}

#if NX_MAJOR_VER >= 11
/*
BasicSaveManager_DoNXSave
private: bool __cdecl UGS::Part::BasicSaveManager::DoNXSave(class UGS::Part::SaveInput * __ptr64) __ptr64
*/
static pfBasicSaveManager_DoNXSave BasicSaveManager_DoNXSave_original;
static pfBasicSaveManager_DoNXSave BasicSaveManager_DoNXSave_next;
static BOOL BasicSaveManager_DoNXSave_hooked(CPP_PTR obj, CPP_PTR p1)
{
	BOOL ret = FALSE;
	CALL_START("BasicSaveManager_DoNXSave(obj='%p', p1='%p')", obj, p1);
	//check edit right and log the result
	{
		tag_t	partTag = SaveInput_GetPartTag(p1);
		auto partContext = PartContext::FindByTag(partTag);
		if (partContext != nullptr
			&& partContext->is_protected()) {
			bool isallowed = false;
			g_rpmSession->CheckFileRights(partContext->part_file().wstr(), RIGHT_EDIT, isallowed, true);
		}
	}
	CALL_NEXT(ret = BasicSaveManager_DoNXSave_next(obj, p1));
	CALL_END("BasicSaveManager_DoNXSave(obj='%p', p1='%p') returns '%d'", obj, p1, ret);
	return ret;
}
#endif
static BOOL libpart_hooked = FALSE;
static void libpart_hook_internal(HMODULE module)
{
	if (!libpart_hooked && module != NULL)
	{
		HOOK_CPP(BasicSaveManager_DoNXSave, module);
		if (nxl_protection_is_enabled())
		{
			//INIT_API_CPP(PartFileStream_GetOSPathName, module);
			//INIT_API_CPP(PartFileStream_IsForWrite, module);
			HOOK_CPP(PartFileStream_ReleaseReplaceFile_0, module);//file_saved/part_loaded

			HOOK_CPP(PART_copy_part, module); //Export -> Heal Geometry
			HOOK_CPP(PART_export_part, module);//Export -> DWG/DXF

			HOOK_CPP(PART_save_as_foreign_part, module);//Save->Save As(unmanaged mode， non-prt format)
			HOOK_CPP(PART_load_foreign, module);
		}
		//after introduced hooks on FCC_DownloadFilesFromPLM@libugmr.dll, the hooks on PartFilesStream_Open are not neccessary now
		//the only purpose of these hooks are to display NextLabs style dialog for exceptions on opening parts
		//introduce this registry for debug purpose
#if NX_MAJOR_VER < 2007
		if (REG_get_dword(REG_ROOT_KEY, "UseNXLExceptionDialog", 1) > 0)
		{
#if NX_MAJOR_VER >= 12
			//for managed mode, we need this hook to catch FCC exception to provide more information for user
			HOOK_CPP(PartFileStream_Open_0, module);
			HOOK_CPP(PartFileStream_Open_1, module);
			HOOK_CPP(PartFileStream_Open_2, module);
#endif
		}
#endif
		libpart_hooked = TRUE;
	}
}

void libpart_hook()
{
	libpart_hook_internal(GetModuleHandle("libpart.dll"));
}