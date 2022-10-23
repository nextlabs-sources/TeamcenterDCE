#include "nxl_evaluator_defs.h"
#include <utils_system.h>
#include "utils_string.h"
#include "utils_log.h"
#include <utils_mem.h>

#define ATTR_OB_COUNT "CE_ATTR_OBLIGATION_COUNT"
#define ATTR_OB_NAME "CE_ATTR_OBLIGATION_NAME"
#define ATTR_OB_POLICY "CE_ATTR_OBLIGATION_POLICY"
#define ATTR_OB_VALUE "CE_ATTR_OBLIGATION_VALUE"
#define ATTR_OB_VALUE_COUNT "CE_ATTR_OBLIGATION_NUMVALUES"


void obligations_free(obligation_list obs, int nob)
{
	if (obs != NULL)
	{
		int i;
		for (i = 0; i<nob; i++)
		{
			NXL_FREE(obs[i].name);
			NXL_FREE(obs[i].policy);
			ht_free(obs[i].attributes);
		}
		NXL_FREE(obs);
	}
}
int obligation_load(string_list_ro attrs, obligation_list *obligations)
{
	int nob = 0;
	*obligations = NULL;
	if (attrs != NULL
		&& attrs->count > 0)
	{
		//convert attrs to hashtable
		hashtable_p ht = ht_create(attrs->count / 2);
		const char *htStr;
		int iattr = 0;
		for (iattr = 0; (iattr + 1) < attrs->count; iattr += 2)
		{
			DBGLOG("==>[%d/%d]%s", iattr + 1, attrs->count, attrs->items[iattr]);
			DBGLOG("==>[%d/%d]%s", iattr + 2, attrs->count, attrs->items[iattr + 1]);
			ht_set_chars(ht, attrs->items[iattr], attrs->items[iattr + 1]);
		}
		//
		htStr = ht_get_chars(ht, ATTR_OB_COUNT);
		if (htStr != NULL)
		{
			char buff[255];
			nob = strtol(htStr, (char**)&buff, 10);
			if (nob > 0)
			{
				int iob = 1;
				int nfound = 0;
				NXL_ALLOCN(*obligations, struct obligation_s, nob);
				DBGLOG("%d Obligations are expected!", nob);
				for (iob = 1; iob <= nob; iob++)
				{
					int nvalues;
					DBGLOG("Loading Obligation[%d/%d]", iob, nob);
					//find name
					sprintf_s(buff, sizeof(buff), ATTR_OB_NAME":%d", iob);
					htStr = ht_get_chars(ht, buff);
					if (htStr == NULL)
					{
						DBGLOG("==>Cannot find the value for '%s'", buff);
						continue;
					}
					DBGLOG("==>%s=%s", buff, htStr);
					(*obligations)[nfound].name = string_dup(htStr);
					//find policy name
					sprintf_s(buff, sizeof(buff), ATTR_OB_POLICY":%d", iob);
					htStr = ht_get_chars(ht, buff);
					if (htStr == NULL)
					{
						DBGLOG("==>Cannot find the value for '%s'", buff);
						continue;
					}
					DBGLOG("==>%s=%s", buff, htStr);
					(*obligations)[nfound].policy = string_dup(htStr);
					sprintf_s(buff, sizeof(buff), ATTR_OB_VALUE_COUNT":%d", iob);
					//find values count
					htStr = ht_get_chars(ht, buff);
					if (htStr == NULL)
					{
						DBGLOG("==>Cannot find the value for '%s'", buff);
						continue;
					}
					DBGLOG("==>%s=%s", buff, htStr);
					nvalues = strtol(htStr, NULL, 10);
					if (nvalues > 0)
					{
						int iattr;
						hashtable_p attrs = ht_create(nvalues / 2);
						const char *htkey;
						const char *htval;
						if (nvalues % 2 != 0)
						{
							DBGLOG("==>The count of values is not even!(%d)", nvalues);
						}
						for (iattr = 1; iattr <= nvalues; iattr += 2)
						{
							sprintf_s(buff, sizeof(buff), ATTR_OB_VALUE":%d:%d", iob, iattr);
							htkey = ht_get_chars(ht, buff);
							sprintf_s(buff, sizeof(buff), ATTR_OB_VALUE":%d:%d", iob, iattr + 1);
							htval = ht_get_chars(ht, buff);
							DBGLOG("==>Values[%d]:%s=%s", iattr, htkey, htval);
							ht_set_chars(attrs, htkey, htval);
						}
						(*obligations)[nfound].attributes = attrs;
					}
					else
					{
						//initial attributes as empty hashtable
						(*obligations)[nfound].attributes = ht_create(0);
					}
					nfound++;
				}//for(iob=1; iob<=nob; iob++)
				nob = nfound;
			}
			else
			{
				DBGLOG("Invalid '"ATTR_OB_COUNT"' value:%s", htStr);
			}
		}
		else
		{
			DBGLOG("'"ATTR_OB_COUNT"' is not found in obligation attributes");
		}
		ht_free(ht);
	}
	DBGLOG("Found %d Obligations", nob);
	return nob;
}
