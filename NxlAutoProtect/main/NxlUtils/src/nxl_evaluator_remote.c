#include "nxl_evaluator_remote.h"
#include "utils_string.h"
#include "utils_log.h"
#include "utils_jni_internal.h"
#include "utils_mem.h"
#include <utils_nxl.h>
#include <utils_system.h>

#define JPC_HOST() getenv("NXL_JPC_HOST")
extern int obligation_load(string_list_ro list, obligation_list *obligations);
/*
JNI related
*/
static BOOL s_initialized = FALSE;
//CEApplication
jclass ceAppCls = NULL;
jmethodID ceAppCtor = NULL;
//CEAttributes
jclass ceAttrsCls = NULL;
jmethodID ceAttrsCtor = NULL;
jmethodID ceAttrsAddMethod = NULL;
jmethodID ceAttrsToArrayMethod = NULL;
//CEEnforcement
jclass ceEnfCls = NULL;
jmethodID ceEnfResponseMethod = NULL;
jmethodID ceEnfObligationsMethod = NULL;
//CEResource
jclass ceResCls = NULL;
jmethodID ceResCtor = NULL;
//CESdk
jclass ceSdkCls = NULL;
jmethodID ceSdkCtor = NULL;
jmethodID ceSdkInitCnnMethod = NULL;
jmethodID ceSdkCloseMethod = NULL;
jmethodID ceSdkCheckResourcesMethod = NULL;
jobject ceSdkObj = NULL;
//CEUser
jclass ceUsrCls = NULL;
jmethodID ceUsrCtor = NULL;

