#include "test.h"
#include "nxl_evaluator_remote.h"
#include "utils_mem.h"
#include "utils_jni.h"

BOOL in_array(const char *tar, const char *arr[], int n)
{
	int i;
	for(i=0; i<n; i++)
	{
		if((tar==NULL&&arr[i]==NULL))
			return TRUE;
		else if(strcmp(tar, arr[i])==0)
			return TRUE;
	}
	return FALSE;
}

result_t assert_obligation_drm(obligation_p obligation)
{
	const char *val;
	const char *taggingValues[] = {"User-Defined","Specific-Property","System-Properties"};
	const char *nameValues[] = {"ip_classification","user"};
	const char *valValues[] = {"secret","super-secret","top-secret", "infodba", "tcadmin", "guest"};
	result_t ret;
	INIT(ret);

	ASSERT_STR(obligation->name, "TCDRM", ret, 0);
	ASSERT(obligation->attributes->count, 4, ret, 0);
	//verify tagging type
	val = ht_get_chars(obligation->attributes, "Tagging");
	ASSERT((val!=NULL&&in_array(val, taggingValues, sizeof(taggingValues)/sizeof(char*))), TRUE, ret, __LINE__);
	//verify Name
	val = ht_get_chars(obligation->attributes, "Name");
	ASSERT((val==NULL||!in_array(val, nameValues, sizeof(nameValues)/sizeof(char*))), FALSE, ret, __LINE__);
	//verify Name
	val = ht_get_chars(obligation->attributes, "Value");
	ASSERT((val==NULL||!in_array(val, valValues, sizeof(valValues)/sizeof(char*))), FALSE, ret, __LINE__);

	return ret;
}

