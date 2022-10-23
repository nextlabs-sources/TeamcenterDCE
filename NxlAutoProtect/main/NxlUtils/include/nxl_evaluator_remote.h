#ifndef NXL_UTILS_EVALUATOR_REMOTE_H_INCLUDED
#define NXL_UTILS_EVALUATOR_REMOTE_H_INCLUDED
#include "nxl_evaluator_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/*
		use jpc as policy evaluation server
		if remoteIP is null or empty, the value in registry HKLM\\SOFTWARE\\NextLabs\\PolicyController:Host will be used
		this APi will search nlJavaSdk2.jar in following order
			1, the same directory with NxlUtils.dll 
			2, %DCE_ROOT%/shared_libs/
	*/
NXLAPI eval_result_t remote_evaluate(const char *remoteIP, const char *action,
	eval_app_ro app, eval_user_ro user, eval_res_ro src, 
	obligation_list *obligations, int *nob);//Output

typedef struct pep_agent_properties_s {
	const char * pdpHost;
	const char * pdpPort;
	const char * pdpHttps;
	const char * pdpIgnoreHttps;
	const char * oauth2Host; 
	const char * oauth2Port;
	const char * oauth2Https; 
	const char * oauth2ClientId; 
	const char * oauth2ClientSecret;
	BOOL oauth2ClientSecretEncrypted;
} pep_agent_properties_t, *pep_agent_properties_p;


NXLAPI eval_result_t openaz_evaluate(pep_agent_properties_p agentProperties, const char *action,
	eval_app_ro app, eval_user_ro user, eval_res_ro src,
	obligation_list *obligations, int *nob);//Output

#ifdef __cplusplus
}
#endif

#endif
