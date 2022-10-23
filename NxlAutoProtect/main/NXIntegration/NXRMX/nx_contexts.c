#include <uf_part.h>
#include <uf_ugmgr.h>
#include "nxl_app.h"
#include "nx_utils.h"
#include <hashtable.h>
#include <utils_windows.h>
#include <utils_nxl.h>
#include "nx_contexts.h"
#include <hook/hook_defs.h>
#include <hook/hook_injector.h>
#include <NXRMX/NXUsageControl.h>

std::vector<PartContext> PartContext::s_contexts;


std::vector<NxlPath> s_protectedFilesWithoutNxlExt;
//when this variable is set as true, all protected file will be appended .nxl extension immediately after protect
//when this variable is set as false, the protected file will be appended .nxl extension after protected only when it's in rpm folder
bool s_forceNxlExtension = true;
void nx_begin_export()
{
	s_protectedFilesWithoutNxlExt.clear();
	s_forceNxlExtension = false;
}
void nx_end_export()
{
	for (auto i = s_protectedFilesWithoutNxlExt.begin(); i != s_protectedFilesWithoutNxlExt.end(); i++)
	{
		//append .nxl extension to the file only when it's in non-rpm folder
		bool isrpmdir = false;
		if (ASSERT_FILE_EXISTS(*i)
			&& g_rpmSession->IsFileInRPMDir(i->wstr(), &isrpmdir)==0
			&& !isrpmdir)
		{
			auto nxlfile = i->EnsureNxlExtension();
			if (!MoveFileExW(i->wstr(), nxlfile.wstr(), MOVEFILE_REPLACE_EXISTING))
			{
				DBGLOG("Failed(%#x) to rename '%s'", GetLastError(), i->str());
			}
			else
			{
				DBGLOG("Renamed protected file to '%s'", nxlfile.tstr());
			}
		}
	}
	s_protectedFilesWithoutNxlExt.clear();
	s_forceNxlExtension = true;
}

#define REG_KEY_TRANSIENT_RPM_DIRS "SOFTWARE\\NextLabs\\NXRMX\\TransientRpmDirs"
void transient_rpm_dir_add(const char *dir)
{
	NxlPath path(dir);
	bool newRpmFolder;
	//add this dir as transient rpm dir
	if (g_rpmSession->AddRPMFolder(path.wstr(), &newRpmFolder, true) != 0)
		return;
	if (!newRpmFolder)
		return;

	HKEY hkey;
	DWORD dwDisposition;
	if (REG_CALL(RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_TRANSIENT_RPM_DIRS, 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dwDisposition)))
	{
		//add the mapping to registery for extenal application access
		DWORD transient = 1;
		if (REG_CALL(RegSetValueEx(hkey, path.str(), 0, REG_DWORD, (PBYTE)&transient, sizeof(transient))))
		{
			DBGLOG("AddTransientRpmDir:'%s'", path.tstr());
		}
		RegCloseKey(hkey);
	}
}
void transient_rpm_dir_clear()
{
	HKEY hkey;
	DWORD dwDisposition;
	if (REG_CALL(RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_TRANSIENT_RPM_DIRS, 0, NULL, 0, KEY_READ|KEY_WRITE, NULL, &hkey, &dwDisposition)))
	{
		WCHAR  valueName[MAX_PATH + 1];
		DWORD valIndex = 0;
		while (true)
		{
			DWORD maxValueName = MAX_PATH + 1;
			valueName[0] = '\0';
			DWORD ret = RegEnumValueW(hkey, valIndex, valueName, &maxValueName, NULL, NULL, NULL, NULL);
			if (ret == ERROR_NO_MORE_ITEMS) {
				DBGLOG("HKCU\\" REG_KEY_TRANSIENT_RPM_DIRS ": No more items");
				break;
			}
			if (ret != ERROR_SUCCESS)
			{
				report_win_error(ret, ERROR_SUCCESS, __FUNCTION__, __FILE__, __LINE__, "RegEnumValueA()");
				break;
			}
			DBGLOG("HKCU\\" REG_KEY_TRANSIENT_RPM_DIRS "[%lu]:'%S'", valIndex, valueName);
			if (g_rpmSession->RemoveRPMFolder(valueName, true) == 0)
			{
				if (REG_CALL(RegDeleteValueW(hkey, valueName)))
				{
					continue;
				}
			}
			valIndex++;
		}
		RegCloseKey(hkey);
	}
}

