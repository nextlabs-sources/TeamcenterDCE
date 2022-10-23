/*
	Library:	liblwks.dll
*/
#ifndef NXL_LIBLWKS_DLL_H_INCLUDED
#define NXL_LIBLWKS_DLL_H_INCLUDED

/* typedef for 2 funcs */
/*
	SH_vrml
	ApiName:	SH_vrml
	FullName:	?SH_vrml@@YAHPEADNH@Z
	UnDecorated:	int __cdecl SH_vrml(char * __ptr64,double,int)
	Package = SH
	Versions(#Ordinal):
		1980.1700.0.0 (#1284)
*/
#define SH_vrml_FULLNAME "?SH_vrml@@YAHPEADNH@Z"
typedef int (*pfSH_vrml)(char * p1	// char * __ptr64
	,double p2	// double
	,int p3	// int
	);

/*
	WEB_main
	ApiName:	WEB_main
	FullName:	?WEB_main@@YAXHHPEBD0@Z
	UnDecorated:	void __cdecl WEB_main(int,int,char const * __ptr64,char const * __ptr64)
	Package = WEB
	Versions(#Ordinal):
		1980.1700.0.0 (#1754)
*/
#define WEB_main_FULLNAME "?WEB_main@@YAXHHPEBD0@Z"
typedef void (*pfWEB_main)(int p1	// int
	,int p2	// int
	,char const * p3	// char const * __ptr64
	,char const * p4	// char const * __ptr64
	);


#endif
