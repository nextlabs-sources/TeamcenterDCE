/*
	Library:	libugmr.dll
*/
#ifndef NXL_LIBUGMR_DLL_H_INCLUDED
#define NXL_LIBUGMR_DLL_H_INCLUDED

typedef const void *TableEntry_PTR;	// class UGS::PDM::TableEntry * __ptr64

/* typedef for 15 funcs */
/*
	UGS::PDM::TableEntry::GetPartTag
	ApiName:	TableEntry_GetPartTag
	FullName:	?GetPartTag@TableEntry@PDM@UGS@@QEBAIXZ
	UnDecorated:	public: unsigned int __cdecl UGS::PDM::TableEntry::GetPartTag(void)const __ptr64
	Package = UGS::PDM::TableEntry
	Versions(#Ordinal):
		2007.1700.0.0 (#3290)
*/
#define TableEntry_GetPartTag_FULLNAME "?GetPartTag@TableEntry@PDM@UGS@@QEBAIXZ"
typedef unsigned int (*pfTableEntry_GetPartTag)(TableEntry_PTR obj	// class UGS::PDM::TableEntry * __ptr64
	);
extern unsigned int TableEntry_GetPartTag(TableEntry_PTR obj	// class UGS::PDM::TableEntry * __ptr64
	);

/*
	UGS::PDM::TableEntry::GetPathName
	ApiName:	TableEntry_GetPathName
	FullName:	?GetPathName@TableEntry@PDM@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::PDM::TableEntry::GetPathName(void)const __ptr64
	Package = UGS::PDM::TableEntry
	Versions(#Ordinal):
		2007.1700.0.0 (#3306)
*/
#define TableEntry_GetPathName_FULLNAME "?GetPathName@TableEntry@PDM@UGS@@QEBAPEBDXZ"
typedef char const * (*pfTableEntry_GetPathName)(TableEntry_PTR obj	// class UGS::PDM::TableEntry * __ptr64
	);
extern char const * TableEntry_GetPathName(TableEntry_PTR obj	// class UGS::PDM::TableEntry * __ptr64
	);

/*
	UGS::PDM::TableEntry::LookupBySpec
	ApiName:	TableEntry_LookupBySpec
	FullName:	?LookupBySpec@TableEntry@PDM@UGS@@SAPEAV123@PEBD@Z
	UnDecorated:	public: static class UGS::PDM::TableEntry * __ptr64 __cdecl UGS::PDM::TableEntry::LookupBySpec(char const * __ptr64)
	Package = UGS::PDM::TableEntry
	Versions(#Ordinal):
		2007.1700.0.0 (#4164)
*/
#define TableEntry_LookupBySpec_FULLNAME "?LookupBySpec@TableEntry@PDM@UGS@@SAPEAV123@PEBD@Z"
typedef TableEntry_PTR (*pfTableEntry_LookupBySpec)(char const * p1	// char const * __ptr64
	);
extern TableEntry_PTR TableEntry_LookupBySpec(char const * p1	// char const * __ptr64
	);

/*
	UGS::PDM::TableEntry::PrintToSyslog
	ApiName:	TableEntry_PrintToSyslog
	FullName:	?PrintToSyslog@TableEntry@PDM@UGS@@QEBAXXZ
	UnDecorated:	public: void __cdecl UGS::PDM::TableEntry::PrintToSyslog(void)const __ptr64
	Package = UGS::PDM::TableEntry
	Versions(#Ordinal):
		2007.1700.0.0 (#4392)
*/
#define TableEntry_PrintToSyslog_FULLNAME "?PrintToSyslog@TableEntry@PDM@UGS@@QEBAXXZ"
typedef void (*pfTableEntry_PrintToSyslog)(TableEntry_PTR obj	// class UGS::PDM::TableEntry * __ptr64
	);
extern void TableEntry_PrintToSyslog(TableEntry_PTR obj	// class UGS::PDM::TableEntry * __ptr64
	);

/*
	UGMGR_FCC_DownloadBytes
	ApiName:	FCC_DownloadBytes
	FullName:	?UGMGR_FCC_DownloadBytes@@YAHPEBDI@Z
	UnDecorated:	int __cdecl UGMGR_FCC_DownloadBytes(char const * __ptr64,unsigned int)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5695)
*/
#define FCC_DownloadBytes_FULLNAME "?UGMGR_FCC_DownloadBytes@@YAHPEBDI@Z"
typedef int (*pfFCC_DownloadBytes)(char const * p1	// char const * __ptr64
	,unsigned int p2	// unsigned int
	);

