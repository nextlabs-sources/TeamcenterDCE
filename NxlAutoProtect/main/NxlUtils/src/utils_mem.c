#include <stdio.h>
#include <stdlib.h>
#include "utils_log.h"
#include "utils_mem.h"

typedef struct mem_node_s
{
	const void *pointer;
	const char *name;
	struct mem_node_s *next;
}*mem_node_p;

static unsigned long mem_allocated = 0;
static mem_node_p rootNode = NULL;

static int add_node(const void *p, const char *name)
{
	if(p != NULL)
	{
		int num = 0;
		mem_node_p lastNode = NULL;
		mem_node_p newNode = (mem_node_p)malloc(sizeof(struct mem_node_s));
		newNode->pointer = p;
		newNode->name = name;
		newNode->next = NULL;
		//find last
		if(rootNode == NULL)
		{
			//new node is the first node;
			rootNode = newNode;
		}
		else
		{
			num++;
			lastNode = rootNode;
			while(lastNode->next != NULL)
			{
				num++;
				lastNode = lastNode->next;
			}
			lastNode->next = newNode;
		}
		if(num != mem_allocated)
			DTLLOG("('%s'):Inconsitent Node Number:mem_allocated=%d Nodes=%d", name, mem_allocated, num);
		mem_allocated++;
	}

	return mem_allocated;
}
static int remove_node(const void *p)
{
	mem_node_p prevNode = NULL;
	mem_node_p curNode = rootNode;
	while(curNode!=NULL
		&&curNode->pointer!=p)
	{
		prevNode = curNode;
		curNode = curNode->next;
	}
	if(curNode == NULL)
	{
		//not find this pointer
		DBGLOG("Failed to find pointer-%d('%s')", p, p);
	}
	else
	{
		if(prevNode == NULL)
		{
			//first node is deleted
			rootNode = curNode->next;
		}
		else
		{
			prevNode->next = curNode->next;
		}
		free(curNode);
		mem_allocated--;
	}
	return mem_allocated;
}
static int replace_node(const void *oldVal, const void *newVal, const char *newName)
{
	mem_node_p curNode = rootNode;
	while(curNode!=NULL
		&&curNode->pointer!=oldVal)
	{
		curNode = curNode->next;
	}
	if(curNode == NULL)
	{
		//not find this pointer
		DBGLOG("Failed to find pointer-%d('%s')", oldVal, oldVal);
		return add_node(newVal, newName);
	}
	else
	{
		curNode->pointer = newVal;
		curNode->name = newName;
	}
	return mem_allocated;
}

unsigned long monitor_allocated()
{
	return mem_allocated;
}

unsigned long monitor_list()
{
	unsigned long i = 0;
	mem_node_p node = rootNode;
	while(node!=NULL)
	{
		i++;
		DBGLOG("==>[%d/%d]%d:'%s'='%s'", i+1, mem_allocated, node->pointer, node->name, node->pointer);
		node = node->next;
	}
	return i;
}
void monitor_reset()
{
	int i=0;
	if(rootNode != NULL)
	{
		mem_node_p node = rootNode;
		while(node!=NULL)
		{
			mem_node_p nextNode = node->next;
			DBGLOG("[%d/%d]%d-'%s'='%s'", i+1, mem_allocated, node->pointer, node->name, node->pointer);
			free(node);
			node = nextNode;
			i++;
		}
		rootNode = NULL;
	}
	else
	{
		DBGLOG("All Monitor nodes are freed~!");
	}
	if(i != mem_allocated)
	{
		DBGLOG("Inconsistent Node Num:mem_allocated=%d Nodes=%d", mem_allocated, i);
	}
	mem_allocated = 0;
}
void *monitor_track(void *p, const char *name, const char *file, const char *func, int line)
{
	if(p != NULL)
	{
		DBGLOG("%s():TRACKING('%s')=%d Live=%d @file-%s line-%d", func, name, p, add_node(p, name), file, line);
	}
	return p;
}
void monitor_free(void *p, const char *name, const char *file, const char *func, int line)
{
	DBGLOG("%s():RELEASED('%s')=%d Live=%d @file-%s line-%d", func, name, p, remove_node(p), file, line);
}
void *monitor_retrack(void *oldPtr, void *newPtr, const char *name, const char *file, const char *func, int line)
{
	if(oldPtr == NULL && newPtr != NULL)
	{
		monitor_track(newPtr, name, file, func, line);
	}
	else if(oldPtr != NULL && newPtr == NULL)
	{
		monitor_free(oldPtr, name, file, func, line);
	}
	else if(oldPtr != NULL && newPtr != NULL)
	{
		DBGLOG("%s():RE-TRACKING('%s')=%d->%d Live=%d @file-%s line-%d", func, name, oldPtr, newPtr, replace_node(oldPtr, newPtr, name), file, line);
	}
	return newPtr;
}
void monitor_track_array(void **ps, int n, const char *name, const char *file, const char *func, int line)
{
	if(ps!=NULL && n > 0)
	{
		int i;
		for(i=0; i<n; i++)
		{
			DBGLOG("%s():TRACKING(%s[%d/%d])=%d Live=%d @file-%s line-%d", func, name, i+1, n,
				ps[i], add_node(ps[i], name), file, line);
		}
		DBGLOG("%s():TRACKING('%s')=%d Live=%d @file-%s line-%d", func, name,
			ps, add_node(ps, name), file, line);
	}
}