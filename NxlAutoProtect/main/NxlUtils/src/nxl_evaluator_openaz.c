#include "nxl_evaluator_remote.h"
#include "utils_string.h"
#include "utils_log.h"
#include "utils_jni_internal.h"
#include <utils_system.h>
#include <utils_nxl.h>
#include <utils_mem.h>

jvalue call_instance_method(JNIEnv *env, jobject obj, const char *methodName, const char *methodSignature, jmethodID *refMethodId, ...);

static BOOL s_initialized = FALSE;
jclass s_clsString = NULL;
jstring s_jemptystr = NULL;
const char *s_constant_key_resource_type = NULL;

static int openaz_initialize(JNIEnv *env)
{
	int ret = JNI_OK;
	if (s_initialized == TRUE)
	{
		return ret;
	}
	DBGLOG("Initializing global variables(classId/methodId)");
	//Java Exception
	//if(excepCls == NULL)
	{
		jclass clsString = (*env)->FindClass(env, "java/lang/String");
		jstring jemptystr = (*env)->NewStringUTF(env, "");
		s_clsString = (*env)->NewGlobalRef(env, clsString);
		s_jemptystr = (*env)->NewGlobalRef(env, jemptystr);
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
	{
		jclass clsconstants;
		if (JNI_EVAL(clsconstants = (*env)->FindClass(env, "com/nextlabs/openaz/utils/Constants")))
		{
			jobject jidentifier;
			jstring jreskey;
			jfieldID jfield;
			const char *reskey;
			JNI_CALL(jfield = (*env)->GetStaticFieldID(env, clsconstants, "ID_RESOURCE_RESOURCE_TYPE", "Lorg/apache/openaz/xacml/api/Identifier;"));
			JNI_CALL(jidentifier = (*env)->GetStaticObjectField(env, clsconstants, jfield));
			jreskey = call_instance_method(env, jidentifier, "stringValue", "()Ljava/lang/String;", NULL).l;
			reskey = (*env)->GetStringUTFChars(env, jreskey, NULL);
			s_constant_key_resource_type = string_dup(reskey);
			(*env)->ReleaseStringUTFChars(env, jreskey, reskey);
			DELETE_REF(jidentifier);
			DELETE_REF(jreskey);
			DELETE_REF(clsconstants);
		}
		else
		{
			ret++;
		}
	}
	DBGLOG("Finished with %d", ret);
	if (ret == JNI_OK)
		s_initialized = TRUE;
	return ret;
}
string_list_ro openaz_pep_jars()
{
	static string_list_p jars = NULL;
	if (jars == NULL
		|| jars->count <= 0) {
		const char* dependentModules[] = { "openaz", "jaxb-ri" };
		const char *searchPaths[] = { get_utils_dir(), getenv(ENV_DCE_ROOT) };
		const char *relPaths[] = { ".", "shared_libs" };
		int npath = sizeof(searchPaths) / sizeof(const char *);
		int ipath, imodule;
		int nmodules = sizeof(dependentModules) / sizeof(const char*);
		char libdir[MAX_PATH];
		if (jars == NULL) {
			jars = string_list_new();
		}
		for (imodule = 0; imodule < nmodules; imodule++)
		{
			DBGLOG("Loading module-%s...", dependentModules[imodule]);
			for (ipath = 0; ipath < npath; ipath++)
			{
				sprintf_s(libdir, MAX_PATH, "%s" PATH_DELIMITER "%s" PATH_DELIMITER "%s", searchPaths[ipath], relPaths[ipath], dependentModules[imodule]);
				if (file_exist(libdir))
				{
					string_list_p files = NULL;
					int j,n;
					char fullName[MAX_PATH];
					DBGLOG("Searching '%s'....", libdir);
					files = dir_list_files(libdir);
					DBGLOG("Found %d files", files->count);
					for (j = 0, n = 0; j < files->count; j++)
					{
						if (string_ends_with(files->items[j], ".jar", FALSE)) {
							sprintf_s(fullName, MAX_PATH, "%s" PATH_DELIMITER "%s", libdir, files->items[j]);
							string_list_add(jars, fullName);
							n++;
						}
					}
					DBGLOG("Found %d jars", n);
					string_list_free(files);
					if (n > 0)
					{
						break;
					}
				}
				else
				{
					DBGLOG("PathNotFound:'%s'", libdir);
				}
			}
		}
	}

	return jars;
}
#define DPC_ROOT_PROPERTY  "nextlabs.dpc.root"
#define ENGINE_NAME  "nextlabs.pdp.engine.name"
#define URN_NEXTLABS  "urn:nextlabs:names:evalsvc:1.0"
#define PDP_REST_EXECUTOR_NAME  "nextlabs.cloudaz.executor_name"
#define PDP_REST_HOST  "nextlabs.cloudaz.host"
#define PDP_REST_PORT  "nextlabs.cloudaz.port"
#define PDP_REST_RESOURCE_PATH  "nextlabs.cloudaz.resource_path"
#define PDP_REST_HTTPS  "nextlabs.cloudaz.https"
#define PDP_REST_IGNORE_HTTPS_CERTIFICATE  "nextlabs.cloudaz.ignore_https_certificate"
#define PDP_REST_AUTH_TYPE  "nextlabs.cloudaz.auth_type"
#define PDP_REST_CAS_AUTH_USERNAME  "nextlabs.cloudaz.cas_auth.username"
#define PDP_REST_CAS_AUTH_PASSWORD  "nextlabs.cloudaz.cas_auth.password"
#define PDP_REST_PAYLOAD_CONTENT_TYPE  "nextlabs.cloudaz.payload_content_type"
#define PDP_REST_OAUTH2_SERVER  "nextlabs.cloudaz.oauth2.server"
#define PDP_REST_OAUTH2_PORT  "nextlabs.cloudaz.oauth2.port"
#define PDP_REST_OAUTH2_HTTPS  "nextlabs.cloudaz.oauth2.https"
#define PDP_REST_OAUTH2_TOKEN_ENDPOINT_PATH  "nextlabs.cloudaz.oauth2.token_endpoint_path"
#define PDP_REST_OAUTH2_TOKEN_GRANT_TYPE  "nextlabs.cloudaz.oauth2.grant_type"
#define PDP_REST_OAUTH2_TOKEN_EXPIRES_IN  "nextlabs.cloudaz.oauth2.token_expires_in"
#define PDP_REST_OAUTH2_USERNAME  "nextlabs.cloudaz.oauth2.username"
#define PDP_REST_OAUTH2_PASSWORD  "nextlabs.cloudaz.oauth2.password"
#define PDP_REST_OAUTH2_CLIENT_ID  "nextlabs.cloudaz.oauth2.client_id"
#define PDP_REST_OAUTH2_CLIENT_SECRET  "nextlabs.cloudaz.oauth2.client_secret"
#define CAS_TICKET_PATH  "/cas/v1/tickets"
#define PDP_REST_FORM_PARAM_SERVICE  "EVAL"
#define PDP_REST_FORM_PARAM_Version  "1.0"

#define XACMLProperties_PROP_PDPENGINEFACTORY "xacml.pdpEngineFactory"


#define ERROR_CLASS_NOT_FOUND(clsName) ERRLOG("Failed to find class-'%s'", clsName)

#define CIPHER_KEY "DCE1234!NeXTlAbS"
//#define CIPHER_INITVECTOR "RandomInitVector"
#define NUM_OF_IV_BYTES 16
#define CIPHER_ALGO "AES"
#define CIPHER_CHAR_SET "UTF-8"
#define CIPHER_TRANSFORMATION "AES/CBC/PKCS5PADDING"

#define PRINT_BYTES16(bytesArray, ifirst) {	\
	jboolean isCopy = JNI_FALSE;	\
	jbyte* bytes = (*env)->GetByteArrayElements(env, bytesArray, &isCopy);	\
	DBGLOG(#bytesArray "[%d]%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:", ifirst	\
		, bytes[ifirst+0]	\
		, bytes[ifirst+1]	\
		, bytes[ifirst+2]	\
		, bytes[ifirst+3]	\
		, bytes[ifirst+4]	\
		, bytes[ifirst+5]	\
		, bytes[ifirst+6]	\
		, bytes[ifirst+7]	\
		, bytes[ifirst+8]	\
		, bytes[ifirst+9]	\
		, bytes[ifirst+10]	\
		, bytes[ifirst+11]	\
		, bytes[ifirst+12]	\
		, bytes[ifirst+13]	\
		, bytes[ifirst+14]	\
		, bytes[ifirst+15]);\
	(*env)->ReleaseByteArrayElements(env, bytesArray, bytes, JNI_ABORT); }

