#include <Windows.h>
#include <Shlwapi.h>
#include <hook/hook_defs.h>
#include <NXRMX/nx_contexts.h>
#include "fcc_proxy.h"

/*
	FCC_DownloadFilesFromPLM
*/
static pfFCC_DownloadFilesFromPLM FCC_DownloadFilesFromPLM_original = NULL;
static pfFCC_DownloadFilesFromPLM FCC_DownloadFilesFromPLM_next = NULL;
static int FCC_DownloadFilesFromPLM_hooked (
	const char *policy,
	int nUids,
	const char **uids,
	pfFCCCallBack calback,
	const void *clientObject,
	char ** const localFiles,			//INPUT ARRAY for output
	int ** const fails					//INPUT ARRAY for output
	)
{
	int i, ret;
	CALL_START("FCC_DownloadFilesFromPLM(policy='%s', nUids=%d)", policy, nUids);

	CALL_NEXT(ret = FCC_DownloadFilesFromPLM_next(policy, nUids, uids, calback, clientObject, localFiles, fails));
	for(i=0; i<nUids; i++)
	{
		DBGLOG("==>[%d/%d]Uid='%s' File='%s'", i+1, nUids, uids[i], localFiles[i]);
		nx_on_fcc_downloaded(localFiles[i]);
	}

	CALL_END("FCC_DownloadFilesFromPLM(policy='%s', nUids=%d) returns %d", policy, nUids, ret);
	return ret;
}
/*
	FCC_DownloadFilesToLocation
*/
static pfFCC_DownloadFilesToLocation FCC_DownloadFilesToLocation_original = NULL;
static pfFCC_DownloadFilesToLocation FCC_DownloadFilesToLocation_next = NULL;
static int FCC_DownloadFilesToLocation_hooked(
	char const * policy,
	int nUids,
	char const ** const uids,
	pfFCCCallBack callback,
	void const * clientobject,
	char const * directoryPath,
	char const * * const localFiles,	//OUTPUT ARRAY
	int * const fails					//OUTPUT ARRAY
	)
{
	int i, ret;
	CALL_START("FCC_DownloadFilesToLocation(policy='%s', nUids=%d directory='%s')", policy, nUids, directoryPath);
	
	CALL_NEXT(ret = FCC_DownloadFilesToLocation_next(policy, nUids, uids, callback, clientobject, directoryPath, localFiles, fails));
	for(i=0; i<nUids; i++)
	{
		DBGLOG("==>[%d/%d]Uid='%s' File='%s'", i+1, nUids, uids[i], localFiles[i]);
		nx_on_fcc_downloaded(localFiles[i]);
	}

	CALL_END("FCC_DownloadFilesToLocation(policy='%s', nUids=%d directory='%s') returns %d", policy, nUids, directoryPath, ret);
	return ret;
}
/*
	FCC_RegisterTicket
*/
static pfFCC_RegisterTicket FCC_RegisterTicket_original = NULL;
static pfFCC_RegisterTicket FCC_RegisterTicket_next = NULL;
static int FCC_RegisterTicket_hooked(
	char const *ticket, 
	char **ticketUid		//OUTPUT
	)
{
	int ret;
	ret = FCC_RegisterTicket_next(ticket, ticketUid);
	TRACELOG("FCC_RegisterTicket returns %d UID='%s'(INPUT:Ticket=%s)", ret, *ticketUid, ticket);
	return ret;
}
/*
	FCC_RegisterTickets
*/
static pfFCC_RegisterTickets FCC_RegisterTickets_original = NULL;
static pfFCC_RegisterTickets FCC_RegisterTickets_next = NULL;
static int FCC_RegisterTickets_hooked(
	int nTickets,
	char const * * const tickets,
	char * * const ticketUids,		//INPUT ARRAY for output
	int * const fails				//INPUT ARRAY for output
	)
{
	int i, ret;
	ret = FCC_RegisterTickets_next(nTickets, tickets, ticketUids, fails);
	TRACELOG("FCC_RegisterTickets RETURNS %d", ret);
	for(i=0; i<nTickets; i++)
	{
		TRACELOG("==>[%d/%d]TicketUid='%s' INPUT:Ticket=%s", i+1, nTickets, ticketUids[i], tickets[i]);
	}
	return ret;
}
/*
	FCC_UnRegisterTicket
*/
static pfFCC_UnRegisterTicket FCC_UnRegisterTicket_original = NULL;
static pfFCC_UnRegisterTicket FCC_UnRegisterTicket_next = NULL;
static int FCC_UnRegisterTicket_hooked(char const *ticketUid)
{
	int ret = FCC_UnRegisterTicket_next(ticketUid);
	TRACELOG("FCC_UnRegisterTicket returns %d (INPUT:TicketUID=%s)", ret, ticketUid);
	return ret;
}
/*
	FCC_UnRegisterTickets
*/
static pfFCC_UnRegisterTickets FCC_UnRegisterTickets_original = NULL;
static pfFCC_UnRegisterTickets FCC_UnRegisterTickets_next = NULL;
static int FCC_UnRegisterTickets_hooked(
	int nTickets,
	char * * const ticketUids,
	int * const fails		//INPUT ARRAY for output
	)
{
	int i;
	int ret = FCC_UnRegisterTickets_next(nTickets, ticketUids, fails);
	TRACELOG("FCC_UnRegisterTickets returns %d", ret);
	for(i=0; i<nTickets; i++)
	{
		TRACELOG("==>TicketUIDs[%d/%d]=%s", i+1, nTickets, ticketUids[i]);
	}
	return ret;
}
/*
	FCC_UploadFilesToPLM
*/
static pfFCC_UploadFilesToPLM FCC_UploadFilesToPLM_original = NULL;
static pfFCC_UploadFilesToPLM FCC_UploadFilesToPLM_next = NULL;
static int FCC_UploadFilesToPLM_hooked(
	int nUids,
	char const * * const uids,
	pfFCCCallBack callback,
	void const * clientObject,
	char const * * const files,
	int nFiles,
	char * * const volumeIDs,		//INPUT ARRAY for output,
	int * const fails				//INPUT ARRAY for output
	)
{
	int i, ret;
	CALL_START("FCC_UploadFilesToPLM(nUids=%d callback=%p clientObject=%p nFiles=%d)", nUids, callback, clientObject, nFiles);
	for(i=0; i<nUids; i++)
	{
		DBGLOG("==>[%d/%d]Uid='%s' File='%s'", i+1, nUids, uids[i], files[i]);
		nx_on_fcc_uploading(files[i]);
	}
	CALL_NEXT(ret = FCC_UploadFilesToPLM_next(nUids, uids, callback, clientObject, files, nFiles, volumeIDs, fails));
	for(i=0; i<nFiles; i++)
	{
		DBGLOG("==>[%d/%d]VolumeId='%s' Fail=%d", i+1, nFiles, volumeIDs[i], fails[i]);
		nx_on_fcc_uploaded(files[i], volumeIDs[i]);
	}
	CALL_END("FCC_UploadFilesToPLM(nUids=%d callback=%p clientObject=%p nFiles=%d) returns %d", nUids, callback, clientObject, nFiles, ret);
	return ret;
}
/*
	FCC_UploadFileToPLM
*/
static pfFCC_UploadFileToPLM FCC_UploadFileToPLM_original = NULL;
static pfFCC_UploadFileToPLM FCC_UploadFileToPLM_next = NULL;
static int FCC_UploadFileToPLM_hooked(
	char const * uid,
	pfFCCCallBack callback,
	void const * clientObject,
	char const * filePath,
	char * * volumeId		//OUTPUT
	)
{
	int ret;
	CALL_START("FCC_UploadFileToPLM(uid='%s' callback=%p clientObject=%p filePath='%s' *volumeId='%s')", uid, callback, clientObject, filePath, *volumeId);
	nx_on_fcc_uploading(filePath);
	CALL_NEXT(ret = FCC_UploadFileToPLM_next(uid, callback, clientObject, filePath, volumeId));
	nx_on_fcc_uploaded(filePath, *volumeId);
	CALL_END("FCC_UploadFileToPLM(uid='%s' callback=%p clientObject=%p filePath='%s' *volumeId='%s') returns %d", uid, callback, clientObject, filePath, *volumeId, ret);
	return ret;
}