static int env_initialize(JNIEnv *env)
{
	int ret = JNI_OK;
	if(s_initialized == TRUE)
	{
		return ret;
	}
	DBGLOG("Initializing global variables(classId/methodId)");
	//Java Exception
	//if(excepCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&excepCls, env, "java/lang/Throwable");
		
		//Methods
		ret += JNI_getMethod_global(&excepGetMsgMethod, env, 
			excepCls,
			"getMessage",
			"()Ljava/lang/String;");
		ret += JNI_getMethod_global(&excepToStrMethod, env, 
			excepCls,
			"toString",
			"()Ljava/lang/String;");
		ret += JNI_getMethod_global(&excepPrintSTMethod, env, 
			excepCls,
			"printStackTrace",
			"()V");
	}
	//CEApplication
	//if(ceAppCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&ceAppCls, env, "com/nextlabs/destiny/sdk/CEApplication");

		//Constructor
		ret += JNI_getMethod_global(&ceAppCtor, env,
			ceAppCls,
			"<init>",
				"(Ljava/lang/String;"	//App Name
				"Ljava/lang/String;)"	//App Path
				"V");
	}
	//CEAttributes
	//if(ceAttrsCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&ceAttrsCls, env, "com/nextlabs/destiny/sdk/CEAttributes");

		//Constructor
		ret += JNI_getMethod_global(&ceAttrsCtor, env,
			ceAttrsCls,
			"<init>",
			"()V");

		//Methods
		ret += JNI_getMethod_global(&ceAttrsAddMethod, env,
			ceAttrsCls,
			"add",
			"(Ljava/lang/String;"	//key
			"Ljava/lang/String;)V");	//value
		ret += JNI_getMethod_global(&ceAttrsToArrayMethod, env,
			ceAttrsCls,
			"toArray",
			"()[Ljava/lang/String;");
	}
	//CEUser
	//if(ceUsrCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&ceUsrCls, env, "com/nextlabs/destiny/sdk/CEUser");

		//Constructor
		ret += JNI_getMethod_global(&ceUsrCtor, env,
			ceUsrCls,
			"<init>",
			"(Ljava/lang/String;Ljava/lang/String;)V");
	}
	//CEEnforcement
	//if(ceEnfCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&ceEnfCls, env, "com/nextlabs/destiny/sdk/CEEnforcement");
		
		//Methods
		ret += JNI_getMethod_global(&ceEnfResponseMethod, env, 
			ceEnfCls, 
			"getResponseAsString", 
			"()Ljava/lang/String;");
		ret += JNI_getMethod_global(&ceEnfObligationsMethod, env, 
			ceEnfCls,
			"getObligations",
			"()Lcom/nextlabs/destiny/sdk/CEAttributes;");
	}
	//CEResource
	//if(ceResCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&ceResCls, env, "com/nextlabs/destiny/sdk/CEResource");

		//Constructor
		ret += JNI_getMethod_global(&ceResCtor, env, 
			ceResCls,
			"<init>",
			"(Ljava/lang/String;"	// resource name
			"Ljava/lang/String;)V");	//resource type
	}
	//CESdk
	//if(ceSdkCls == NULL)
	{
		//Class
		ret += JNI_getClass_global(&ceSdkCls, env, "com/nextlabs/destiny/sdk/CESdk");

		//Constructor
		ret += JNI_getMethod_global(&ceSdkCtor, env, 
			ceSdkCls,
			"<init>",
			"()V");

		//Methods
		ret += JNI_getMethod_global(&ceSdkInitCnnMethod, env, 
			ceSdkCls, 
			"Initialize",
				"(Lcom/nextlabs/destiny/sdk/CEApplication;"	//app
				"Lcom/nextlabs/destiny/sdk/CEUser;"	//user
				"Ljava/lang/String;"	//ip address
				"I"		//port number default 0
				"I)"	//timeout(int)
				"J");	//return: connection handle(long)
		ret += JNI_getMethod_global(&ceSdkCloseMethod, env,
			ceSdkCls,
			"Close",
				"(J"	//connection handle
				"I"		//timeout
				")V");
		ret += JNI_getMethod_global(&ceSdkCheckResourcesMethod, env, 
			ceSdkCls, 
			"CheckResources",
				"(J"	//connection handle(long)
				"Ljava/lang/String;"	//action name
				"Lcom/nextlabs/destiny/sdk/CEResource;Lcom/nextlabs/destiny/sdk/CEAttributes;"	//source resource and attributes
				"Lcom/nextlabs/destiny/sdk/CEResource;Lcom/nextlabs/destiny/sdk/CEAttributes;"	//target resource and attributes
				"Lcom/nextlabs/destiny/sdk/CEUser;Lcom/nextlabs/destiny/sdk/CEAttributes;"	//user and attributes
				"Lcom/nextlabs/destiny/sdk/CEApplication;Lcom/nextlabs/destiny/sdk/CEAttributes;"	//application and attributes
				"[Ljava/lang/String;"	//recipents(string array)
				"IZII)"	//ip address(int); perform obligation(bool); noise level(int); time out(int)
				"Lcom/nextlabs/destiny/sdk/CEEnforcement;");	//return CEEnforcement;

		//Objects
		if(ceSdkCls != NULL && ceSdkCtor != NULL && ceSdkObj == NULL)
		{
			jobject obj;
			if(JNI_EVAL(obj = (*env)->NewObject(env, ceSdkCls, ceSdkCtor)))
			{
				ret += JNI_make_global(&ceSdkObj, env, obj);
			}
		}
	}
	DBGLOG("Finished with %d", ret);
	if(ret==JNI_OK)
		s_initialized = TRUE;
	return ret;
}
const char *default_jpcsdk()
{
	static char *jarfile = NULL;
	if(jarfile == NULL)
	{
		jarfile = search_dce_lib("nlJavaSDK2.jar");
	}
	return jarfile;
}
const char *default_jpc()
{
	static char *remoteIP = NULL;
	if(remoteIP == NULL)
	{
		remoteIP = string_dup(JPC_HOST());
	}
	return remoteIP;
}
eval_user_ro default_eval_user()
{
	static struct eval_user_s user = {0};
	if(user.name == NULL)
	{
		user_info_ro logonInfo = get_logon_info();
		user.name = logonInfo->name;
		user.id = logonInfo->sid;
		user.attributes = NULL;
	}
	return &user;
}
int jattrs_create(JNIEnv *env, kvl_ro attrs, jobject *jattrs)
{
	extern jclass ceAttrsCls;
	extern jmethodID ceAttrsCtor;
	extern jmethodID ceAttrsAddMethod;
	int iAttr = 0;
	*jattrs = NULL;
	if (env != NULL)
	{
		//create empty app attributes
		JNI_CALL(*jattrs = (*env)->NewObject(env, ceAttrsCls, ceAttrsCtor));
		if (*jattrs != NULL && attrs != NULL)
		{
			if (attrs->count > 0)
			{
				for (iAttr = 0; iAttr<attrs->count; iAttr++)
				{
					jstring jkey = (*env)->NewStringUTF(env, attrs->keys[iAttr]);
					jstring jval = (*env)->NewStringUTF(env, attrs->vals[iAttr]);
					JNI_CALL((*env)->CallObjectMethod(env, *jattrs, ceAttrsAddMethod,
						jkey, jval));
					DELETE_REF(jkey);
					DELETE_REF(jval);
				}
			}
		}
	}
	return iAttr;
}

