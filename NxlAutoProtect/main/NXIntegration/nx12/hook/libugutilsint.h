/*
	Library:	libugutilsint.dll
*/
#ifndef NXL_LIBUGUTILSINT_DLL_H_INCLUDED
#define NXL_LIBUGUTILSINT_DLL_H_INCLUDED

/* typedef for 2 funcs */
/*
	UI_PREV_save_bookmark_preview
	ApiName:	PREV_save_bookmark_preview
	FullName:	?UI_PREV_save_bookmark_preview@@YAXPEAD@Z
	UnDecorated:	void __cdecl UI_PREV_save_bookmark_preview(char * __ptr64)
	Package = UI_PREV
	Versions(#Ordinal):
		12.0.2.9 (#3035)
*/
#define PREV_save_bookmark_preview_FULLNAME "?UI_PREV_save_bookmark_preview@@YAXPEAD@Z"
typedef void (*pfPREV_save_bookmark_preview)(char * p1	// char * __ptr64
	);

/*
	bui_Export_RapidProto
	ApiName:	bui_Export_RapidProto
	FullName:	?bui_Export_RapidProto@@YAXXZ
	UnDecorated:	void __cdecl bui_Export_RapidProto(void)
	Versions(#Ordinal):
		12.0.2.9 (#3285)
*/
#define bui_Export_RapidProto_FULLNAME "?bui_Export_RapidProto@@YAXXZ"
typedef void (*pfbui_Export_RapidProto)();


#endif