result_t test_evaluator()
{
	struct eval_app_s app;
	struct eval_user_s infodba, tcadmin, guest;
	struct eval_res_s secret, tSecret, sSecret;
	kvl_p secretAttributes, tSecretAttributes, sSecretAttributes;
	const char *action = "RIGHT_EXECUTE_WORKFLOW";
	struct eval_user_s *users[] = {&infodba, &tcadmin, &guest,&infodba, &tcadmin, &guest,&infodba, &tcadmin, &guest};
	struct eval_res_s *resources[] = {&secret, &secret, &secret, &tSecret, &tSecret, &tSecret, &sSecret, &sSecret, &sSecret};
	//int evalResults[] = {EVAL_ALLOW, EVAL_ALLOW, EVAL_ALLOW, EVAL_ALLOW, EVAL_DENY, EVAL_DENY, EVAL_ALLOW, EVAL_ALLOW, EVAL_DENY};
	int nobligations[] = {4, 3, 2, 4, 0, 0, 4, 3, 0};
	int i;
	int n = sizeof(users)/sizeof(struct eval_user_s);
	result_t ret;
	INIT(ret);

	//initialize
	app.name = "NxlUtils.Test";
	app.url = NULL;
	app.attributes = NULL;

	infodba.name = "infodba";
	infodba.id = "infodba";
	infodba.attributes = NULL;
	tcadmin.name = "tcadmin";
	tcadmin.id = "tcadmin";
	tcadmin.attributes = NULL;
	guest.name = "guest";
	guest.id = "guest";
	guest.attributes = NULL;

	secret.name = "SecretSrc";
	secret.type = RES_TYPE_PORTAL;
	secretAttributes = kvl_new(1);
	kvl_add(secretAttributes, "ip_classification", "secret");
	secret.attributes = secretAttributes;

	tSecret.name = "topSecretSrc";
	tSecret.type = RES_TYPE_PORTAL;
	tSecretAttributes = kvl_new(1);
	kvl_add(tSecretAttributes, "ip_classification", "top-secret");
	tSecret.attributes = tSecretAttributes;
	
	sSecret.name = "SuperSecretSrc";
	sSecret.type = RES_TYPE_PORTAL;
	sSecretAttributes = kvl_new(1);
	kvl_add(sSecretAttributes, "ip_classification", "super-secret");
	sSecret.attributes = sSecretAttributes;
	
	{
		const char *remoteIP = "10.23.58.157";//jpc01
		for(i=0; i<n; i++)
		{
			obligation_list obligations;
			int nob;
			int j;
			//DO EVALUATION
			//int result = 
				remote_evaluate(remoteIP, action,
				&app, users[i], resources[i], &obligations, &nob);
			//ASSERT(result, evalResults[i], ret, i);
			ASSERT(nob, nobligations[i], ret, i);
			for(j=0; j<nob; j++)
			{
				//verify
				if(strcmp(obligations[j].name, "TCDRM")==0)
				{
					SUBRUN(assert_obligation_drm(&obligations[j]), ret);
				}
				else
				{
					OUTPUT("%s():Unexpected Obligation:%s(%s)", __FUNCTION__, obligations[j].name, obligations[j].policy);
				}
			}
			obligations_free(obligations, nob);
		}
	}

	kvl_free(secretAttributes);
	kvl_free(tSecretAttributes);
	kvl_free(sSecretAttributes);
	return ret;
}
result_t test_openaz()
{
	struct eval_app_s app;
	struct eval_user_s infodba, tcadmin, guest;
	struct eval_res_s secret, tSecret, sSecret, workflow;
	kvl_p secretAttributes, tSecretAttributes, sSecretAttributes, workflowAttributes;
	const char *action = "RIGHT_EXECUTE_REMOVEPROTECTION";
	struct eval_user_s *users[] = { &infodba, &tcadmin, &guest, &infodba, &tcadmin, &guest,&infodba, &tcadmin, &guest };
	struct eval_res_s *resources[] = { &secret, &workflow, &secret, &tSecret, &tSecret, &tSecret, &sSecret, &sSecret, &sSecret };
	//int evalResults[] = {EVAL_ALLOW, EVAL_ALLOW, EVAL_ALLOW, EVAL_ALLOW, EVAL_DENY, EVAL_DENY, EVAL_ALLOW, EVAL_ALLOW, EVAL_DENY};
	int nobligations[] = { 4, 3, 2, 4, 0, 0, 4, 3, 0 };
	int i;
	int n = sizeof(users) / sizeof(struct eval_user_s);
	pep_agent_properties_t properties = {NULL};
	result_t ret;
	INIT(ret);

	//
	properties.pdpHost = "qajpc02";
	properties.pdpPort = "58080";
	properties.pdpHttps = "false";
	properties.pdpIgnoreHttps = "true";

	properties.oauth2Host = "qacc02";
	properties.oauth2Port = "443";
	properties.oauth2Https = "true";
	properties.oauth2ClientId = "apiclient";

#define SECRET_OLD "bp2oetNPei2uKbGID1bX+Q==" // "123456Blue!"
#define SECRET_NEW "qsip44grzWxTC/JaL+S8h8Nl79HBXHz0CJxxJu8IxXE\\="
	properties.oauth2ClientSecret = SECRET_NEW;
	properties.oauth2ClientSecretEncrypted = TRUE;
	//initialize
	app.name = "NxlUtils.Test";
	app.url = NULL;
	app.attributes = NULL;

	infodba.name = "infodba";
	infodba.id = "infodba";
	infodba.attributes = NULL;
	tcadmin.name = "tcadmin";
	tcadmin.id = "tcadmin";
	tcadmin.attributes = NULL;
	guest.name = "guest";
	guest.id = "guest";
	guest.attributes = NULL;

	secret.name = "SecretSrc";
	secret.type = RES_TYPE_PORTAL;
	secretAttributes = kvl_new(1);
	kvl_add(secretAttributes, "ip_classification", "secret");
	secret.attributes = secretAttributes;

	tSecret.name = "topSecretSrc";
	tSecret.type = RES_TYPE_PORTAL;
	tSecretAttributes = kvl_new(1);
	kvl_add(tSecretAttributes, "ip_classification", "top-secret");
	tSecret.attributes = tSecretAttributes;

	sSecret.name = "SuperSecretSrc";
	sSecret.type = RES_TYPE_PORTAL;
	sSecretAttributes = kvl_new(1);
	kvl_add(sSecretAttributes, "ip_classification", "super-secret");
	sSecret.attributes = sSecretAttributes;
	
	workflow.name = "workflow";
	workflow.type = RES_TYPE_PORTAL;
	workflowAttributes = kvl_new(1);
	kvl_add(workflowAttributes, "operation-source", "workflow");
	workflow.attributes = workflowAttributes;

	{
		for (i = 0; i<n; i++)
		{
			obligation_list obligations = NULL;
			int nob = 2;
			int j;
			//DO EVALUATION
			openaz_evaluate(&properties, action,
				&app, users[i], resources[i], &obligations, &nob);
			//ASSERT(result, evalResults[i], ret, i);
			ASSERT(nob, nobligations[i], ret, i);
			for (j = 0; j<nob; j++)
			{
				//verify
				if (strcmp(obligations[j].name, "TCDRM") == 0)
				{
					SUBRUN(assert_obligation_drm(&obligations[j]), ret);
				}
				else
				{
					OUTPUT("%s():Unexpected Obligation:%s(%s)", __FUNCTION__, obligations[j].name, obligations[j].policy);
				}
			}
			obligations_free(obligations, nob);
			//break;
		}
	}

	kvl_free(secretAttributes);
	kvl_free(tSecretAttributes);
	kvl_free(sSecretAttributes);
	return ret;
}