#include <ctype.h>
#include <internal.h>
#include "utils_mem.h"
#include "utils_log.h"
#include <utils_string.h>

char *string_ncopy(const char *str, const int nchar)
{
	char *newStr = NULL;
	if(str!=NULL 
		&& NXL_ALLOCN(newStr, char, nchar+1)!=NULL)
	{
		strncpy_s(newStr, nchar+1, str, nchar);
		newStr[nchar] = '\0';
	}
	return newStr;
}

char *string_dup(const char *str)
{
	if(str==NULL) return NULL;
	else
	{
		char *result = strdup(str);
#ifdef DEBUG_MEM
		monitor_track(result, "dup", __FILE__, __FUNCTION__, __LINE__);
#endif
		return result;
	}
}
void string_free(char *str)
{
	NXL_FREE(str);
}

BOOL string_isNullOrSpace(const char *str)
{
	if(str!=NULL)
	{
		//is not NULL
		while(isspace((unsigned char)*str))
		{
			str++;
		}
		return (*str=='\0')?TRUE:FALSE;
	}
	return TRUE;
}

NXLAPI char *string_toLower(const char *str)
{
	if(str!=NULL)
	{
		return string_toLower2(string_dup(str));
	}
	return NULL;
}

char *string_toLower2(char *str)
{
	if(str != NULL)
	{
		char *lower = str;
		while((*lower) != '\0')
		{
			*lower = tolower(*lower);
			lower++;
		}
		return str;
	}
	return NULL;
}

char *path_find_ext(char *path)
{
	char *dot = NULL;
	if(path != NULL)
	{
		char *end = path;
		while(*end != '\0') end++;
		dot = end;
		while((--dot) != path)
		{
			switch(*dot)
			{
				case '.':
					return dot;
				case PATH_DELIMITER_CHAR:
					//no dot is found in name
					return end;
				default:
					break;
			}
		}
		return end;
	}
	return dot;
}
char *path_find_name(char *path)
{
	if(path != NULL)
	{
		char *lastSlash = NULL;
		char *p = path;
		while(*p != '\0')
		{
			if(*p == PATH_DELIMITER_CHAR)
			{
				lastSlash = p;
			}
			p++;
		}
		if(lastSlash != NULL)
		{
			return lastSlash;
		}
	}
	return path;
}
NXLAPI BOOL path_remove_name(char *path)
{
	if(path != NULL)
	{
		char *lastSlash = NULL;
		char *p = path;
		while(*p != '\0')
		{
			if(*p == PATH_DELIMITER_CHAR)
			{
				lastSlash = p;
			}
			p++;
		}
		if(lastSlash != NULL)
		{
			lastSlash[0] = '\0';
			return TRUE;
		}
	}
	return FALSE;
}


const char *strings_join(char * const *strings, int count, const char *split)
{
	static char *joined = NULL;
	size_t size = 0;
	size_t splitsize = strlen(split);
	int i;
	NXL_FREE(joined);
	for(i = 0; i < count; i++)
	{
		const char *joining = strings[i];
		if(string_isNullOrSpace(joining))
			continue;

		if(joined == NULL)
		{
			joined = string_dup(joining);
			size = strlen(joined) + 1;
			continue;
		}
		size += splitsize + strlen(joining);
		NXL_REALLOC(joined, char, size);
		strcat_s(joined, size, split);
		strcat_s(joined, size, joining);
	}
	MOCK_FREE(joined);//for cache
	return joined;
}

const char *strings_join_v(const char *split,int argNum, ...)
{
	va_list args;
	int i;
	const char *result;
	char **strings;
	NXL_ALLOCN(strings, char *, argNum);
	va_start(args, argNum);
	for(i = 0;i < argNum; i++)
	{
		strings[i] = va_arg(args, char *);
	}

	va_end(args);
	result = strings_join(strings, argNum, split);
	NXL_FREE(strings);
	return result;
}

void strings_free(char **strings, int cnt)
{
	int i;
	if(strings!=NULL)
	{
		for(i = 0; i<cnt; i++)
		{
			NXL_FREE(strings[i]);
		}
		NXL_FREE(strings);
	}
}

int strings_index_of(char * const *strs, const int size, const char *str)
{
	if(strs!= NULL)
	{
		int i;
		for(i=0; i<size; i++)
		{
			if(strs[i]==NULL)
			{
				if(str==NULL) return i;
			}
			else if(str==NULL) continue;
			else if(strcmp(str, strs[i])==0)
			{
				return i;
			}
		}
	}
	return -1;
}

