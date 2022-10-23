/*
	Library:	libugutilsint.dll
*/

#include <Shlwapi.h>
#include <hook/hook_defs.h>
#include <hook/libugutilsint.h>
#include <hook/libsyss.h>
#include <NXRMX/nx_contexts.h>
#include "utils_nxl.h"
#include <uf_part.h>
#include <utils_windows.h>

/*
	3117:	bui_Export_RapidProto
	ApiName:	bui_Export_RapidProto
	FullName:	?bui_Export_RapidProto@@YAXXZ
	UnDecorated:	void bui_Export_RapidProto(void)	
*/

static pfbui_Export_RapidProto bui_Export_RapidProto_original;
static pfbui_Export_RapidProto bui_Export_RapidProto_next;
static bool libugutilsint_CFI_udsetspec_hooked = FALSE;

static pfCFI_udsetspec CFI_udsetspec_original;
static pfCFI_udsetspec CFI_udsetspec_next;
static std::string stl_filepath;

static int CFI_udsetspec_hooked(char const *p1, unsigned int p2, unsigned int p3, char * * p4, char * * p5)
{
	unsigned int ret = 0;
	CALL_START("CFI_udsetspec(p1='%s', p2='%u', p3='%u', p4='%p', p5='%p')", p1, p2, p3, p4, p5);
	CALL_NEXT(ret = CFI_udsetspec_next(p1, p2, p3, p4, p5));	
	
	if(p1 != NULL && file_exist(p1) && string_ends_with(p1, ".stl", FALSE))
	{
		stl_filepath = p1;
		NXDBG("CFI_udsetspec_hooked cached stl_filepath='%s'", stl_filepath.c_str());
	} 
	CALL_END("CFI_udsetspec(p1='%s', p2='%u', p3='%u', p4='%p', p5='%p') returns %d", p1, p2, p3, p4, p5, ret);
	return ret;
}

static void bui_Export_RapidProto_hooked()
{
	CALL_START("bui_Export_RapidProto()");

	if(!libugutilsint_CFI_udsetspec_hooked)
	{
		HOOK_API("libsyss.dll", CFI_udsetspec);
		libugutilsint_CFI_udsetspec_hooked = true;
	}
	CALL_NEXT(bui_Export_RapidProto_next());	
	
	if(!stl_filepath.empty()) {
				
		/*
			nx_on_exported is called as a workaround because the objects selected from the UI cannot be determined at present.
			Several methods that return UGUI_selection_s have been hooked but the UI selection is not available in the following:

			SEL_select_class, SEL_select_by_class, SEL_init_select_class, SEL_select_bodies, SEL_initialize_selected_object
			SEL_initialize_selected_object_full, SEL_initialize_selection_list, SEL_set_active_selection
			SEL_class_sel_filter_fn //Is called once, but too soon and UI object isn't available then
			SEL_select_by_type, SEL_select_faces, SEL_select_features_dialog, SEL_select_multiple
			SEL_select_single, SEL_ask_filter_sel_obj, SEL__set_object_draggable
					
			Documenting methods already tried if revisited later.
		*/
		NXDBG("bui_Export_RapidProto() will call nx_on_exported_xto1('%s')", stl_filepath.c_str());
		nx_on_exported_xto1(stl_filepath.c_str()); 
		stl_filepath.clear();
	}
	
	if(libugutilsint_CFI_udsetspec_hooked)
	{
		UNHOOK(CFI_udsetspec);
		libugutilsint_CFI_udsetspec_hooked = false;
	}

	CALL_END("bui_Export_RapidProto()");
}

void libugutilsint_hook()
{
	if (nx_get_major_version() < 11)
	{
		HOOK_API("libugutilsint.dll", bui_Export_RapidProto); //Export STL
	}
}