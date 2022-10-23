#include "utils_mem.h"
#include <internal.h>
#include <utils_string.h>
/*
	string_list Module
*/
string_list_p string_list_new()
{
	string_list_p newList = NULL;
	NXL_ALLOC(newList, struct string_list_s);
	newList->count = 0;
	newList->size = 0;
	newList->items = NULL;
	return newList;
}
void string_list_free(string_list_p list)
{
	if(list!=NULL)
	{
		string_list_clear(list);
		NXL_FREE(list);
	}
}
void string_list_clear(string_list_p list)
{
	if(list != NULL)
	{
		int i=0;
		for(i=0; i < list->count; i++)
		{
			NXL_FREE(list->items[i]);
		}
		NXL_FREE(list->items);
		list->size = 0;
		list->count = 0;
	}
}
void string_list_ensure_internal(string_list_p list, unsigned int targetSize)
{
	if(targetSize > 0
		&& list->size < targetSize)
	{
		//calculate size
		int newSize = list->size==0
			? targetSize
			: (list->size * 2);
		while(newSize < targetSize)
		{
			newSize *=2;
		}
		//enlarge
		NXL_REALLOC(list->items, char *, newSize);
		list->size = newSize;
	}
}
//fo string_list_* only
int string_list_add_private(string_list_p list, char *newItem)
{
	int index = list->count;
	string_list_ensure_internal(list, list->count + 1);
	list->items[index] = newItem;
	list->count += 1;
	return index;
}
//for internal library only
int string_list_add_internal(string_list_p list, char *newItem)
{
	if(list != NULL)
	{
		return string_list_add_private(list, newItem);
	}
	return -1;
}
BOOL string_list_set_internal(string_list_p list, char *newItem, int index)
{
	char *oldItem = list->items[index];
	NXL_FREE(oldItem);
	list->items[index] = newItem;
	return TRUE;
}

int string_list_add(string_list_p list, const char *newItem)
{
	if(list != NULL)
	{
		return string_list_add_private(list, string_dup(newItem));
	}
	return -1;
}
BOOL string_list_set(string_list_p list, const char *newItem, int index)
{
	if(list != NULL
		&& index >=0
		&& index < list->count)
	{
		char *dupItem = string_dup(newItem);
		if(string_list_set_internal(list, dupItem, index))
		{
			return TRUE;
		}
		string_free(dupItem);
	}
	return FALSE;
}
int string_list_index_of(string_list_p list, const char *item)
{
	if(list!=NULL)
	{
		return strings_index_of(list->items, list->count, item);
	}
	return -1;
}
BOOL string_list_removeAt(string_list_p list, int index)
{
	if(list != NULL
		&& index >= 0
		&& index < list->count)
	{
		int i;
		string_free(list->items[index]);
		list->count--;
		//move other items
		for(i=index; i<list->count; i++)
		{
			list->items[i] = list->items[i+1];
		}
		return TRUE;
	}
	return FALSE;
}