#pragma once
#ifdef WNT
#include <basetsd.h>
#endif

#ifdef __linux__
#include <inttypes.h>
#define INT64 int64_t
#endif

#define FSC_POLICY_TCDRM "NEX"
/**
* UTF-8 version of FSC_ReadBytes.
* Performs an FSC random access (segment) download.
* Policy string is Any three binary bytes (or ASCII characters). It is reserved for future use.
* Ticket string inputs are expected to be URL-encoded UTF-8.
* File offset is in bytes from the start of the file.
* The byte count is in bytes. The buffer must be large enough to contain the number of bytes to be read.
* Return value is a non-negative byte count in case of success, or a negative error code if there was an error.
* Error and message text, if not NULL, is returned in UTF-8 encoding.
* All non-NULL error strings must be freed by the caller.
*/
int FSC_ReadBytes_UTF(
	const char* policy,
	const char* readTicket,
	const INT64 fileOffset,
	const int bytesToRead,
	void* buffer,
	char **szErrorUTF8);

#ifdef UNICODE
int FSC_ReadBytes_Wide(
	const char* policy,
	const char* readTicket,
	const INT64 fileOffset,
	const int bytesToRead,
	void* buffer,
	wchar_t **szErrorWide);
#else
int FSC_ReadBytes(  // Returns the number of bytes read, or a negative number if there was an error.
	const char* policy,
	const char* readTicket,
	const INT64 fileOffset,
	const int bytesToRead,
	void* buffer,
	char **szError);
#endif

