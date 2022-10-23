#include <uf_defs.h>
#include <uf.h>
#include <uf_ui.h>
#include <uf_part.h>
#include <uf_assem.h>
/*nxlutils*/
#include <utils_string.h>
#include <utils_mem.h>
//
#include "nxuc_app.h"
#include <nx_utils.h>
#include <nx_evaluator.h>

int nx_evaluate(const char *action, int isWorkpartOnly)
{
	obligation_list obligations = NULL;
	int nob = 0;
	eval_result_t evalResult = EVAL_DEFAULT;
	if(isWorkpartOnly)
	{
		//TODO:get workpart tag
		tag_t workPart = UF_ASSEM_ask_work_part();
		evalResult = nx_evaluate_deny(action, &workPart, 1, &obligations, &nob, TRUE);
	}
	else
	{
		tags_p partTags = part_list_all();
		if(partTags->count > 0)
		{
			evalResult = nx_evaluate_deny(action, partTags->tags, partTags->count, &obligations, &nob, TRUE);
		}
	}

	if(evalResult == EVAL_DEFAULT)
	{
		nx_dialog_show("NextLabs PC is required!");
	}
	obligations_free(obligations, nob);
	return EVAL_ALLOW == evalResult;
}