/*
	Library:	liblwks.dll
	Version:	10.0.0.24
*/
#include <Shlwapi.h>
#include <hook/hook_defs.h>
#include <hook/liblwks.h>
#include <hook/hook_file_closed.h>
#include <NXRMX/nx_contexts.h>
#include <uf_part.h>
#include <utils_string.h>

static const char *s_rootFile = NULL;
void HandleCreatedVrmlFile(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if(isWrite
		&& string_ends_with(file, ".wrl", FALSE))
	{
		DBGLOG("FinishedWriting:'%s'(Size=%lld)", file, fileSize);
		if(s_rootFile == NULL
			|| _stricmp(PathFindFileName(file), PathFindFileName(s_rootFile)) != 0)
		{
			nx_on_exported_fuzzy(file);
		}
	}
}
/*
	SH_vrml
	int __cdecl SH_vrml(char * __ptr64,double,int)
*/
typedef enum export_vrml_cfg_e{
	vrml_ver1 = 0,
	vrml_output_lights = 1,
	vrml_output_materials = 2,
	vrml_output_textures = 4,
	vrml_ver2 = 8,
	vrml_output_background = 16,
	vrml_flythru_format = 32,
	vrml_multi_files = 64,
	vrml_recursive_multi_file = 128
} export_vrml_opt_t;
static pfSH_vrml SH_vrml_original;
static pfSH_vrml SH_vrml_next;
static int SH_vrml_hooked(char * p1, double p2, int p3)
{
	int ret = 0;
	//TODO:
	// p1 = file name NOTE: may be name only
	// p2 = tolerance
	// p3 = combination version and configurations
	//		Ver 1.0 = 0
	//		Ver 2.0 = 8
	//		Output Lights = 1
	//		Output Materials = 2
	//		Output Textures = 4
	//		Output Background = 16
	//		Flythru Format = 32
	//		Multi File Format = 64
	//		Recursive Multi File = 128
	//
	CALL_START("SH_vrml(p1='%s', p2='%lf', p3='%d'[MultiFile=%d RecursiveFile=%d])", p1, p2, p3, p3&vrml_multi_files, p3&vrml_recursive_multi_file);
	if(p3 & vrml_multi_files)
	{
		s_rootFile = p1;
		register_FileClosedHandler(HandleCreatedVrmlFile);
	}
	CALL_NEXT(ret = SH_vrml_next(p1, p2, p3));
	if(p3 & vrml_multi_files)
	{
		//export into multiple files
		s_rootFile = NULL;
		unregister_FileClosedHandler(HandleCreatedVrmlFile);
		nx_on_exported_1to1(UF_PART_ask_display_part(), p1);
	}
	else
	{
		nx_on_exported_visibles(p1);
	}
	CALL_END("SH_vrml(p1='%s', p2='%lf', p3='%d') returns '%d'", p1, p2, p3, ret);
	return ret;
}

/*
	WEB_main
	void __cdecl WEB_main(int,int,char const * __ptr64,char const * __ptr64)
*/
static pfWEB_main WEB_main_original;
static pfWEB_main WEB_main_next;
static void WEB_main_hooked(int p1, int p2, char const * p3, char const * p4)
{
	CALL_START("WEB_main(p1='%d', p2='%d', p3='%s', p4='%s')", p1, p2, p3, p4);
	CALL_NEXT(WEB_main_next(p1, p2, p3, p4));
	//TODO:
	//#define EXPORT_HTML_ASSEMBLY 0
	//#define EXPORT_HTML_EACH_COMPONENT 1
	nx_on_exported_visibles(p3);
	CALL_END("WEB_main(p1='%d', p2='%d', p3='%s', p4='%s')", p1, p2, p3, p4);
}

void liblwks_hook()
{
	//File->Export->VRML...
	HOOK_API("liblwks.dll", SH_vrml);
	//File->Export->Author HTML...
	HOOK_API("liblwks.dll", WEB_main);
}

