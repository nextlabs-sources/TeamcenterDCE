/*
	Library:	libassy.dll
*/
#ifndef NXL_LIBASSY_DLL_H_INCLUDED
#define NXL_LIBASSY_DLL_H_INCLUDED

/* typedef for 3 funcs */
/*
	ASSY_clone_add_assembly
	ApiName:	ASSY_clone_add_assembly
	FullName:	?ASSY_clone_add_assembly@@YAHPEAVASSY_clone@UGS@@PEBDPEAUUF_PART_load_status_s@@@Z
	UnDecorated:	int __cdecl ASSY_clone_add_assembly(class UGS::ASSY_clone * __ptr64,char const * __ptr64,struct UF_PART_load_status_s * __ptr64)
	Package = ASSY
	Versions(#Ordinal):
		1953.1700.0.0 (#2948)
*/
#define ASSY_clone_add_assembly_FULLNAME "?ASSY_clone_add_assembly@@YAHPEAVASSY_clone@UGS@@PEBDPEAUUF_PART_load_status_s@@@Z"
typedef int (*pfASSY_clone_add_assembly)(CPP_PTR p1	// class UGS::ASSY_clone * __ptr64
	,char const * p2	// char const * __ptr64
	,CPP_PTR p3	// struct UF_PART_load_status_s * __ptr64
	);

/*
	ASSY_perform_clone
	ApiName:	ASSY_perform_clone
	FullName:	?ASSY_perform_clone@@YAHPEAVASSY_clone@UGS@@PEAUUF_CLONE_naming_failures_s@@1@Z
	UnDecorated:	int __cdecl ASSY_perform_clone(class UGS::ASSY_clone * __ptr64,struct UF_CLONE_naming_failures_s * __ptr64,struct UF_CLONE_naming_failures_s * __ptr64)
	Package = ASSY
	Versions(#Ordinal):
		1953.1700.0.0 (#3208)
*/
#define ASSY_perform_clone_FULLNAME "?ASSY_perform_clone@@YAHPEAVASSY_clone@UGS@@PEAUUF_CLONE_naming_failures_s@@1@Z"
typedef int (*pfASSY_perform_clone)(CPP_PTR p1	// class UGS::ASSY_clone * __ptr64
	,CPP_PTR p2	// struct UF_CLONE_naming_failures_s * __ptr64
	,CPP_PTR p3	// struct UF_CLONE_naming_failures_s * __ptr64
	);

/*
	BKM_save_bookmark
	ApiName:	BKM_save_bookmark
	FullName:	?BKM_save_bookmark@@YAHPEBDPEAUBKM_save_options_s@@@Z
	UnDecorated:	int __cdecl BKM_save_bookmark(char const * __ptr64,struct BKM_save_options_s * __ptr64)
	Package = BKM
	Versions(#Ordinal):
		1953.1700.0.0 (#3767)
*/
#define BKM_save_bookmark_FULLNAME "?BKM_save_bookmark@@YAHPEBDPEAUBKM_save_options_s@@@Z"
typedef int (*pfBKM_save_bookmark)(char const * p1	// char const * __ptr64
	,CPP_PTR p2	// struct BKM_save_options_s * __ptr64
	);


#endif
