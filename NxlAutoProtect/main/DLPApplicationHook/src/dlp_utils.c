#include "utils_log.h"
#include "utils_string.h"
#include "dlp_utils.h"


BOOL file_set_attribute(const char *fileName, DWORD attribute, BOOL isAdd)
{
	if(attribute !=INVALID_FILE_ATTRIBUTES)
	{
		DWORD dwAttrs = GetFileAttributes(fileName); 
		if (dwAttrs!=INVALID_FILE_ATTRIBUTES)
		{
			if(isAdd)
			{
				//adding specific attribute to this file
				if(!(dwAttrs & attribute))
				{
					if(SetFileAttributes(fileName, dwAttrs | attribute))
					{
						return TRUE;
					}
					else
					{
						DBGLOG("==>Failed(%X) to ADD the new FileAttribute(%X) to the file(Attribute=%X)", GetLastError(), attribute, dwAttrs);
					}
				}
			}
			else
			{
				//removing specific attribute from this file
				if(dwAttrs & attribute)
				{
					if(SetFileAttributes(fileName, dwAttrs & (~attribute)))
					{
						return TRUE;
					}
					else
					{
						DBGLOG("==>Failed(%X) to REMOVE the new FileAttribute(%X) to the file(Attribute=%X)", GetLastError(), attribute, dwAttrs);
					}
				}
			}
		}
	}
	return FALSE;
}