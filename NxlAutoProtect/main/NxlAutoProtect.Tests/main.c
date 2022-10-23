#include <stdio.h>
#include <tcinit/tcinit.h>
#include <pom/pom/pom.h> //POM_get_user(); POM_get_user_id();
#include <constants/constants.h>

#include <tc_utils.h>
#include <dce_mem.h>
#include <utils_log.h>

#define LOG(fmt, ...) nxl_log(TO_CONSOLE, fmt, ##__VA_ARGS__)

void debug_property_constant(const char *type, const char *prop, const char *constant)
{
	char *pConstantValue;
	if(ITK_EVAL(CONSTANTS_get_property_constant_value(constant, type, prop, &pConstantValue)))
	{
		LOG("%s.%s.%s=%s", type, prop, constant, pConstantValue);
		TC_FREE(pConstantValue);
	}
}

 int ITK_user_main(int argc, char* argv[])
 {
	 if(ITK_EVAL(ITK_auto_login()))
	 {
		 char *userName, *userId;
		 tag_t userTag;
		 ITK_CALL(POM_get_user(&userName, &userTag));
		 ITK_CALL(POM_get_user_id(&userId));

		 DBGLOG("==>Current User(%d) - %s(ID:%s)", userTag, userName, userId);
		 LOG("%s(%s), Welcome to PLM", userName, userId);

		 debug_property_constant("Dataset", "ip_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("Dataset", "ip_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("Dataset", "gov_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("Dataset", "gov_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("Item", "ip_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("Item", "ip_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("Item", "gov_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("Item", "gov_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("ItemRevision", "ip_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("ItemRevision", "ip_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("ItemRevision", "gov_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("ItemRevision", "gov_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("Document", "ip_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("Document", "ip_classification", PROPERTY_CONST_VISIBLE);
		 debug_property_constant("Document", "gov_classification", "Fnd0SecurityPropagationEnabled");
		 debug_property_constant("Document", "gov_classification", PROPERTY_CONST_VISIBLE);

		 TC_FREE(userName);
		 TC_FREE(userId);
	 }
	 return 0;

 }