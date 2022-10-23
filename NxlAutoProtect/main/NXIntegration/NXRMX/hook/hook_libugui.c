/*
	Library:	libugui.dll
	Version:	10.0.0.24
*/
#include <hook/hook_defs.h>
#include <hook/libugui.h>
#include <hook/libjadex.h>
#include <NXRMX/nx_contexts.h>
#include <NXRMX/nx_utils.h>
#include <uf_defs.h>

UF_UI_selection_p_t g_selectionUI = NULL;

typedef struct
{
	char file_name[MAX_FSPEC_BUFSIZE];
	char unknownDatas[100];
}*image_data_guess_t_p;
/*
	IMAGE_create_image
	int __cdecl IMAGE_create_image(struct IMAGE_data_s * __ptr64)
*/
static pfIMAGE_create_image IMAGE_create_image_original;
static pfIMAGE_create_image IMAGE_create_image_next;
static int IMAGE_create_image_hooked(image_data_guess_t_p p1)
{
	int ret = 0;
	CALL_START("IMAGE_create_image(p1='%p')", p1);
	CALL_NEXT(ret = IMAGE_create_image_next(p1));
	nx_on_exported_visibles(p1->file_name);
	CALL_END("IMAGE_create_image(p1='%p') returns '%d'", p1, ret);
	return ret;
}

/*
	SEL_select_class_or_inherit
	int __cdecl SEL_select_class_or_inherit(char * __ptr64,char * __ptr64,struct UGUI_selection_s * __ptr64,int)
*/
static pfSEL_select_class_or_inherit SEL_select_class_or_inherit_original;
static pfSEL_select_class_or_inherit SEL_select_class_or_inherit_next;
static int SEL_select_class_or_inherit_hooked(char * p1, char * p2, UF_UI_selection_p_t selectionUI, int p4)
{
	int ret = 0;
	CALL_START("SEL_select_class_or_inherit(p1='%s', p2='%s', UGUI_selection_s='%p', p4='%d')", p1, p2, selectionUI, p4);
	CALL_NEXT(ret = SEL_select_class_or_inherit_next(p1, p2, selectionUI, p4));
	g_selectionUI = selectionUI;
	CALL_END("SEL_select_class_or_inherit(p1='%s', p2='%s', UGUI_selection_s='%p', p4='%d') returns '%d'", p1, p2, selectionUI, p4, ret);
	return ret;
}

/*
	UIFW_create_command
	int __cdecl UIFW_create_command(char const * __ptr64,class UGS::OM::MethodicObject * __ptr64,void * __ptr64)
*/
static pfUIFW_create_command UIFW_create_command_original;
static pfUIFW_create_command UIFW_create_command_next;
static int UIFW_create_command_hooked(char const * p1, CPP_PTR p2, void * p3)
{
	int ret = 0;
	CALL_START("UIFW_create_command(p1='%s', p2='%p', p3)", p1, p2);
	CALL_NEXT(ret = UIFW_create_command_next(p1, p2, p3));
	CALL_END("UIFW_create_command(p1='%s', p2='%p', p3) returns '%d'", p1, p2, ret);
	return ret;
}

#define LIB_UGUI "libugui.dll"
void libugui_hook()
{
	//File->Export->Part->Class Selection
	HOOK_API(LIB_UGUI, SEL_select_class_or_inherit);
	//
	//File->Export->PNG
	//File->Export->JPG
	//File->Export->TIFF
	//File->Export->GIF
	//File->Export->BMP
	HOOK_API(LIB_UGUI, IMAGE_create_image);//create image file
	//HOOK_API(LIB_UGUI, IMAGE_export_operation911);//show dialog
	
	//File->Export->AUTOCAD DXF/DWG
	//File->Export->IGES
	//File->Export->STEP203
	//File->Export->STEP214
	//File->Export->2D Exchange
	// this API is only for showing the dialog
	// the export is done in corresponding Creator_commit@libjadex.dll
	//	DxfdwgCreator_commit
	//	IgesCreator_commit
	//	Step203Creator_commit
	//	Step214Creator_commit
	//	NXTo2dCreator_commit
	HOOK_API(LIB_UGUI, UIFW_create_command);
}