static BOOL protect_file(const NxlPath &file, const NxlMetadataCollection &tags)
{
	auto error = g_rpmSession->ProtectFile(file.wstr(), tags, s_forceNxlExtension);
	switch (error.Code())
	{
		case ERROR_SUCCESS:
			NXSYS("Protected:'%s'", file.tstr());
			if (!s_forceNxlExtension)
			{
				s_protectedFilesWithoutNxlExt.push_back(file);
			}
			return true;
		case ERROR_FILE_IS_PROTECTED:
			NXERR("FileAlreadyProtected('%s'):'%s'", file.tstr(), g_rpmSession->GetLastError().Message());
			return true;
		default:
			//TODO:??
			NXERR("ProtectFailed('%s'):'%s'", file.tstr(), g_rpmSession->GetLastError().Message());
			return false;
	}
}

void part_event_handler(UF_callback_reason_e_t reason, const void *pTag, void *closure)
{
    if(NX_EVAL(UF_initialize()))
    {
		tag_t partTag = *((tag_t*)pTag);
		const char *message = (const char *)closure;
		PartContext *pContext = PartContext::FindByTag2(partTag);

		if(pContext != NULL)
		{
			NXDBG("%s:(1/%u)" CONTEXT_FMT, message, PartContext::NumOfContexts(), CONTEXT_FMT_ARGS(pContext));
			switch (reason)
			{
			case UF_close_part_reason:
				PartContext::RemoveContext(partTag);
				break;
			case UF_rename_part_reason:
			case UF_save_part_reason:
				pContext->RefreshPartName();
				pContext->RefreshPartFile();
				break;
			default:
				break;
			}
		}
		else
		{
			char partName[MAX_FSPEC_BUFSIZE] = "";
			if (partTag != NULLTAG
				&& NX_EVAL(UF_PART_ask_part_name(partTag, partName)))
			{
				int loadStates = UF_PART_is_loaded(partName);
				NXDBG("%s:%u('%s'):loadStatus=%d", message, partTag, partName, loadStates);
			}
			else
			{
				NXDBG("%s:%u", message, partTag);
			}
		}
		NX_EVAL(UF_terminate());
	}
}

/*
	NX Event Handlers
*/
void nx_on_reset()
{
	//reset old context
	NXDBG("Clearing Saving Contexts...");
	PartContext::Reset();
}
void nx_on_initial()
{
	//load all part
	nx_on_reset();
	NXDBG("Initializing Contexts...");
	int ipart;
	int nParts = UF_PART_ask_num_parts();
	for(ipart = 0; ipart < nParts; ipart++)
	{
		PartContext::LoadContext(UF_PART_ask_nth_part(ipart));
	}
	NXDBG("Initialized %d Protected Part Files", PartContext::NumOfContexts());
}
void nx_on_finalize()
{
	//int icontext;
	//int nprocessed = 0;
	NXDBG("TODO:Finalizing Contexts...");
	/*
	while(ctx != NULL)
	{
		NXDBG("ProcessContext:"CONTEXT_FMT, CONTEXT_FMT_ARGS(ctx));
		if(protect_file(ctx->partFile, ctx->nxlTags))
		{
			nprocessed++;
		}
		ctx = ctx->nextContext;
	}
	NXDBG("Process %d Contexts", nprocessed);
	*/
}