/*
	UGMGR_FCC_DownloadFilesFromPLM
	ApiName:	FCC_DownloadFilesFromPLM
	FullName:	?UGMGR_FCC_DownloadFilesFromPLM@@YAHPEBDHQEAPEBDP6AH0PEAX_J3PEAH@ZPEBXQEAPEADQEAH@Z
	UnDecorated:	int __cdecl UGMGR_FCC_DownloadFilesFromPLM(char const * __ptr64,int,char const * __ptr64 * __ptr64 const,int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64),void const * __ptr64,char * __ptr64 * __ptr64 const,int * __ptr64 const)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5696)
*/
#define FCC_DownloadFilesFromPLM_FULLNAME "?UGMGR_FCC_DownloadFilesFromPLM@@YAHPEBDHQEAPEBDP6AH0PEAX_J3PEAH@ZPEBXQEAPEADQEAH@Z"
typedef int (*pfFCC_DownloadFilesFromPLM)(char const * p1	// char const * __ptr64
	,int p2	// int
	,char const *const * p3	// char const * __ptr64 * __ptr64 const
	,CALLBACK_PTR p4	// int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64)
	,void const * p5	// void const * __ptr64
	,char *const * p6	// char * __ptr64 * __ptr64 const
	,int const * p7	// int * __ptr64 const
	);

/*
	UGMGR_FCC_DownloadFilesToLocation
	ApiName:	FCC_DownloadFilesToLocation
	FullName:	?UGMGR_FCC_DownloadFilesToLocation@@YAHPEBDHQEAPEBDP6AH0PEAX_J3PEAH@ZPEBX01QEAH@Z
	UnDecorated:	int __cdecl UGMGR_FCC_DownloadFilesToLocation(char const * __ptr64,int,char const * __ptr64 * __ptr64 const,int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64),void const * __ptr64,char const * __ptr64,char const * __ptr64 * __ptr64 const,int * __ptr64 const)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5697)
*/
#define FCC_DownloadFilesToLocation_FULLNAME "?UGMGR_FCC_DownloadFilesToLocation@@YAHPEBDHQEAPEBDP6AH0PEAX_J3PEAH@ZPEBX01QEAH@Z"
typedef int (*pfFCC_DownloadFilesToLocation)(char const * p1	// char const * __ptr64
	,int p2	// int
	,char const *const * p3	// char const * __ptr64 * __ptr64 const
	,CALLBACK_PTR p4	// int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64)
	,void const * p5	// void const * __ptr64
	,char const * p6	// char const * __ptr64
	,char const *const * p7	// char const * __ptr64 * __ptr64 const
	,int const * p8	// int * __ptr64 const
	);

/*
	UGMGR_FCC_RegisterTicket
	ApiName:	FCC_RegisterTicket
	FullName:	?UGMGR_FCC_RegisterTicket@@YAHPEBDPEAPEAD@Z
	UnDecorated:	int __cdecl UGMGR_FCC_RegisterTicket(char const * __ptr64,char * __ptr64 * __ptr64)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5703)
*/
#define FCC_RegisterTicket_FULLNAME "?UGMGR_FCC_RegisterTicket@@YAHPEBDPEAPEAD@Z"
typedef int (*pfFCC_RegisterTicket)(char const * p1	// char const * __ptr64
	,char ** p2	// char * __ptr64 * __ptr64
	);

/*
	UGMGR_FCC_RegisterTickets
	ApiName:	FCC_RegisterTickets
	FullName:	?UGMGR_FCC_RegisterTickets@@YAHHQEAPEBDQEAPEADQEAH@Z
	UnDecorated:	int __cdecl UGMGR_FCC_RegisterTickets(int,char const * __ptr64 * __ptr64 const,char * __ptr64 * __ptr64 const,int * __ptr64 const)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5704)
*/
#define FCC_RegisterTickets_FULLNAME "?UGMGR_FCC_RegisterTickets@@YAHHQEAPEBDQEAPEADQEAH@Z"
typedef int (*pfFCC_RegisterTickets)(int p1	// int
	,char const *const * p2	// char const * __ptr64 * __ptr64 const
	,char *const * p3	// char * __ptr64 * __ptr64 const
	,int const * p4	// int * __ptr64 const
	);

/*
	UGMGR_FCC_RollbackFileUploadedToPLM
	ApiName:	FCC_RollbackFileUploadedToPLM
	FullName:	?UGMGR_FCC_RollbackFileUploadedToPLM@@YAHPEBD0@Z
	UnDecorated:	int __cdecl UGMGR_FCC_RollbackFileUploadedToPLM(char const * __ptr64,char const * __ptr64)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5705)
*/
#define FCC_RollbackFileUploadedToPLM_FULLNAME "?UGMGR_FCC_RollbackFileUploadedToPLM@@YAHPEBD0@Z"
typedef int (*pfFCC_RollbackFileUploadedToPLM)(char const * p1	// char const * __ptr64
	,char const * p2	// char const * __ptr64
	);

/*
	UGMGR_FCC_UnRegisterTicket
	ApiName:	FCC_UnRegisterTicket
	FullName:	?UGMGR_FCC_UnRegisterTicket@@YAHPEBD@Z
	UnDecorated:	int __cdecl UGMGR_FCC_UnRegisterTicket(char const * __ptr64)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5707)
*/
#define FCC_UnRegisterTicket_FULLNAME "?UGMGR_FCC_UnRegisterTicket@@YAHPEBD@Z"
typedef int (*pfFCC_UnRegisterTicket)(char const * p1	// char const * __ptr64
	);