const char *cipher_secret(JNIEnv *env, const char *input, BOOL encryptOrDecrypt)
{
	static char *retString = NULL;
	jstring jRetString = NULL;
	jstring jinput = (*env)->NewStringUTF(env, input);

	jstring jcharset = (*env)->NewStringUTF(env, CIPHER_CHAR_SET);

	jobject jParameterSpec = NULL;
	jobject jKeySpec = NULL;
	jarray jbytesIV = NULL;
	jclass clsConverter = (*env)->FindClass(env, "jakarta/xml/bind/DatatypeConverter");
	jmethodID mConverterFromBase64 = (*env)->GetStaticMethodID(env, clsConverter, "parseBase64Binary", "(Ljava/lang/String;)[B");
	DBGLOG("%s...", encryptOrDecrypt ? "Encrypting" : "Decrypting");
	if(encryptOrDecrypt)
	{
		jclass clsSecureRandom;
		if (JNI_EVAL(clsSecureRandom = (*env)->FindClass(env, "java/security/SecureRandom")))
		{
			jmethodID ctorSecureRandom = (*env)->GetMethodID(env, clsSecureRandom, "<init>", "()V");//TODO:
			jobject jSecureRandom = (*env)->NewObject(env, clsSecureRandom, ctorSecureRandom);
			jbytesIV = (*env)->NewByteArray(env, NUM_OF_IV_BYTES);
			call_instance_method(env, jSecureRandom, "nextBytes", "([B)V", NULL, jbytesIV);
			DELETE_REF(jSecureRandom);
			DELETE_REF(clsSecureRandom);
		}
	}
	else {
		jarray encodedBytes = (*env)->CallStaticObjectMethod(env, clsConverter, mConverterFromBase64, jinput);
		jbytesIV = (*env)->NewByteArray(env, NUM_OF_IV_BYTES);
		jbytearray_copy(env, jbytesIV, 0, encodedBytes, 0, NUM_OF_IV_BYTES);
		DELETE_REF(encodedBytes);
	}
	if(jbytesIV != NULL)
	{
		jclass clsParameterSpec;
		if (JNI_EVAL(clsParameterSpec = (*env)->FindClass(env, "javax/crypto/spec/IvParameterSpec")))
		{
			jmethodID mParameterSpecCtor = (*env)->GetMethodID(env, clsParameterSpec, "<init>", "([B)V");
			jParameterSpec = (*env)->NewObject(env, clsParameterSpec, mParameterSpecCtor, jbytesIV);
			DELETE_REF(clsParameterSpec);
		}
	}
	{
		jclass clsKeySpec;
		if (JNI_EVAL(clsKeySpec = (*env)->FindClass(env, "javax/crypto/spec/SecretKeySpec")))
		{
			jmethodID mKeySpecCtor = (*env)->GetMethodID(env, clsKeySpec, "<init>", "([BLjava/lang/String;)V");
			jstring jkey = (*env)->NewStringUTF(env, CIPHER_KEY);
			jobject jkeyBytes = call_instance_method(env, jkey, "getBytes", "()[B", NULL, jcharset).l;
			jstring jalgo = (*env)->NewStringUTF(env, CIPHER_ALGO);
			jKeySpec = (*env)->NewObject(env, clsKeySpec, mKeySpecCtor, jkeyBytes, jalgo);
			DELETE_REF(jalgo);
			DELETE_REF(jkeyBytes);
			DELETE_REF(jkey);
			DELETE_REF(clsKeySpec);
		}
	}
	if (jParameterSpec
		&& jKeySpec)
	{
		jclass clsCipher = (*env)->FindClass(env, "javax/crypto/Cipher");
		jmethodID mCipherGetInst = (*env)->GetStaticMethodID(env, clsCipher, "getInstance", "(Ljava/lang/String;)Ljavax/crypto/Cipher;");
		jstring jtransformation = (*env)->NewStringUTF(env, CIPHER_TRANSFORMATION);
		jobject jCipher = (*env)->CallStaticObjectMethod(env, clsCipher, mCipherGetInst, jtransformation);
		jmethodID mCipherInit = (*env)->GetMethodID(env, clsCipher, "init", "(ILjava/security/Key;Ljava/security/spec/AlgorithmParameterSpec;)V");
		jfieldID fMode = (*env)->GetStaticFieldID(env, clsCipher, encryptOrDecrypt ? "ENCRYPT_MODE" : "DECRYPT_MODE", "I");
		jint mode = (*env)->GetStaticIntField(env, clsCipher, fMode);
		jvalue dummy = call_instance_method(env, jCipher, "", "", &mCipherInit, mode, jKeySpec, jParameterSpec);
		jmethodID mCipherDoFinal = (*env)->GetMethodID(env, clsCipher, "doFinal", "([B)[B");
		if (encryptOrDecrypt)
		{
			jobject jinputBytes = call_instance_method(env, jinput, "getBytes", "()[B", NULL).l;

			jobject jencryptedBytes = call_instance_method(env, jCipher, "", "", &mCipherDoFinal, jinputBytes).l;

			jsize lenEncrypted = (*env)->GetArrayLength(env, jencryptedBytes);
			jobject jEncodedBytes = (*env)->NewByteArray(env, NUM_OF_IV_BYTES + lenEncrypted);

			jbytearray_copy(env, jEncodedBytes, 0, jbytesIV, 0, NUM_OF_IV_BYTES);
			jbytearray_copy(env, jEncodedBytes, NUM_OF_IV_BYTES, jencryptedBytes, 0, lenEncrypted);

			jmethodID mConverterToBase64 = (*env)->GetStaticMethodID(env, clsConverter, "printBase64Binary", "([B)Ljava/lang/String;");
			jRetString = (*env)->CallStaticObjectMethod(env, clsConverter, mConverterToBase64, jEncodedBytes);
			DELETE_REF(jEncodedBytes);
			DELETE_REF(jinputBytes);
			DELETE_REF(jencryptedBytes);
		}
		else
		{
			jarray encodedBytes = (*env)->CallStaticObjectMethod(env, clsConverter, mConverterFromBase64, jinput);
			jsize nEncryptedBytes = (*env)->GetArrayLength(env, encodedBytes) - NUM_OF_IV_BYTES;
			jarray jencryptedBytes = (*env)->NewByteArray(env, nEncryptedBytes);
			if(jbytearray_copy(env, jencryptedBytes, 0, encodedBytes, NUM_OF_IV_BYTES, nEncryptedBytes))
			{

				jobject joriginalBytes = call_instance_method(env, jCipher, "", "", &mCipherDoFinal, jencryptedBytes).l;

				jmethodID mStringCtor = (*env)->GetMethodID(env, s_clsString, "<init>", "([B)V");
				jRetString = (*env)->NewObject(env, s_clsString, mStringCtor, joriginalBytes);

				DELETE_REF(joriginalBytes);
			}
			DELETE_REF(jencryptedBytes);
			DELETE_REF(encodedBytes);
		}
		DELETE_REF(clsCipher);
		DELETE_REF(jtransformation);
		DELETE_REF(jCipher);
	}
	DELETE_REF(clsConverter);
	DELETE_REF(jbytesIV);
	DELETE_REF(jParameterSpec);
	DELETE_REF(jKeySpec);
	DELETE_REF(jcharset);
	DELETE_REF(jinput);
	//returning
	if (retString != NULL) {
		string_free(retString);
		retString = NULL;
	}
	if (jRetString)
	{
		const char *utf = (*env)->GetStringUTFChars(env, jRetString, NULL);
		retString = string_dup(utf);
		(*env)->ReleaseStringUTFChars(env, jRetString, utf);
		DELETE_REF(jRetString);
		DBGLOG("Success");
		//DBGLOG("Returning('%s')...", retString);
		return retString;
	}
	DBGLOG("Failed");
	return input;
}
#define PROPERTIES_CLASS "java/util/Properties"
jobject create_properties(JNIEnv *env, pep_agent_properties_p agentProperties)
{
	jobject jproperties = NULL;
	const char* clientSecret = agentProperties->oauth2ClientSecretEncrypted
		? clientSecret = cipher_secret(env, agentProperties->oauth2ClientSecret, FALSE)
		: agentProperties->oauth2ClientSecret;
	if (clientSecret != NULL)
	{
		const char* properties[] = {
			ENGINE_NAME, "com.nextlabs.openaz.pdp.RestPDPEngine",
			PDP_REST_PAYLOAD_CONTENT_TYPE, "application/json",
			PDP_REST_HOST, agentProperties->pdpHost,
			PDP_REST_PORT, agentProperties->pdpPort,
			PDP_REST_HTTPS, agentProperties->pdpHttps,
			PDP_REST_IGNORE_HTTPS_CERTIFICATE, agentProperties->pdpIgnoreHttps,
			PDP_REST_AUTH_TYPE, "OAUTH2",
			PDP_REST_OAUTH2_TOKEN_GRANT_TYPE, "client_credentials",
			PDP_REST_OAUTH2_SERVER, agentProperties->oauth2Host,
			PDP_REST_OAUTH2_PORT, agentProperties->oauth2Port,
			PDP_REST_OAUTH2_HTTPS, agentProperties->oauth2Https,
			PDP_REST_OAUTH2_CLIENT_ID, agentProperties->oauth2ClientId,
			PDP_REST_OAUTH2_CLIENT_SECRET,	clientSecret,
			PDP_REST_OAUTH2_TOKEN_ENDPOINT_PATH, "/cas/token",
			XACMLProperties_PROP_PDPENGINEFACTORY, "com.nextlabs.openaz.pdp.PDPEngineFactoryImpl",
			"pep.mapper.classes", "com.nextlabs.openaz.pepapi.RecipientMapper,com.nextlabs.openaz.pepapi.DiscretionaryPoliciesMapper,com.nextlabs.openaz.pepapi.HostMapper,com.nextlabs.openaz.pepapi.ApplicationMapper"
		};
		int npropeties = sizeof(properties) / sizeof(char*);
		//build properties
		jclass clsProperties;
		if (JNI_EVAL(clsProperties = (*env)->FindClass(env, PROPERTIES_CLASS)))
		{
			int i;
			jmethodID mPropertiesCtor, mSetProperty;
			DBGLOG("Initializing " PROPERTIES_CLASS " object...");
			JNI_CALL(mPropertiesCtor = (*env)->GetMethodID(env, clsProperties, "<init>", "()V"));
			JNI_CALL(jproperties = (*env)->NewObject(env, clsProperties, mPropertiesCtor));
			JNI_CALL(mSetProperty = (*env)->GetMethodID(env, clsProperties, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;"));
			for (i = 0; i <= npropeties - 2; i += 2)
			{
				jstring jkeystr, jvalstr;
				if (stricmp(properties[i], PDP_REST_OAUTH2_CLIENT_SECRET) == 0)
				{
					DBGLOG("[%d/%d]'%s'", i, npropeties, properties[i]);

				}
				else
				{
					DBGLOG("[%d/%d]'%s'='%s'", i, npropeties, properties[i], properties[i + 1]);
				}
				jkeystr = (*env)->NewStringUTF(env, properties[i]);
				jvalstr = (*env)->NewStringUTF(env, properties[i + 1]);
				JNI_CALL2((*env)->CallObjectMethod(env, jproperties, mSetProperty, jkeystr, jvalstr));
				DELETE_REF(jkeystr);
				DELETE_REF(jvalstr);
			}
			DELETE_REF(clsProperties);
		}
		else
		{
			ERROR_CLASS_NOT_FOUND(PROPERTIES_CLASS);
		}
	}

	return jproperties;
}
jmethodID find_object_method(JNIEnv *env, jobject obj, const char *methodName, const char *methodSignature) {
	jclass cls = (*env)->GetObjectClass(env, obj);
	jmethodID retId = NULL;
	if (cls)
	{
		JNI_CALL(retId = (*env)->GetMethodID(env, cls, methodName, methodSignature));
		if (!retId) {
			ERRLOG("no matched method(name='%s' sig='%s') in object class-'%s'", methodName, methodSignature, JNI_get_class_name(env, obj));
		}
	}
	return retId;
}
jvalue call_instance_method(JNIEnv *env, jobject obj, const char *methodName, const char *methodSignature, jmethodID *pMethod, ...)
{
	jvalue ret;
	if (obj)
	{
		jmethodID mid;

		if (pMethod && *pMethod)
		{
			mid = *pMethod;
		}
		else
		{
			mid = find_object_method(env, obj, methodName, methodSignature);
			if (pMethod)
			{
				*pMethod = mid;
			}
		}
		if (mid)
		{
			int lenSig = strlen(methodSignature);
			BOOL retIsArrayOfValue = methodSignature[lenSig - 2] == '[';
			va_list args;
			va_start(args, pMethod);
			if (retIsArrayOfValue)
			{
				ret.l = (*env)->CallObjectMethodV(env, obj, mid, args);
				DBGLOG("obj-%p->%s(%s) returns object(%p)", obj, methodName, methodSignature, ret.l);
			}
			else
			{
				switch (toupper(methodSignature[lenSig - 1]))
				{
				case 'Z':
					ret.z = (*env)->CallBooleanMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns boolean(%d)", obj, methodName, methodSignature, ret.z);
					break;
				case 'B':
					ret.b = (*env)->CallByteMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns byte(%x)", obj, methodName, methodSignature, ret.b);
					break;
				case 'C':
					ret.c = (*env)->CallCharMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns char(%c)", obj, methodName, methodSignature, ret.c);
					break;
				case 's':
					ret.s = (*env)->CallShortMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns short(%d)", obj, methodName, methodSignature, ret.s);
					break;
				case 'I':
					ret.i = (*env)->CallIntMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns int(%d)", obj, methodName, methodSignature, ret.i);
					break;
				case 'J':
					ret.j = (*env)->CallLongMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns long(%ld)", obj, methodName, methodSignature, ret.j);
					break;
				case 'F':
					ret.f = (*env)->CallFloatMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns float(%f)", obj, methodName, methodSignature, ret.f);
					break;
				case 'D':
					ret.d = (*env)->CallDoubleMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns double(%lf)", obj, methodName, methodSignature, ret.d);
					break;
				case 'V':
					(*env)->CallVoidMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns void", obj, methodName, methodSignature);
					break;
				default:
					ret.l = (*env)->CallObjectMethodV(env, obj, mid, args);
					DBGLOG("obj-%p->%s(%s) returns object(%p)", obj, methodName, methodSignature, ret.l);
					break;
				}
			}
			JNI_checkException(env, methodSignature, ret.l, methodName, __FILE__, __LINE__, FALSE);
			va_end(args);
		}
	}

	return ret;
}

jobject create_string_array(JNIEnv *env, const char **strs, int nstr)
{
	jobject jarray = (*env)->NewObjectArray(env, nstr, s_clsString, s_jemptystr);
	int i;
	for (i = 0; i < nstr; i++)
	{
		jstring str = (*env)->NewStringUTF(env, strs[i]);
		JNI_CALL_void((*env)->SetObjectArrayElement(env, jarray, i, str));
		DELETE_REF(str);
	}

	return jarray;
}
void addAttribute(JNIEnv *env, jobject obj, jmethodID *pMid, const char *key, const char *val)
{
	jmethodID mAddAttribute;
	jstring jkeystr;
	jobject jvalarray;
	if (pMid && *pMid)
	{
		mAddAttribute = *pMid;
	}
	else
	{
		mAddAttribute = find_object_method(env, obj, "addAttribute", "(Ljava/lang/String;[Ljava/lang/String;)V");
		if(pMid)
		{
			*pMid = mAddAttribute;
		}
	}
	DBGLOG("'%s'='%s'", key, val);
	jkeystr = (*env)->NewStringUTF(env, key);
	jvalarray = create_string_array(env, &val, 1);
	JNI_CALL2((*env)->CallObjectMethod(env, obj, mAddAttribute, jkeystr, jvalarray));
	DELETE_REF(jkeystr);
	DELETE_REF(jvalarray);
}

#define CLASS_PEP_AGENT_FACTORY "org/apache/openaz/pepapi/std/StdPepAgentFactory"
jobject create_pep_agent(JNIEnv* env, pep_agent_properties_p agentProperties) {
	jobject retPepAgent = NULL;
	jobject jproperties = create_properties(env, agentProperties);
	if (jproperties != NULL)
	{
		jclass clsFactory;
		DBGLOG("Creating JNI PepAgent Object...");
		if (JNI_EVAL(clsFactory = (*env)->FindClass(env, CLASS_PEP_AGENT_FACTORY)))
		{
			jobject jfactory = NULL;
			//create pepagent
			jmethodID mFacotryCtor;
			JNI_CALL(mFacotryCtor = (*env)->GetMethodID(env, clsFactory, "<init>", "(Ljava/util/Properties;)V"));
			JNI_CALL(jfactory = (*env)->NewObject(env, clsFactory, mFacotryCtor, jproperties));
			retPepAgent = call_instance_method(env, jfactory, "getPepAgent", "()Lorg/apache/openaz/pepapi/PepAgent;", NULL).l;
			//clean
			DELETE_REF(jfactory);
			DELETE_REF(clsFactory);
			DBGLOG("PepAgent=%p(className='%s')", retPepAgent, JNI_get_class_name(env, retPepAgent));
		}
		else
		{
			ERROR_CLASS_NOT_FOUND(CLASS_PEP_AGENT_FACTORY);
		}
		DELETE_REF(jproperties);
	}
	return retPepAgent;
}

#define CLASS_PEP_ACTION "org/apache/openaz/pepapi/Action"
jobject create_pep_action(JNIEnv *env, const char *action) {
	jobject jaction = NULL;
	jclass clsAction;
	if (JNI_EVAL(clsAction = (*env)->FindClass(env, CLASS_PEP_ACTION)))
	{
		jstring jactionstr = (*env)->NewStringUTF(env, action);
		jmethodID mNewInstance;
		JNI_CALL(mNewInstance = (*env)->GetStaticMethodID(env, clsAction, "newInstance", "(Ljava/lang/String;)Lorg/apache/openaz/pepapi/Action;"));
		JNI_CALL(jaction = (*env)->CallStaticObjectMethod(env, clsAction, mNewInstance, jactionstr));
		DELETE_REF(clsAction);
		DELETE_REF(jactionstr);
	}
	else
	{
		ERROR_CLASS_NOT_FOUND(CLASS_PEP_ACTION);
	}
	DBGLOG("PepAction('%s')=%p", action, jaction);

	return jaction;
}
#define CLASS_PEP_SUBJECT "org/apache/openaz/pepapi/Subject"
jobject create_pep_user(JNIEnv *env, eval_user_ro userInfo) {
	jobject juser = NULL;
	jclass clsSubject;
	if (JNI_EVAL(clsSubject = (*env)->FindClass(env, CLASS_PEP_SUBJECT)))
	{
		jmethodID mNewInstance;
		jmethodID mAddAttribute = NULL;
		jstring juid = (*env)->NewStringUTF(env, userInfo->id);
		JNI_CALL(mNewInstance = (*env)->GetStaticMethodID(env, clsSubject, "newInstance", "(Ljava/lang/String;)Lorg/apache/openaz/pepapi/Subject;"));
		JNI_CALL(juser = (*env)->CallStaticObjectMethod(env, clsSubject, mNewInstance, juid));

		//user_id
		addAttribute(env, juser, &mAddAttribute, "user_id", userInfo->id);
		//set user attributes
		if (userInfo->attributes != NULL)
		{
			int i;
			for (i = 0; i < userInfo->attributes->count; i++) {
				addAttribute(env, juser, &mAddAttribute, userInfo->attributes->keys[i], userInfo->attributes->vals[i]);
			}
		}
		DELETE_REF(clsSubject);
		DELETE_REF(juid);
	}
	else
	{
		ERROR_CLASS_NOT_FOUND(CLASS_PEP_SUBJECT);
	}
	DBGLOG("PepUser(ID='%s' name='%s')=%p", userInfo->id, userInfo->name, juser);
	return juser;
}

#define CLASS_PEP_RESOURCE "org/apache/openaz/pepapi/Resource"
jobject create_pep_resource(JNIEnv *env, eval_res_ro resInfo, eval_app_ro appInfo)
{
	jobject jresource = NULL;
	jclass clsRes ;
	DBGLOG("Initializing JNI PepResource object(resName='%s' resType='%s')...", resInfo->name, resInfo->type);
	if (JNI_EVAL(clsRes = (*env)->FindClass(env, CLASS_PEP_RESOURCE)))
	{
		jmethodID mNewInstance;
		jmethodID mAddAttribute = NULL;
		jstring jtypestr = (*env)->NewStringUTF(env, resInfo->type);
		JNI_CALL(mNewInstance = (*env)->GetStaticMethodID(env, clsRes, "newInstance", "(Ljava/lang/String;)Lorg/apache/openaz/pepapi/Resource;"));
		JNI_CALL(jresource = (*env)->CallStaticObjectMethod(env, clsRes, mNewInstance, jtypestr));

		//resource type
		addAttribute(env, jresource, &mAddAttribute, s_constant_key_resource_type, resInfo->type);
		//set url
		addAttribute(env, jresource, &mAddAttribute, "url", resInfo->name);
		addAttribute(env, jresource, &mAddAttribute, "host", host_get_name());
		//set application
		addAttribute(env, jresource, &mAddAttribute, "application", appInfo->name);
		//set resouce attributes
		if (resInfo->attributes != NULL)
		{
			int i;
			for (i = 0; i < resInfo->attributes->count; i++) {
				addAttribute(env, jresource, &mAddAttribute, resInfo->attributes->keys[i], resInfo->attributes->vals[i]);
			}
		}
		DELETE_REF(clsRes);
		DELETE_REF(jtypestr);
	}
	else
	{
		ERROR_CLASS_NOT_FOUND(CLASS_PEP_RESOURCE);
	}
	DBGLOG("jresource=%p", jresource);
	return jresource;
}
eval_result_t openaz_evaluate(pep_agent_properties_p agentProperties, const char *action,
	eval_app_ro app, eval_user_ro user, eval_res_ro src,
	obligation_list *obligations, int *nob)
{
	eval_result_t retCode = EVAL_DEFAULT;
	JNIEnv *env = NULL;
	DBGLOG("Evaluating Action-%s on Server-'%s:%s'", action, agentProperties->pdpHost, agentProperties->pdpPort);
	if (obligations != NULL)
		*obligations = NULL;
	if (nob != NULL)
		*nob = 0;
	if (string_isNullOrSpace(action))
	{
		ERRLOG("Action is not properly set!");
	}
	else if (app == NULL)
	{
		ERRLOG("Application Attributes is not properly initialized!");
	}
	else if (user == NULL)
	{
		ERRLOG("User Attributes is not properly initialized!");
	}
	else if (src == NULL)
	{
		ERRLOG("Source Resource is not Properly initialized!");
	}
	else
	{
		if (JNI_init2(&env, openaz_pep_jars()) == JNI_OK)
		{
			//make sure the global variables are loaded
			if (openaz_initialize(env) != JNI_OK)
			{
				DBGLOG("Failed to initialize global vars!");
				goto DETACH_JNI;
			}
		}
	}
	if (env != NULL)
	{
		jobject pepagent = NULL;
		jobject pepaction = NULL;
		jobject  pepuser = NULL;
		jobject  pepresource = NULL;
		
		if ((pepagent = create_pep_agent(env, agentProperties)) && pepagent != NULL
			&& (pepaction = create_pep_action(env, action)) && pepaction != NULL
			&& (pepuser = create_pep_user(env, user)) && pepuser != NULL
			&& (pepresource = create_pep_resource(env, src, app)) && pepresource != NULL)
		{
			jobject jresponse;
			jobject jargArray = NULL;
			DBGLOG("Evaluatiing...");
			{
				jclass clsobject = (*env)->FindClass(env, "java/lang/Object");
				jargArray = (*env)->NewObjectArray(env, 3, clsobject, NULL);
				JNI_CALL_void((*env)->SetObjectArrayElement(env, jargArray, 0, pepuser));
				JNI_CALL_void((*env)->SetObjectArrayElement(env, jargArray, 1, pepaction));
				JNI_CALL_void((*env)->SetObjectArrayElement(env, jargArray, 2, pepresource));
				DELETE_REF(clsobject);
			}
			if (jresponse = call_instance_method(env, pepagent, "decide", "([Ljava/lang/Object;)Lorg/apache/openaz/pepapi/PepResponse;", NULL, jargArray).l)
			{
				jboolean isallowed = call_instance_method(env, jresponse, "allowed", "()Z", NULL).z;
				jobject jresult = call_instance_method(env, jresponse, "getWrappedResult", "()Lorg/apache/openaz/xacml/api/Result;", NULL).l;
				//Get Decision from Result
				jobject jdecison = call_instance_method(env, jresult, "getDecision", "()Lorg/apache/openaz/xacml/api/Decision;", NULL).l;
				jstring jdecisonstr = call_instance_method(env, jdecison, "toString", "()Ljava/lang/String;", NULL).l;
				//process result string
				if (jdecisonstr)
				{
					const char *dstr = (*env)->GetStringUTFChars(env, jdecisonstr, NULL);
					DBGLOG("==>Response String:%s", dstr);
					CALL_DTL((*env)->ReleaseStringUTFChars(env, jdecisonstr, dstr));
				}
				//compare in case insensitive mode
				DBGLOG("allowed=%d", isallowed);
				if (isallowed)
				{
					retCode = EVAL_ALLOW;
				}// is ALLOW
				else
				{
					retCode = EVAL_DENY;
				}
				DELETE_REF(jdecison);
				DELETE_REF(jdecisonstr);
				DELETE_REF(jresult);
				//process obligations
				if (obligations != NULL && nob != NULL)
				{
					//get obligation Collection from Response
					//java.util.Map<java.lang.String, org.apache.openaz.pepapi.Obligation>
					jobject jObMap = call_instance_method(env, jresponse, "getObligations", "()Ljava/util/Map;", NULL).l;
					jmethodID mMapGetEntrySet = NULL;
					jobject jentryset = call_instance_method(env, jObMap, "entrySet", "()Ljava/util/Set;", &mMapGetEntrySet).l;
					jmethodID mSetToArray = NULL;
					jobjectArray jsetarray = call_instance_method(env, jentryset, "toArray", "()[Ljava/lang/Object;", &mSetToArray).l;
					long size = (*env)->GetArrayLength(env, jsetarray);
					int i;
					jmethodID mMapEntryGetKey = NULL;
					jmethodID mMapEntryGetValue = NULL;
					NXL_ALLOCN(*obligations, struct obligation_s, size);
					for (i = 0; i < size; i++)
					{
						jobject jmapentry = (*env)->GetObjectArrayElement(env, jsetarray, i);
						(*obligations)[i].name = NULL;
						(*obligations)[i].policy = NULL;
						(*obligations)[i].attributes = NULL;
						{
							jstring jkey = call_instance_method(env, jmapentry, "getKey", "()Ljava/lang/Object;", &mMapEntryGetKey).l;
							const char *obname = (*env)->GetStringUTFChars(env, jkey, NULL);
							(*obligations)[i].name = string_dup(obname);
							DBGLOG("Obligation[%d/%d]:Name='%s'", i + 1, size, obname);
							(*env)->ReleaseStringUTFChars(env, jkey, obname);
							DELETE_REF(jkey);
						}
						{
							jobject jobligation = call_instance_method(env, jmapentry, "getValue", "()Ljava/lang/Object;", &mMapEntryGetValue).l;
							//java.util.Map<java.lang.String, java.lang.Object[]>
							jobject jAttrMap = call_instance_method(env, jobligation, "getAttributeMap", "()Ljava/util/Map;", NULL).l;
							jobject jAttrsEntrySet = call_instance_method(env, jAttrMap, "entrySet", "()Ljava/util/Set;", &mMapGetEntrySet).l;
							jobjectArray jAttrsSetarray = call_instance_method(env, jAttrsEntrySet, "toArray", "()[Ljava/lang/Object;", &mSetToArray).l;
							long nattrs = (*env)->GetArrayLength(env, jAttrsSetarray);
							int j;
							(*obligations)[i].attributes = ht_create(nattrs);
							for (j = 0; j < nattrs; j++)
							{
								jobject jattr = (*env)->GetObjectArrayElement(env, jAttrsSetarray, j);
								jstring jattrkey = call_instance_method(env, jattr, "getKey", "()Ljava/lang/Object;", &mMapEntryGetKey).l;
								jobjectArray jattrval = call_instance_method(env, jattr, "getValue", "()Ljava/lang/Object;", &mMapEntryGetValue).l;
								const char *attrname = (*env)->GetStringUTFChars(env, jattrkey, NULL);
								long nvalues = (*env)->GetArrayLength(env, jattrval);
								if (nvalues > 0)
								{
									jstring jfirst = (*env)->GetObjectArrayElement(env, jattrval, 0);
									const char *val = (*env)->GetStringUTFChars(env, jfirst, NULL);
									DBGLOG("==>Attibute[%d/%d]:Name='%s' nValues=%d first='%s' classname='%s'", j + 1, nattrs, attrname, nvalues, val, JNI_get_class_name(env, jfirst));
									ht_set_chars((*obligations)[i].attributes, attrname, val);
									(*env)->ReleaseStringUTFChars(env, jfirst, val);
								}
								else
								{
									ht_set_chars((*obligations)[i].attributes, attrname, "");
									DBGLOG("==>Attibute[%d/%d]:Name='%s' nValues=%d", j + 1, nattrs, attrname, nvalues);
								}
								(*env)->ReleaseStringUTFChars(env, jattrkey, attrname);
								DELETE_REF(jattr);
								DELETE_REF(jattrkey);
								DELETE_REF(jattrval);
							}
							DELETE_REF(jAttrMap);
							DELETE_REF(jAttrsEntrySet);
							DELETE_REF(jAttrsSetarray);
							DELETE_REF(jobligation);
						}
						DELETE_REF(jmapentry);
					}
					*nob = size;
					//TODO:
					DELETE_REF(jsetarray);
					DELETE_REF(jObMap);
				}
			}
			DELETE_REF(jargArray);
			DELETE_REF(jresponse);
		}
	CLEAN_JNI:
		DELETE_REF(pepaction);
		DELETE_REF(pepuser);
		DELETE_REF(pepresource);
		DELETE_REF(pepagent);
	}
DETACH_JNI:
	JNI_detach();

	return retCode;

}