void nx_on_part_saved(tag_t partTag, const char *tmpFile)
{
	auto ctx = PartContext::RetrieveContext(partTag);
	if(ctx == NULL)
	{
		NXDBG("PartSaved:%u TargetFile='%s' ContextNotFound", partTag, tmpFile);
	}
	else
	{
		NXDBG("PartSaved:%u TargetFile='%s' Context=" CONTEXT_FMT, partTag, tmpFile, CONTEXT_FMT_ARGS(ctx));
		if (ctx->is_protected()) {
			protect_file(NxlPath(tmpFile), ctx->tags());
		}
	}
}

static bool save_parts_to_file(const std::vector<tag_t> &parts, const NxlPath &toFile)
{
	if(parts.size() > 0)
	{
		int ipart;
		NxlMetadataCollection mergedTags;
		int nprotectedParts = 0;
		for(ipart=0; ipart < parts.size(); ipart++)
		{
			auto context = PartContext::RetrieveContext(parts[ipart]);
			if(context != NULL)
			{
				if (context->is_protected()) {
					nprotectedParts++;
					NXDBG("[%d/%d]MergingContext-%d:" CONTEXT_FMT, ipart + 1, parts.size(), nprotectedParts, CONTEXT_FMT_ARGS(context));
					mergedTags.join(context->tags());
				}
				else
				{
					NXDBG("[%d/%d]:" CONTEXT_FMT, ipart + 1, parts.size(), CONTEXT_FMT_ARGS(context));
				}
			}
			else
			{
				NXDBG("[%d/%d]Part-%u:ContextNotFound", ipart+1, parts.size(), parts[ipart]);
			}
		}
		NXDBG("Found %d protected part files", nprotectedParts);
		if(nprotectedParts > 0)
		{
			return protect_file(toFile, mergedTags);
		}
	}
	else
	{
		NXDBG("Saving %d parts file into '%s'", parts.size(), toFile.tstr());
	}
	return false;
}
void nx_on_exported_objects(const std::vector<tag_t> &objects, const char *tarFile)
{
	NXDBG("ObjectsExported:To='%s' nBodies='%d'", tarFile, objects.size());
	if(objects.size() > 0)
	{
		NxlPath file(tarFile);
		if(ASSERT_FILE_EXISTS(file))
		{
			std::vector<tag_t> sourceParts = export_find_source_parts(objects);
			save_parts_to_file(sourceParts, file);
		}
	}
}
bool nx_on_exported_parts(const std::vector<tag_t> &parts, const char *exportedFile)
{
	NXDBG("PartsExported:To='%s' nParts='%d'", exportedFile, parts.size());
	NxlPath file(exportedFile);
	if(ASSERT_FILE_EXISTS(file))
	{
		return save_parts_to_file(parts, file);
	}
	return false;
}

