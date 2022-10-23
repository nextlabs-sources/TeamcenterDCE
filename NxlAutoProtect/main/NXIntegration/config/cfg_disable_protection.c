#include <string.h>
#include <nxl_evaluator_remote.h>


#define POLICY_EVAL_ACTION "TCDRM_CONFIGURATION"
#define POLICY_OBLIGATION_NAME "TCDRM_SKIP_PROTECTION"
BOOL nxl_protection_is_enabled()
{
	//for Bajaj provide the ability to disable nextlabs protection during save/save as/export
	static BOOL isenabled = -1;
	if(isenabled < 0)
	{
		//TODO:evaluate for enabled or disable
		eval_result_t evalResult = EVAL_DEFAULT;
		struct eval_app_s app = {0};
		eval_user_ro pUser = default_eval_user();
		struct eval_res_s res = {0};
		isenabled = TRUE;
		//init app
		app.name = PROJECT_NAME;
		//NOTE:resource information is not necessary for this evaluation
		//but java sdk requires some non-empty values for sending evaluation
		res.name = TARGETFILENAME;
		res.type = RES_TYPE_PORTAL;
		//
		obligation_list obs = NULL;
		int nob = 0;

		if((evalResult = remote_evaluate(NULL, POLICY_EVAL_ACTION, &app, pUser, &res, &obs, &nob))
			!= EVAL_DEFAULT)
		{
			if(nob > 0)
			{
				int iob;
				for(iob = 0; iob < nob; iob++)
				{
					if(stricmp(obs[iob].name, POLICY_OBLIGATION_NAME) == 0)
					{
						isenabled = FALSE;
					}
				}
			}
		}
	}
	return isenabled;
}