/*
	UGMGR_FCC_UnRegisterTickets
	ApiName:	FCC_UnRegisterTickets
	FullName:	?UGMGR_FCC_UnRegisterTickets@@YAHHQEAPEADQEAH@Z
	UnDecorated:	int __cdecl UGMGR_FCC_UnRegisterTickets(int,char * __ptr64 * __ptr64 const,int * __ptr64 const)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5708)
*/
#define FCC_UnRegisterTickets_FULLNAME "?UGMGR_FCC_UnRegisterTickets@@YAHHQEAPEADQEAH@Z"
typedef int (*pfFCC_UnRegisterTickets)(int p1	// int
	,char *const * p2	// char * __ptr64 * __ptr64 const
	,int const * p3	// int * __ptr64 const
	);

/*
	UGMGR_FCC_UploadFileToPLM
	ApiName:	FCC_UploadFileToPLM
	FullName:	?UGMGR_FCC_UploadFileToPLM@@YAHPEBDP6AH0PEAX_J2PEAH@ZPEBX0PEAPEAD@Z
	UnDecorated:	int __cdecl UGMGR_FCC_UploadFileToPLM(char const * __ptr64,int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64),void const * __ptr64,char const * __ptr64,char * __ptr64 * __ptr64)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5709)
*/
#define FCC_UploadFileToPLM_FULLNAME "?UGMGR_FCC_UploadFileToPLM@@YAHPEBDP6AH0PEAX_J2PEAH@ZPEBX0PEAPEAD@Z"
typedef int (*pfFCC_UploadFileToPLM)(char const * p1	// char const * __ptr64
	,CALLBACK_PTR p2	// int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64)
	,void const * p3	// void const * __ptr64
	,char const * p4	// char const * __ptr64
	,char ** p5	// char * __ptr64 * __ptr64
	);

/*
	UGMGR_FCC_UploadFilesToPLM
	ApiName:	FCC_UploadFilesToPLM
	FullName:	?UGMGR_FCC_UploadFilesToPLM@@YAHHQEAPEBDP6AHPEBDPEAX_J3PEAH@ZPEBX0HQEAPEADQEAH@Z
	UnDecorated:	int __cdecl UGMGR_FCC_UploadFilesToPLM(int,char const * __ptr64 * __ptr64 const,int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64),void const * __ptr64,char const * __ptr64 * __ptr64 const,int,char * __ptr64 * __ptr64 const,int * __ptr64 const)
	Package = UGMGR_FCC
	Versions(#Ordinal):
		2007.1700.0.0 (#5710)
*/
#define FCC_UploadFilesToPLM_FULLNAME "?UGMGR_FCC_UploadFilesToPLM@@YAHHQEAPEBDP6AHPEBDPEAX_J3PEAH@ZPEBX0HQEAPEADQEAH@Z"
typedef int (*pfFCC_UploadFilesToPLM)(int p1	// int
	,char const *const * p2	// char const * __ptr64 * __ptr64 const
	,CALLBACK_PTR p3	// int (__cdecl*)(char const * __ptr64,void * __ptr64,__int64,__int64,int * __ptr64)
	,void const * p4	// void const * __ptr64
	,char const *const * p5	// char const * __ptr64 * __ptr64 const
	,int p6	// int
	,char *const * p7	// char * __ptr64 * __ptr64 const
	,int const * p8	// int * __ptr64 const
	);

/*
	UGMGR_NX_import_named_refs
	ApiName:	NX_import_named_refs
	FullName:	?UGMGR_NX_import_named_refs@@YAHPEAD0000HPEAPEAD11PEAH@Z
	UnDecorated:	int __cdecl UGMGR_NX_import_named_refs(char * __ptr64,char * __ptr64,char * __ptr64,char * __ptr64,char * __ptr64,int,char * __ptr64 * __ptr64,char * __ptr64 * __ptr64,char * __ptr64 * __ptr64,int * __ptr64)
	Package = UGMGR_NX
	Versions(#Ordinal):
		2007.1700.0.0 (#5812)
*/
#define NX_import_named_refs_FULLNAME "?UGMGR_NX_import_named_refs@@YAHPEAD0000HPEAPEAD11PEAH@Z"
typedef int (*pfNX_import_named_refs)(char * p1	// char * __ptr64
	,char * p2	// char * __ptr64
	,char * p3	// char * __ptr64
	,char * p4	// char * __ptr64
	,char * p5	// char * __ptr64
	,int p6	// int
	,char ** p7	// char * __ptr64 * __ptr64
	,char ** p8	// char * __ptr64 * __ptr64
	,char ** p9	// char * __ptr64 * __ptr64
	,int * p10	// int * __ptr64
	);


#endif