void nx_on_exported_all(const char *exportedFile)
{
	NXDBG("AllPartsExported:To='%s'", exportedFile);
	NxlPath file(exportedFile);
	if(ASSERT_FILE_EXISTS(file))
	{
		std::vector<tag_t> allparts = part_list_display_parts();
		save_parts_to_file(allparts, file);
	}
}
void nx_on_exported_drawing(tag_t drawing, const char *exportedFile)
{
	NXDBG("DrawingExported:Drawing=%d To='%s'", drawing, exportedFile);
	NxlPath file(exportedFile);
	if(ASSERT_FILE_EXISTS(file))
	{
		std::vector<tag_t> visibleParts;
		if(drawing_list_visible_parts(drawing, visibleParts))
		{
			save_parts_to_file(visibleParts, file);
		}
		else
		{
			//failed, use whole assembly
			//Note:this case mostly happend when the input drawing is not currently displayed
			std::vector<tag_t> allparts = part_list_display_parts();
			save_parts_to_file(allparts, file);
		}
	}
}
inline std::string PathFindNameOnly(const std::string &fullPath) {
	//remove extension;
	std::string name = PathFindFileName(fullPath.c_str());
	PathRemoveExtension(const_cast<LPSTR>(name.c_str()));
	return name;
}
void nx_on_exported_fuzzy(const char *exportedFile)
{
	int nParts = UF_PART_ask_num_parts();
	NXDBG("FuzzyExport:'%s'", exportedFile);
	if(nParts > 0)
	{
		//obtain the file name without extension from file path
		NxlPath filePath(exportedFile);
		if (!ASSERT_FILE_EXISTS(filePath)) return;
		const std::string fileNameOnly = PathFindNameOnly(filePath.path());
		int ipart;
		tag_t matchedPartTag = NULLTAG;
		size_t matchedNameLen = 0;
		NXDBG("Searching Part by name-'%s'...", fileNameOnly.c_str());
		for(ipart = 0; ipart < nParts; ipart++)
		{
			//cache partTag
			tag_t partTag = UF_PART_ask_nth_part(ipart);
			//cache part short name
			char partName[MAX_FSPEC_BUFSIZE];
			if(!NX_EVAL(UF_PART_ask_part_name(partTag, partName)))
				continue;
			std::string partShortName;
			if(nx_isugmr())
			{
#if (NXVER_MAJOR >= 10)
				//for debug only
				UF_UGMGR_part_file_info_t fileInfo = {0};
				UF_UGMGR_part_file_object_p_t *fileObjects = NULL;
				fileInfo.fileName = partName;
				if(NX_EVAL(UF_UGMGR_resolve_part_file_infos(1, &fileInfo, &fileObjects))
					&& fileObjects[0] != NULL)
				{
					partShortName = string_dup(fileObjects[0]->partNumber);
					//for debug only
					NXDBG("[%d]%lu:PN='%s' revID='%s' dsName='%s' dsType='%s'(%s)", ipart, partTag,
						fileObjects[0]->partNumber, fileObjects[0]->revisionID, fileObjects[0]->datasetName, fileObjects[0]->datasetType, partName);
					UF_UGMGR_tag_t database_part_tag;
					if(NX_EVAL(UF_UGMGR_ask_part_tag(fileObjects[0]->partNumber, &database_part_tag)))
					{
						char part_name[UF_UGMGR_NAME_BUFSIZE];
						char part_desc[UF_UGMGR_DESC_BUFSIZE];
						if(NX_EVAL(UF_UGMGR_ask_part_name_desc(database_part_tag, part_name, part_desc)))
						{
							NXDBG("==>database_part_tag:'%u' part_name:'%s' part_desc:'%s'", database_part_tag, part_name, part_desc);
						}
					}
				}
				UF_free(fileObjects);
#else
				char partNumber[UF_UGMGR_NAME_BUFSIZE];
				char partRev[UF_UGMGR_PARTREV_BUFSIZE ];
				char partFileType[UF_UGMGR_FTYPE_BUFSIZE ];
				char partFileName[UF_UGMGR_FNAME_BUFSIZE ];
				if(NX_EVAL(UF_UGMGR_decode_part_file_name(partName, partNumber, partRev, partFileType, partFileName)))
				{
					NXDBG("[%d]%lu:PN='%s' PR='%s' fileType='%s' fileName='%s'(%s)", ipart, partTag, partNumber, partRev, partFileType, partFileName, partName);
					partShortName = partNumber;
				}
#endif
			}
			else
			{
				//unmanaged mode
				partShortName = PathFindNameOnly(partName);
				NXDBG("[%d]%lu='%s'(%s)", ipart, partTag, partShortName.c_str(), partName);
			}
			if(partShortName.empty())
				continue;
			compare_result_t match = nx_name_compare(partShortName.c_str(), fileNameOnly.c_str(), NULL);
			if(match != equal_no)
			{
				size_t matchedLen = partShortName.length();
				if(match == equal_full)
				{
					//full match
					NXDBG("BestMatch:'%s'-'%s'(Old:%u-%d New:%u-%d)", partShortName.c_str(), fileNameOnly.c_str(), matchedPartTag, matchedNameLen, partTag, matchedLen);
					matchedPartTag = partTag;
					break;//break loop
				}
				else if(matchedLen > matchedNameLen)
				{
					//better match
					NXDBG("BetterMatch:'%s'-'%s'(Old:%u-%d New:%u-%d)", partShortName.c_str(), fileNameOnly.c_str(), matchedPartTag, matchedNameLen, partTag, matchedLen);
					matchedPartTag = partTag;
					matchedNameLen = matchedLen;
				}
				else
				{
					NXDBG("NotBetterMatch:'%s'-'%s'(Old:%u-%d New:%u-%d)", partShortName.c_str(), fileNameOnly.c_str(), matchedPartTag, matchedNameLen, partTag, matchedLen);
				}
			}
		}
		if(matchedPartTag != NULLTAG)
		{
			nx_on_part_saved(matchedPartTag, exportedFile);
		}
		else
		{
			ERRLOG("Failed to obtain the source part of this file('%s'), protect it with assembly metadatas(nx_on_exported_xto1)", exportedFile);
			nx_on_exported_xto1(exportedFile);
		}
	}
}
void nx_on_exporting_objects(const std::vector<tag_t> &objects, tag_t toPart)
{
	std::vector<tag_t> sourceParts;
	if (objects.empty())
	{
		NXDBG("ExportingDrawing:To='%u'", toPart);
		sourceParts = part_list_display_parts();
	}
	else
	{
		NXDBG("ExportingObjects:To='%u' nObjects='%d'", toPart, objects.size());
		sourceParts = export_find_source_parts(objects);
	}
	if (sourceParts.size() > 0 && toPart != NULLTAG)
	{
		int ipart;
		int nProtectedParts = 0;
		PartContext *partContext = PartContext::RetrieveContext(toPart);
		NxlMetadataCollection mergedTags;
		//read existing tags
		if (partContext != NULL)
		{
			NXDBG("CurrentContext:"CONTEXT_FMT, CONTEXT_FMT_ARGS(partContext));
			if (partContext->is_protected())
			{
				nProtectedParts++;
				mergedTags.join(partContext->tags());
			}
		}
		//
		for (ipart = 0; ipart < sourceParts.size(); ipart++)
		{
			auto context = PartContext::RetrieveContext(sourceParts[ipart]);
			if (context != NULL)
			{
				NXDBG("SourcePart[%d/%d]:"CONTEXT_FMT, ipart + 1, sourceParts.size(), CONTEXT_FMT_ARGS(context));
				if (context->is_protected()
					&& context->part_tag() != toPart)
				{
					nProtectedParts++;
					mergedTags.join(context->tags());
				}
			}
			else
			{
				NXDBG("SourcePart[%d/%d]:%u-ContextNotFound", ipart + 1, sourceParts.size(), sourceParts[ipart]);
			}
		}
		NXDBG("Found %d Protected Source Parts for Target Part", nProtectedParts);
		if (partContext == NULL)
		{
			//load this part as protected context
			std::string partName = tag_to_name(toPart);
			NxlPath file(name_to_file(partName.c_str()));
			PartContext::LoadContext(toPart, partName, file, nProtectedParts > 0, mergedTags);
		}
		else
		{
			//update existing context;
			partContext->SetProtectionStatus(nProtectedParts > 0, mergedTags);
		}
	}
}