string_list_p jattrs_to_strings(JNIEnv *env, jobject jattrs)
{
	extern jmethodID ceAttrsToArrayMethod;
	jobject jStrArray;
	string_list_p strs = NULL;
	if (JNI_EVAL(jStrArray = (*env)->CallObjectMethod(env, jattrs, ceAttrsToArrayMethod)))
	{
		int arrCnt = (*env)->GetArrayLength(env, jStrArray);
		if (arrCnt > 0)
		{
			int i;
			strs = string_list_new();
			for (i = 0; i<arrCnt; i++)
			{
				jstring jstr = (jstring)(*env)->GetObjectArrayElement(env, jStrArray, i);
				const char *rawString = (*env)->GetStringUTFChars(env, jstr, NULL);
				DTLLOG("jarray[%d]=%s", i, rawString);
				string_list_add(strs, rawString);
				// Don't forget to call `ReleaseStringUTFChars` when you're done.
				(*env)->ReleaseStringUTFChars(env, jstr, rawString);
			}
		}
		DELETE_REF(jStrArray);
	}
	return strs;
}

jsize jbytearray_copy(JNIEnv* env, jbyteArray buff, jsize istart, jbyteArray srcArray, jsize ifrom, jsize numToCopy)
{
	jboolean isCopy = JNI_FALSE;
	jsize nBuffer = (*env)->GetArrayLength(env, buff);
	jsize nSource = (*env)->GetArrayLength(env, srcArray);
	DBGLOG("Copying %d bytes from source(size=%d from=%d) to buffer(size=%d start=%d)", numToCopy, nSource, ifrom, nBuffer, istart);
	if (numToCopy + ifrom > nSource) {
		DBGLOG("no enough bytes in source array(expect=%d actual=%d)", numToCopy, nSource - ifrom);
		numToCopy = nSource - ifrom;
	}
	if (numToCopy + istart <= nBuffer)
	{
		jbyte* bytes = (*env)->GetByteArrayElements(env, srcArray, &isCopy);
		(*env)->SetByteArrayRegion(env, buff, istart, numToCopy, bytes + ifrom);
		(*env)->ReleaseByteArrayElements(env, srcArray, bytes, JNI_ABORT);
		return numToCopy;
	}
	else
	{
		ERRLOG("buffer is too small");
		return 0;
	}
}