BOOL string_starts_with(const char *str, const char *prefix)
{
	if(str == NULL)
		return prefix==NULL;
	if(prefix == NULL)
		return TRUE;
	{
		size_t strLen = strlen(str);
		size_t prefLen = strlen(prefix);
		if(prefLen == 0)
			return TRUE;
		else if(strLen < prefLen)
			return FALSE;
		else if(strncmp(str, prefix, prefLen) == 0)
			return TRUE;
		else
			return FALSE;
	}
}

BOOL string_ends_with(const char *str, const char *suffix, int cs)
{
	if(str == NULL)
	{
		return suffix==NULL;
	}
	else if(suffix == NULL)
	{
		return TRUE;
	}
	else
	{
		size_t strLen = strlen(str);
		size_t sufLen = strlen(suffix);
		if(sufLen == 0)
		{
			return TRUE;
		}
		else if(strLen < sufLen)
		{
			return FALSE;
		}
		else if(cs
			&&strcmp((str+(strLen-sufLen)), suffix) == 0)
		{
			//case sensitive
			return TRUE;
		}
		else if(!cs
			&& _stricmp((str+(strLen-sufLen)), suffix)==0)
		{
			//case insensitive
			return TRUE;
		}
	}
	return FALSE;
}

int string_split(const char *str, const char *splitChars, string_list_p refList)
{
	int iStr;
	int strLen;
	int nfound = 0;
	int iStart;
	if(str == NULL
		||(strLen = (int)strlen(str))<=0
		|| refList == NULL)
		return 0;
	for(iStart = 0, iStr = 0; iStr < strLen; iStr++)
	{
		int iSplit;
		for(iSplit = 0; splitChars[iSplit]!='\0'; iSplit++)
		{
			if(str[iStr] == splitChars[iSplit])
			{
				int subLen = iStr - iStart;
				DTLLOG("Found '%c' at %d in '%s'", str[iStr], iStr, str);
				//splitter is not first character
				if(subLen > 0)
				{
					//append left to result
					string_list_add_internal(refList, string_ncopy(str + iStart, subLen));
					nfound++;
				}
				iStart = iStr + 1;
			}
		}
	}
	if(iStr > iStart)
	{		
		//add remainding 
		string_list_add_internal(refList, string_ncopy(str + iStart, iStr - iStart));
		nfound++;
	}
	return nfound;
}
char *string_trim(const char *str)
{
	if(str!=NULL)
	{
		int istart = 0;
		int iend = strlen(str) - 1;
		while(isspace(str[istart])) istart++;
		while(iend>istart && isspace(str[iend])) iend--;
		if(istart <= iend)
		{
			return string_ncopy(str+istart, iend-istart+1);
		}
	}
	return NULL;
}
BOOL string_isTrue(const char *str)
{
	if(!string_isNullOrSpace(str))
	{
		char *trimmed = string_trim(str);
		if(trimmed!=NULL
			&& (_stricmp(trimmed, "true") == 0
			|| _stricmp(trimmed, "t") == 0))
		{
			NXL_FREE(trimmed);
			return TRUE;
		}
		NXL_FREE(trimmed);
	}
	return FALSE;
}
int string_lastIndexOf(const char *str, char c)
{
	int index = -1;
	if(str != NULL)
	{
		int i;
		for(i=0; str[i] != '\0'; i++)
		{
			if(str[i] == c)
			{
				index = i;
			}
		}
	}
	return index;
}
BOOL string_wildcmp(const char *str, const char *wild)
{
	const char *star = NULL;
	const char *cmp = NULL;
	if(str == NULL || wild == NULL)
	{
		return str == wild;
	}
	while(*str)
	{
		switch(*wild)
		{
			case '*':
				if(!*++wild)
				{
					return TRUE; //when wild string ends with *
				}
				//cache the wild string after star
				star = wild;
				//cache the position of the next char of the string
				cmp = str + 1;
				break;
			case '?':
				//compare next char of string and wildstring
				str++;
				wild++;
				break;
			case '\\':
				//escape next char
				wild++;
			case '\0':
			default:
				if(*str == *wild)
				{
					str++;
					wild++;
					break;
				}
				if(star == NULL) return FALSE; //if no cached star
				//rollback to the cached position of wildstring and string
				str = cmp++; // cached string move to next position
				wild = star;
				break;
		}
	}
	//return TRUE if all wild characters are *
	while(*wild == '*') wild++;
	return !*wild;
}