#ifndef NXL_HOOK_FCC_PROXY_H_INCLUDED
#define NXL_HOOK_FCC_PROXY_H_INCLUDED

#define FCC_PROXY_DLL "FCCClientProxyDllv11064.dll"

//int FCC_GetLastError(char const * *);
//int FCC_Init(void * (*)(unsigned __int64),void (*)(void *));
//int FCC_IsFCCEnabled(int *);
//int FCC_RollbackFileUploadedToPLM(char const *,char const *);
//int FCC_Term(void);

typedef int (*pfFCCCallBack)(char const *uid,void *clientObject, __int64 bytesTransferred, __int64 bytesFileSize, int *calbackCount);
/*
	FCC_DownloadFilesFromPLM
int FCC_DownloadFilesFromPLM(char const *,int,char const * * const,int (*)(char const *,void *,__int64,__int64,int *),void const *,char * * const,int * const);
*/
typedef int (*pfFCC_DownloadFilesFromPLM)(
	const char *policy,
	int nUids,
	const char **uids,
	pfFCCCallBack calback,
	const void *clientObject,
	char ** const localFiles,			//INPUT ARRAY for output
	int ** const fails					//INPUT ARRAY for output
	);

/*
	FCC_DownloadFilesToLocation
int FCC_DownloadFilesToLocation(char const *,int,char const * * const,int (*)(char const *,void *,__int64,__int64,int *),void const *,char const *,char const * * const,int * const);
*/
typedef int (*pfFCC_DownloadFilesToLocation)(
	char const * policy,
	int num,
	char const ** const uids,
	pfFCCCallBack calback,
	void const * clientobject,
	char const * directoryPath,
	char const * * const localFiles,	//OUTPUT ARRAY
	int * const fails					//OUTPUT ARRAY
	);

/*
	FCC_RegisterTicket
int FCC_RegisterTicket(char const *,char * *);
*/
typedef int (*pfFCC_RegisterTicket)(
	char const *ticket, 
	char **ticketUid		//OUTPUT
	);

/*
	FCC_RegisterTickets
int FCC_RegisterTickets(int,char const * * const,char * * const,int * const);
*/
typedef int (*pfFCC_RegisterTickets)(
	int nTickets,
	char const * * const tickets,
	char * * const ticketUids,		//INPUT ARRAY for output
	int * const fails				//INPUT ARRAY for output
	);

/*
	FCC_UnRegisterTicket
int FCC_UnRegisterTicket(char const *);
*/
typedef int (*pfFCC_UnRegisterTicket)(char const *ticketUid);

/*
	FCC_UnRegisterTickets
int FCC_UnRegisterTickets(int,char * * const,int * const);
*/
typedef int (*pfFCC_UnRegisterTickets)(
	int nTickets,
	char * * const ticketUids,
	int * const fails		//INPUT ARRAY for output
	);

/*
	FCC_UploadFilesToPLM
int FCC_UploadFilesToPLM(int,char const * * const,int (*)(char const *,void *,__int64,__int64,int *),void const *,char const * * const,int,char * * const,int * const);
*/
typedef int (*pfFCC_UploadFilesToPLM)(
	int nUids,
	char const * * const uids,
	pfFCCCallBack callback,
	void const * clientObject,
	char const * * const files,
	int nFiles,
	char * * const volumeIDs,		//INPUT ARRAY for output,
	int * const fails				//INPUT ARRAY for output
	);

/*
	FCC_UploadFileToPLM
int FCC_UploadFileToPLM(char const *,int (*)(char const *,void *,__int64,__int64,int *),void const *,char const *,char * *);
*/
typedef int (*pfFCC_UploadFileToPLM)(
	char const * uid,
	pfFCCCallBack callback,
	void const * clientObject,
	char const * filePath,
	char * * volumeId		//OUTPUT
	);

#endif