/*
Implementation of API
*/
eval_result_t remote_evaluate(const char *remoteIP, const char *action, 
	eval_app_ro app, eval_user_ro user, eval_res_ro src, 
	obligation_list *obligations, int *nob)
{
	eval_result_t retCode = EVAL_DEFAULT;
	JNIEnv *env = NULL;
	DBGLOG("Evaluating Action-%s on Server-'%s'", action, remoteIP);
	if(obligations != NULL)	*obligations = NULL;
	if(nob != NULL) *nob = 0;
	if(string_isNullOrSpace(remoteIP))
	{
		remoteIP = default_jpc();
		if(string_isNullOrSpace(remoteIP))
		{
			ERRLOG("Remote IP is not valid!");
			return retCode;
		}
	}
	if(string_isNullOrSpace(action))
	{
		ERRLOG("Action is not properly set!");
	}
	else if(app==NULL)
	{
		ERRLOG("Application Attributes is not properly initialized!");
	}
	else if(user==NULL)
	{
		ERRLOG("User Attributes is not properly initialized!");
	}
	else if(src==NULL)
	{
		ERRLOG("Source Resource is not Properly initialized!");
	}
	else
	{
		if(JNI_init(&env, default_jpcsdk()) == JNI_OK)
		{
			//make sure the global variables are loaded
			if(env_initialize(env)!=JNI_OK)
			{
				DBGLOG("Failed to initialize global vars!");
				goto DETACH_JNI;
			}
		}
	}
	if(env != NULL)
	{
		//static connection handler
		extern jobject ceSdkObj;
		jobject cnnHndl = NULL;
		jobject appObj=NULL, appAttrs=NULL;
		jobject srcObj=NULL, srcAttrs=NULL;
		jobject tarObj=NULL, tarAttrs=NULL;
		jobject usrObj=NULL, usrAttrs=NULL;

		DBGLOG("Initializing JNI CEApplication Object(Name=%s Url=%s)...", app->name, app->url);
		{
			if(string_isNullOrSpace(app->name))
			{
				DBGLOG("==>Application Name is not defined!");
			}
			else
			{
				extern jclass ceAppCls;
				extern jmethodID ceAppCtor;
				jstring jname, jurl; 
				const char *appurl = app->url;
				jname = (*env)->NewStringUTF(env, app->name);
				//set default URL
				if(string_isNullOrSpace(appurl))
				{
					appurl = "N.A.";
				}
				jurl = (*env)->NewStringUTF(env, appurl);
				//create app object
				JNI_CALL(appObj = (*env)->NewObject(env, ceAppCls, ceAppCtor, jname, jurl));
				//generate app attributes
				DBGLOG("==>Generated %d Application Attributes", jattrs_create(env, app->attributes, &appAttrs));
				DELETE_REF(jname);
				DELETE_REF(jurl);
			}
		}
		if(appObj == NULL || appAttrs == NULL)
			goto CLEAN_JNI;

		DBGLOG("Initializing JNI CEUser Object(userName='%s' userId='%s')...", user->name, user->id);
		{
			extern jclass ceUsrCls;
			extern jmethodID ceUsrCtor;
			jstring juserName, juserId;
			juserName = (*env)->NewStringUTF(env, user->name);
			juserId = (*env)->NewStringUTF(env, user->id);
			//create user object
			JNI_CALL(usrObj = (*env)->NewObject(env, ceUsrCls, ceUsrCtor, juserName, juserId));
			//generate user attributes
			DBGLOG("==>Generated %d User Attributes", jattrs_create(env, user->attributes, &usrAttrs));
			DELETE_REF(juserName);
			DELETE_REF(juserId);
		}
		if(usrObj==NULL || usrAttrs==NULL)
			goto CLEAN_JNI;

		//Initializing Connection
		DBGLOG("Initializing Remote Connection Handler(Remote IP=%s)...", remoteIP);
		if(appObj != NULL && usrObj != NULL && remoteIP != NULL)
		{
			extern jmethodID ceSdkInitCnnMethod;
			jstring jIP = NULL;
			int portNumber = 0;
			//split remoteIp with host name and port number
			char *hostName = string_dup(remoteIP);
			int iColon;
			if((iColon = string_lastIndexOf(hostName, ':')) > 0)
			{
				char *portStr = hostName + iColon + 1;
				char *end = NULL;
				hostName[iColon] = '\0';
				//convert str to int
				portNumber = (int)strtol(portStr, &end, 10);
				DBGLOG("RemoteIp='%s' HostName='%s' PortStr='%s' PortNumber='%d' End='%s'", remoteIP, hostName, portStr, portNumber, end);
			}
			else
			{
				DBGLOG("RemoteIp='%s' HostName='%s' UseDefaultPort='%d'", remoteIP, hostName,  portNumber);
			}
			jIP = (*env)->NewStringUTF(env, hostName);
			if(JNI_EVAL(cnnHndl = (*env)->CallObjectMethod(env, ceSdkObj, ceSdkInitCnnMethod,
				appObj, usrObj, jIP, portNumber, 10000)))
			{
				DBGLOG("==>Connection Created(%d)", cnnHndl);
			}
			else
			{
				DBGLOG("==>Failed to connect remote PC.");
			}
			DELETE_REF(jIP);
			string_free(hostName);
		}
		if(cnnHndl == NULL)
			goto CLEAN_JNI;
		
		DBGLOG("Initializing JNI CEResource Object(resourceName='%s' type='%s')...", src->name, src->type);
		{
			extern jclass ceResCls;
			extern jmethodID ceResCtor;
			extern jmethodID ceAttrsAddMethod;
			const char *srcurl = kvl_indexer(src->attributes, RES_KEY_URL);
			jstring jresName = (*env)->NewStringUTF(env, src->name);
			jstring jportal = (*env)->NewStringUTF(env, src->type);
			JNI_CALL(srcObj = (*env)->NewObject(env, ceResCls, ceResCtor, jresName, jportal));
			//generate Source Resource attributes
			//NOTE:manually add url attribute to make portal resource url work
			DBGLOG("==>Generated %d Resource Attributes", jattrs_create(env, src->attributes, &srcAttrs));
			DBGLOG("==>Attribute-'"RES_KEY_URL"'=%s", srcurl);
			if(srcurl==NULL)
			{
				jstring jurlKey = (*env)->NewStringUTF(env, RES_KEY_URL);
				JNI_CALL((*env)->CallObjectMethod(env, srcAttrs, ceAttrsAddMethod, 
					jurlKey, jresName));
				DELETE_REF(jurlKey);
				DBGLOG("==>attribute key-'"RES_KEY_URL"' is not defined, set it as resource name-'%s'", src->name);
			}
			DELETE_REF(jresName);
			DELETE_REF(jportal);
		}
		if(srcObj == NULL||srcAttrs == NULL)
			goto CLEAN_JNI;

		//others
		{
			tarObj = NULL;
			tarAttrs = NULL;
		}
		//CESdk.CheckResources()
		{
			extern jobject ceSdkObj;
			extern jmethodID ceSdkCheckResourcesMethod;
			int ipNumber = IP_local_number();
			jobject jresult;
			jstring jop = (*env)->NewStringUTF(env, action);
			DBGLOG("Evaluating...(Operation=%s IpNumber=%d)", action, ipNumber);
			if(JNI_EVAL(jresult = (*env)->CallObjectMethod(env, ceSdkObj, ceSdkCheckResourcesMethod, 
					cnnHndl,
					jop,
					srcObj, srcAttrs, //Source:CEResource and CEAttributes
					tarObj, tarAttrs, //Target:CEResource and CEAttributes
					usrObj, usrAttrs, //User: CEUser and CEAttributes
					appObj, appAttrs, //App: CEApplication and CEAttributes
					NULL, //recipents: string[]
					ipNumber, //ip address: int
					1, //performObligation: bool
					3, //Noise Level: UserAction=3
					30*1000 //timeout
				)))
			{
				//getResponseAsString()
				extern jmethodID ceEnfResponseMethod;
				//getObligations()
				extern jmethodID ceEnfObligationsMethod;
				jobject jObligations;
				jobject jresponse;
				if(JNI_EVAL(jresponse = (*env)->CallObjectMethod(env, jresult, ceEnfResponseMethod)))
				{
					const char *resStr;
					if(JNI_EVAL(resStr = (*env)->GetStringUTFChars(env, jresponse, NULL)))
					{
						DBGLOG("==>Response String:%s", resStr);
						//compare in case insensitive mode
						if(_stricmp(resStr, "ALLOW")==0)
						{
							retCode = EVAL_ALLOW;
						}// is ALLOW
						else
						{
							retCode = EVAL_DENY;
						}
						CALL_DTL((*env)->ReleaseStringUTFChars(env, jresponse, resStr));
					}//GetStringUTFChars()
					DELETE_REF(jresponse);
				}//getResponseString();
				if(obligations == NULL || nob == NULL)
				{
					DBGLOG("==>Skip loading obligations");
				}
				else if(JNI_EVAL(jObligations = (*env)->CallObjectMethod(env, jresult, ceEnfObligationsMethod)))
				{
					string_list_p list = jattrs_to_strings(env, jObligations);
					if(list != NULL)
					{
						DBGLOG("==>Got %d Obligation Attributes from Evaluation Result", list->count);
						*nob = obligation_load(list, obligations);
						string_list_free(list);
					}
					DELETE_REF(jObligations);
				}//getObligations()
				DELETE_REF(jresult);
			}//if(result != NULL)
			DELETE_REF(jop);
		}
CLEAN_JNI:
		//close connection handler
		if(cnnHndl != NULL)
		{
			extern jmethodID ceSdkCloseMethod;
			if(JNI_EVAL((*env)->CallObjectMethod(env, ceSdkObj, ceSdkCloseMethod, 
					cnnHndl, 30*1000 //timeout
					)))
			{
				DBGLOG("Connection(%d) closed.", cnnHndl);
			}
		}
		DELETE_REF(appObj);
		DELETE_REF(appAttrs);
		DELETE_REF(srcObj);
		DELETE_REF(srcAttrs);
		DELETE_REF(tarObj);
		DELETE_REF(tarAttrs);
		DELETE_REF(usrObj);
		DELETE_REF(usrAttrs);
	}
DETACH_JNI:
	JNI_detach();

	return retCode;
}
