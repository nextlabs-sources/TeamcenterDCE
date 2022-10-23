#ifndef NXL_HOOK_FILE_CLOSED_H_INCLUDED
#define NXL_HOOK_FILE_CLOSED_H_INCLUDED
#include <hook\windows\windows_defs.h>

//Event - WriteFileFinished
typedef void (*pfFileClosedHandler)(const char *file, BOOL isWrite, LONGLONG fileSize);
void register_FileClosedHandler(pfFileClosedHandler handler);
void unregister_FileClosedHandler(pfFileClosedHandler handler);

#endif