static BOOL fcc_hooked = FALSE;
static void fcc_hook_internal(HMODULE mProxy)
{
	if(!fcc_hooked && mProxy!=NULL)
	{
		HOOK_byName(FCC_DownloadFilesFromPLM, mProxy);
		HOOK_byName(FCC_DownloadFilesToLocation, mProxy);
		//HOOK_byName(FCC_RegisterTicket, mProxy);
		//HOOK_byName(FCC_RegisterTickets, mProxy);
		HOOK_byName(FCC_UploadFilesToPLM, mProxy);
		HOOK_byName(FCC_UploadFileToPLM, mProxy);
		//HOOK_byName(FCC_UnRegisterTicket, mProxy);
		//HOOK_byName(FCC_UnRegisterTickets, mProxy);
		fcc_hooked = TRUE;
	}
}
void fcc_unhook()
{
	if(fcc_hooked)
	{
		//UNHOOK(FCC_DownloadFilesFromPLM);
		//UNHOOK(FCC_DownloadFilesToLocation);
		//UNHOOK(FCC_RegisterTicket);
		//UNHOOK(FCC_RegisterTickets);
		UNHOOK(FCC_UploadFilesToPLM);
		UNHOOK(FCC_UploadFileToPLM);
		//UNHOOK(FCC_UnRegisterTicket);
		//UNHOOK(FCC_UnRegisterTickets);

		fcc_hooked = FALSE;
	}
}
void fcc_hook()
{
	fcc_hook_internal(GetModuleHandle(FCC_PROXY_DLL));
}