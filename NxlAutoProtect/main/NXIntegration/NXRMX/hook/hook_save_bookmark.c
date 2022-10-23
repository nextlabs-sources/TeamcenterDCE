/*
	Library:	libassy.dll
	Version:	10.0.0.24
*/
#include <hook/hook_defs.h>
#include <hook/libassy.h>
#include <hook/libugutilsint.h>
#include <NXRMX/nx_contexts.h>
#include <Shlwapi.h>

/*
	BKM_save_bookmark
	int __cdecl BKM_save_bookmark(char const * __ptr64,struct BKM_save_options_s * __ptr64)
*/
static pfBKM_save_bookmark BKM_save_bookmark_original;
static pfBKM_save_bookmark BKM_save_bookmark_next;
static int BKM_save_bookmark_hooked(char const * p1, CPP_PTR p2)
{
	int ret = 0;
	CALL_START("BKM_save_bookmark(p1='%s', p2='%p')", p1, p2);
	CALL_NEXT(ret = BKM_save_bookmark_next(p1, p2));
	nx_on_exported_all(p1);
	CALL_END("BKM_save_bookmark(p1='%s', p2='%p') returns '%d'", p1, p2, ret);
	return ret;
}
/*
	PREV_save_bookmark_preview
	void __cdecl UI_PREV_save_bookmark_preview(char * __ptr64)
*/
static pfPREV_save_bookmark_preview PREV_save_bookmark_preview_original;
static pfPREV_save_bookmark_preview PREV_save_bookmark_preview_next;
static void PREV_save_bookmark_preview_hooked(char * p1)
{
	CALL_START("PREV_save_bookmark_preview(p1='%s')", p1);
	CALL_NEXT(PREV_save_bookmark_preview_next(p1));
	//TBD:the plmxml content is the whole assembly, but the jpg content is only the visible content
	//nx_on_exported_visibles(imageFile);//jpg tag is decided as content
	nx_on_exported_associated(p1, ".jpg");
	CALL_END("PREV_save_bookmark_preview(p1='%s')", p1);
}

void hook_save_bookmark()
{
	HOOK_API("libassy.dll", BKM_save_bookmark);//enforce plmxml file
	HOOK_API("libugutilsint", PREV_save_bookmark_preview);//enforce preview jpg file
}

