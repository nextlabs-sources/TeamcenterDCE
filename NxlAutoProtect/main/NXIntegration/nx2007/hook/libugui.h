/*
	Library:	libugui.dll
*/
#ifndef NXL_LIBUGUI_DLL_H_INCLUDED
#define NXL_LIBUGUI_DLL_H_INCLUDED

#include <uf_ui_types.h>

/* typedef for 5 funcs */
/*
	IMAGE_create_image
	ApiName:	IMAGE_create_image
	FullName:	?IMAGE_create_image@@YAHPEAUIMAGE_data_s@@@Z
	UnDecorated:	int __cdecl IMAGE_create_image(struct IMAGE_data_s * __ptr64)
	Package = IMAGE
	Versions(#Ordinal):
		2007.1700.0.0 (#11880)
*/
#define IMAGE_create_image_FULLNAME "?IMAGE_create_image@@YAHPEAUIMAGE_data_s@@@Z"
typedef int (*pfIMAGE_create_image)(CPP_PTR p1	// struct IMAGE_data_s * __ptr64
	);

/*
	MB_add_pre_action_callback
	ApiName:	MB_add_pre_action_callback
	FullName:	?MB_add_pre_action_callback@@YAPEAUCALLBACK_registered_fn_s@@P6AXHPEBXPEAX@Z1@Z
	UnDecorated:	struct CALLBACK_registered_fn_s * __ptr64 __cdecl MB_add_pre_action_callback(void (__cdecl*)(int,void const * __ptr64,void * __ptr64),void * __ptr64)
	Package = MB
	Versions(#Ordinal):
		2007.1700.0.0 (#13462)
*/
#define MB_add_pre_action_callback_FULLNAME "?MB_add_pre_action_callback@@YAPEAUCALLBACK_registered_fn_s@@P6AXHPEBXPEAX@Z1@Z"
typedef CPP_PTR (*pfMB_add_pre_action_callback)(CALLBACK_PTR p1	// void (__cdecl*)(int,void const * __ptr64,void * __ptr64)
	,void * p2	// void * __ptr64
	);
extern CPP_PTR MB_add_pre_action_callback(CALLBACK_PTR p1	// void (__cdecl*)(int,void const * __ptr64,void * __ptr64)
	,void * p2	// void * __ptr64
	);

/*
	MB_ask_button_name
	ApiName:	MB_ask_button_name
	FullName:	?MB_ask_button_name@@YAPEBDH@Z
	UnDecorated:	char const * __ptr64 __cdecl MB_ask_button_name(int)
	Package = MB
	Versions(#Ordinal):
		2007.1700.0.0 (#13474)
*/
#define MB_ask_button_name_FULLNAME "?MB_ask_button_name@@YAPEBDH@Z"
typedef char const * (*pfMB_ask_button_name)(int p1	// int
	);
extern char const * MB_ask_button_name(int p1	// int
	);

/*
	SEL_select_class_or_inherit
	ApiName:	SEL_select_class_or_inherit
	FullName:	?SEL_select_class_or_inherit@@YAHPEAD0PEAUUGUI_selection_s@@H@Z
	UnDecorated:	int __cdecl SEL_select_class_or_inherit(char * __ptr64,char * __ptr64,struct UGUI_selection_s * __ptr64,int)
	Package = SEL
	Versions(#Ordinal):
		2007.1700.0.0 (#17225)
*/
#define SEL_select_class_or_inherit_FULLNAME "?SEL_select_class_or_inherit@@YAHPEAD0PEAUUGUI_selection_s@@H@Z"
typedef int (*pfSEL_select_class_or_inherit)(char * p1	// char * __ptr64
	,char * p2	// char * __ptr64
	,UF_UI_selection_p_t p3	// struct UGUI_selection_s * __ptr64
	,int p4	// int
	);

/*
	UIFW_create_command
	ApiName:	UIFW_create_command
	FullName:	?UIFW_create_command@@YAHPEBDPEAVMethodicObject@OM@UGS@@PEAX@Z
	UnDecorated:	int __cdecl UIFW_create_command(char const * __ptr64,class UGS::OM::MethodicObject * __ptr64,void * __ptr64)
	Package = UIFW
	Versions(#Ordinal):
		2007.1700.0.0 (#19679)
*/
#define UIFW_create_command_FULLNAME "?UIFW_create_command@@YAHPEBDPEAVMethodicObject@OM@UGS@@PEAX@Z"
typedef int (*pfUIFW_create_command)(char const * p1	// char const * __ptr64
	,CPP_PTR p2	// class UGS::OM::MethodicObject * __ptr64
	,void * p3	// void * __ptr64
	);


#endif
