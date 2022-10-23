#include <hook/hook_defs.h>
#include <hook/libpart.h>
#include <hook/libpartutils.h>
#include <NXRMX/nx_contexts.h>
#include <hook/hook_file_closed.h>

static std::vector<std::string> s_files;
void HandleTranslatedFile(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if (isWrite
		&& file != NULL)
	{
		DBGLOG("FinishedWriting:'%s'(Size=%lld)", file, fileSize);
		s_files.push_back(file);
	}
}
/*
	UG_translate_refset_1
	int __cdecl PVTRANS_UG_translate_refset(unsigned int,char const * __ptr64,int,unsigned int * __ptr64,int * __ptr64,char const * __ptr64,class UGS::UString & __ptr64,char const * __ptr64,void * __ptr64,bool)
*/
static pfUG_translate_refset_1 UG_translate_refset_1_original;
static pfUG_translate_refset_1 UG_translate_refset_1_next;
static int UG_translate_refset_1_hooked(unsigned int partTag, char const * p2, int p3, unsigned int * p4, int * p5, char const * sFolder, UString_REF p7, char const * p8, void * p9, BOOL p10)
{
	int ret = 0;
	const char *inputFileName = UString_utf8_str(p7);
	CALL_START("UG_translate_refset_1(p1='%u', p2='%s', p3='%d', p6='%s', UString_utf8_str(p7)='%s', p8='%s', p9, p10='%d')", partTag, p2, p3, sFolder, inputFileName, p8, p10);
	//pre-export
	s_files.clear();
	//monitor the file creation
	register_FileClosedHandler(HandleTranslatedFile);

	CALL_NEXT(ret = UG_translate_refset_1_next(partTag, p2, p3, p4, p5, sFolder, p7, p8, p9, p10));
	//post export
	unregister_FileClosedHandler(HandleTranslatedFile);
	for (auto ifile = s_files.begin(); ifile != s_files.end(); ifile++)
	{
		char folder[MAX_PATH];
		const char *fileNameWithExt = PathFindFileName(ifile->c_str());
		strcpy_s(folder, MAX_PATH, ifile->c_str());
		PathRemoveFileSpec(folder);
		if (sFolder==NULL|| strlen(sFolder)==0 || _stricmp(sFolder, folder) == 0)
		{
			if (nx_name_compare(inputFileName, fileNameWithExt, NULL) != equal_no)
			{
				nx_on_exported_1to1(partTag, ifile->c_str());
			}
		}
	}
	//clean
	s_files.clear();
	CALL_END("UG_translate_refset_1(p1='%u', p2='%s', p3='%d', p6='%s', UString_utf8_str(p7)='%s', p8='%s', p9, p10='%d') returns '%d'", partTag, p2, p3, sFolder, UString_utf8_str(p7), p8, p10, ret);
	return ret;
}

/*
	SaveManager_AddAssociatedFile
	public: void __cdecl UGS::Part::SaveManager::AddAssociatedFile(unsigned int,class UGS::UString const & __ptr64,bool,enum UGS::DataManagement::Model::Core::IFileType::Type const & __ptr64,class UGS::UString const & __ptr64)const __ptr64
*/
static pfSaveManager_AddAssociatedFile SaveManager_AddAssociatedFile_original;
static pfSaveManager_AddAssociatedFile SaveManager_AddAssociatedFile_next;
static void SaveManager_AddAssociatedFile_hooked(CPP_PTR obj, unsigned int p1, UString_PTR p2, BOOL p3, CPP_ENUM *p4, UString_PTR p5)
{
	const char *fileName = UString_utf8_str(p2);
	NxlPath file(fileName);
	bool isprotected = false;
	CALL_START("SaveManager_AddAssociatedFile(obj='%p', p1='%u', UString_utf8_str(p2)='%s', p3='%d', p4='%p', UString_utf8_str(p5)='%s')", obj, p1, fileName, p3, p4, UString_utf8_str(p5));
	if (g_rpmSession->HelperCheckFileProtection(file.wstr(), &isprotected)==0
		&& isprotected)
	{
		UString_REF newName = UString_append_3(p5, ".nxl");
		CALL_NEXT(SaveManager_AddAssociatedFile_next(obj, p1, p2, p3, p4, newName));
	}
	else
	{
		CALL_NEXT(SaveManager_AddAssociatedFile_next(obj, p1, p2, p3, p4, p5));
	}
	CALL_END("SaveManager_AddAssociatedFile(obj='%p', p1='%u', UString_utf8_str(p2)='%s', p3='%d', p4='%p', UString_utf8_str(p5)='%s')", obj, p1, fileName, p3, p4, UString_utf8_str(p5));
}
void hook_save_part_with_jt_data()
{
	//Save with Option-"Save Jt Data"
	//managed mode
	HOOK_API("libpart.dll", SaveManager_AddAssociatedFile);
	//unmanaged mode
	HOOK_API("libpartutils.dll", UG_translate_refset_1);
}