void nx_on_exporting_by_spawn(const char *inputFile, const char *outputFile)
{
	NXDBG("SpawnExport:InputFile='%s' OutputFile='%s'(LastSpawnedPid=%d)", inputFile, outputFile, SpawnContext::Previous().pid());
	if(file_exist(inputFile))
	{
		//TODO：make the execution in new thread??
		if(SpawnContext::Previous().wait_finish())
		{
			//iges file is created
			nx_on_file_copied(inputFile, outputFile, true);
			//reset pid cache
			//_last_spawned_pid = -1;
		}
	}
	else
	{
		NXERR("==>FileNotFound:'%s'", inputFile);
	}
}
void nx_on_exported_associated(const char *file, const char *extOfAssociated)
{
	NXDBG("ExportAssociated:InputFile='%s' ExtensionOfAssociated='%s'", file, extOfAssociated);
	auto sourceFile = NxlPath(file).RemoveNxlExtension();
	if (!sourceFile.IsValid())
		return;
	auto sourceNxlFile = sourceFile.EnsureNxlExtension();
	auto associatedFile = sourceFile.WithoutExtension().Append(extOfAssociated);
	if (!ASSERT_FILE_EXISTS(sourceFile)) {
		if (ASSERT_FILE_EXISTS(sourceNxlFile)) {
			NXDBG("File-'%s' is protected and in non-rpm Dir", file);
			nx_on_file_copied(sourceNxlFile.chars(), associatedFile.chars(), true);
		}
	}
	else
	{
		nx_on_file_copied(sourceFile.chars(), associatedFile.chars(), true);
	}
}

