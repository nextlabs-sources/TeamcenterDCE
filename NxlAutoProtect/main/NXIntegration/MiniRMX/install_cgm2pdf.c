#include <stdafx.h>
#include <hook/hook_defs.h>
#include <hook/libsyss.h>
#include <hook/hook_file_closed.h>
#include <hook/rmc_fixes.h>
#include <utils_windows.h>
#include <NxlUtils/NxlPath.hpp>

extern void on_exported(const std::vector<NxlPath> &srcFiles, const char *file);
extern void CheckFilePermission(const NxlPath &file);


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
	NxlPath srcFile(p1);
	srcFile = srcFile.RemoveNxlExtension();
	CALL_NEXT(ret = CFI_copy_file_0_next(srcFile.str(), p2, p3, p4, p5, p6, p7));
	CALL_END("CFI_copy_file_0(p1='%s', p2='%s', p3='%d', p4='%d', p5='%d', p6='%d', p7) returns '%d'", srcFile.tstr(), p2, p3, p4, p5, p6, ret);
	return ret;
}
/*
cgm2pdf.exe
*/
#define REG_KEY_RPM_FILES "SOFTWARE\\NextLabs\\NXRMX\\RPMFiles"
static void clean_rpm_file(const NxlPath &file)
{
	//copy the rpm file back to original folder
	HKEY hkey;
	DWORD dwDisposition;
	if (REG_CALL(RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_RPM_FILES, 0, NULL, 0, KEY_QUERY_VALUE | KEY_WRITE, NULL, &hkey, &dwDisposition)))
	{
		char lszValue[MAX_PATH];
		DWORD dwType = REG_SZ;
		DWORD dwSize = MAX_PATH;
		auto rpmfile = file.RemoveNxlExtension();//make sure the entry no nxl extension
		if (REG_CALL(RegQueryValueEx(hkey, rpmfile.str(), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)))
		{
			NxlPath ofile(lszValue);
			DBGLOG("Found RPMEditCopy:'%s'='%s'", rpmfile.tstr(), ofile.tstr());
			g_rpmSession->RPMEditSave(RPMSession::SaveAndFinish, rpmfile.wstr(), ofile.wstr());
			REG_CALL(RegDeleteValue(hkey, rpmfile.tstr()));
		}
		else
		{
			DBGLOG("NotFound RPMEditCopy:'%s'", rpmfile.tstr());
		}
		RegCloseKey(hkey);
	}
}

class cgm2pdfContext
{
public:
	enum ExportMode { Unknown, CreateNew, OverrideExisting, Append };
	cgm2pdfContext(int nargs, wchar_t * const*args)
	{
		int i = 1;
		_mode = Unknown;
		if (wcsicmp(args[i], L"-append") == 0)
		{
			_mode = Append;
			i++;
		}
		_pdfFile = NxlPath(args[i++]);
		_cgmFile = NxlPath(args[i]);
		_loadedFiles.push_back(_cgmFile);
		//cgm file requires view right
		CheckFilePermission(_cgmFile);
		if (_pdfFile.CheckFileExists())
		{
			if (_mode == Unknown)
			{
				_mode = OverrideExisting;
				DBGLOG("OverrideExising-'%s'", _pdfFile.tstr());
				//the input pdf file will be empty by CreateFile(CreateAlways)
				//and the content of cgm file will be exported into the input pdf file directly
				//install rpm workaround fix
				//fix_rpm_createfile_install();
			}
			else
			{
				DBGLOG("Append-'%s'", _pdfFile.tstr());
				_loadedFiles.push_back(_pdfFile);
				if (_pdfFile.HasNxlExtension()) {
					HOOK_API("libsyss.dll", CFI_copy_file_0);
				}
				//the input pdf file will be copied into a new pdf file in temp folder
				//the content of cgm file will be exported into the temp pdf file
				//the input pdf file will be deleted by DeleteFile, and the temp pdf file will be renamed to input pdf file
				fix_rpm_deletefile_install();
				//pdf file requires read right
				CheckFilePermission(_pdfFile);
			}
		}
		else
		{
			_mode = CreateNew;
			DBGLOG("CreateNew-'%s'", _pdfFile.tstr());
			//cgm file will be exported into a new pdf file in temp folder
		}

	}
	~cgm2pdfContext()
	{
		//TODO:when this will happen?
		//TODO:what if this section is not called, for example when the export is actually failed
		clean_rpm_file(_cgmFile);
	}
	void OnFileClosed(const char *file, BOOL isWrite, LONGLONG fileSize)
	{
		if (NxlPath::EndsWith(file, ".pdf")
			|| NxlPath::EndsWith(file, ".cgm")
			|| NxlPath::EndsWith(file, ".nxl"))
		{
			if (isWrite)
			{
				switch (_mode)
				{
				case Append:
					//append mode, a new temp file will be created in temp folder
					if (StrStrIA(file, _pdfFile.str()) == nullptr)//the file may be .pdf.nxl while input is .pdf(ref the CreateFileW hook)
					{
						DBGLOG("TempPDFCreated:'%s'(Size=%lld)", file, fileSize);
						break;
					}
				case CreateNew:
				default:
					DBGLOG("Exported:'%s'(Size=%lld)", file, fileSize);
					on_exported(_loadedFiles, file);
					clean_rpm_file(_pdfFile);
					break;
				}
			}
			else
			{
				DBGLOG("Loaded:'%s'(Size=%lld)", file, fileSize);
			}
		}
	}
private:
	ExportMode _mode;
	NxlPath _cgmFile;
	NxlPath _pdfFile;
	std::vector<NxlPath> _loadedFiles;
};

static cgm2pdfContext *s_cgm2pdfContext;
static void cgm2pdf_on_FileClosed(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if (s_cgm2pdfContext != nullptr)
	{
		s_cgm2pdfContext->OnFileClosed(file, isWrite, fileSize);
	}
}
BOOL cgm2pdf_install()
{
	extern int cmd_nargs;
	extern WCHAR **cmd_args;
	s_cgm2pdfContext = new cgm2pdfContext(cmd_nargs, cmd_args);
	register_FileClosedHandler(cgm2pdf_on_FileClosed);
	return TRUE;
}
void cgm2pdf_uninstall()
{
	unregister_FileClosedHandler(cgm2pdf_on_FileClosed);
	delete s_cgm2pdfContext;
	s_cgm2pdfContext = nullptr;
}