void nx_on_file_reading(const char *file)
{
	//verify if file name is correct
	NxlPath filePath(file);
	bool isProtected = false;
	bool inrpmdir = false;
	g_rpmSession->ValidateFileExtension(filePath.wstr(), &isProtected, &inrpmdir);
	if (isProtected)
	{
		if (!inrpmdir) {
			DBGLOG("'%s' is not in a RPM folder", file);
			g_rpmSession->NotifyUser(L"Opening Protected file from non-RPM folder is not supported", filePath.wstr(), WTEXT(ACTION_VIEW));
		}
		else
		{
			bool allow = false;
			g_rpmSession->CheckFileRights(filePath.wstr(), RIGHT_VIEW, allow, true);
			if (!allow) {
				DBGLOG("You don't have permission to View this file-'%s'", file);
				g_rpmSession->NotifyUser(L"You don't have permission to View this file", filePath.wstr(), WTEXT(ACTION_VIEW));
			}
		}
	}
}
void nx_on_file_copied(const char *srcFile, const char *tarFile, bool appendNxl)
{
	NxlPath source(srcFile);
	NxlPath target(tarFile);
	if(ASSERT_FILE_EXISTS(source)
		&& ASSERT_FILE_EXISTS(target))
	{
		bool isSrcProtected;
		NxlMetadataCollection srcTags;
		if(g_rpmSession->HelperReadFileTags(source.wstr(), &isSrcProtected, srcTags) == 0
			&& isSrcProtected)
		{
			bool isTarProtected;
			if (g_rpmSession->ValidateFileExtension(target.wstr(), &isTarProtected) == 0)
			{
				if (!isTarProtected)
				{
					g_rpmSession->ProtectFile(target.wstr(), srcTags, appendNxl);
				}
			}
		}
	}
}
void nx_on_part_loaded(tag_t partTag, const char *partName, const char *partFile)
{
	auto context = PartContext::FindByTag(partTag);
	NxlPath file(partFile);
	if(context != NULL)
	{
		NXDBG("PartLoaded:'%u'(Name='%s' File='%s') Context has been loaded" CONTEXT_FMT, partTag, partName, partFile, CONTEXT_FMT_ARGS(context));
	}
	else if(partTag != NULL_TAG
		&& file.CheckFileExists())
	{
		//load new context
		auto loaded = PartContext::LoadContext(partTag, partName, file);
		//loaded as protected file
		NXDBG("PartLoaded:'%u'(Name='%s' File='%s'):" CONTEXT_FMT, partTag, partName, partFile, CONTEXT_FMT_ARGS(loaded));
	}
}

void nx_on_part_saving(tag_t partTag, const char *targetFile)
{
	NXDBG("PartSaving:%d(SavingTo='%s')", partTag, targetFile);
}

//part file is downloaded to local
void nx_on_fcc_downloaded(const char *fccFile)
{
}
//local file is being saved to FCC
void nx_on_fcc_uploading(const char *tmpFile)
{
}
//local file is saved to Teamcenter
void nx_on_fcc_uploaded(const char *tmpFile, const char *volumeId)
{
}

