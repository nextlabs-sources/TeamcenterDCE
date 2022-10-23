#include <jni.h>
#include <utils_system.h>
#include "utils_log.h"
#include "utils_jni_internal.h"
#include "utils_mem.h"
#include "utils_string.h"
#include "utils_nxl.h"

#if defined(WNT)
typedef DWORD thread_id;
#define CurrentThread() GetCurrentThreadId()
#define THREAD_EQUAL(a, b) (a==b)
#define CLASSPATH_FILE_SPLITTER ";"
#elif defined(__linux__)
#include <pthread.h>
typedef pthread_t thread_id;
#define CurrentThread() pthread_self()
#define THREAD_EQUAL(a, b) pthread_equal(a, b)
#define CLASSPATH_FILE_SPLITTER ":"
#endif

#define JVM_DLL "jvm.dll"
#define OPTION_CLASSPATH "-Djava.class.path="
JavaVM *g_jvm = NULL;
static thread_id g_threadId = 0;

BOOL load_jvm()
{
#ifdef WNT
	HMODULE module;
	char buffer[MAX_PATH] = "";
	if((module = GetModuleHandle(JVM_DLL)) == NULL)
	{
		//path to search jvm.dll
		char *envNames[] = {"JRE_HOME", "JAVA_HOME", "JRE64_HOME", "JAVA64_HOME"};
		char *relPaths[] = {"bin\\server\\", "jre\\bin\\server\\", "bin\\server\\", "jre\\bin\\server\\"};
		int n = sizeof(envNames)/sizeof(char*);
		int i;
		DBGLOG("PATH=%s", getenv("PATH"));
		for(i=0; i<n; i++)
		{
			char *path = getenv(envNames[i]);
			if(path != NULL)
			{
				sprintf(buffer, "%s\\%s", path, relPaths[i]);
				if(SetDllDirectory(buffer))
				{
					DBGLOG("SetDllDirectory('%s'):Success(%s=%s)", buffer, envNames[i], path);
					break;
				}
				DBGLOG("SetDllDirectory('%s'):Failed(%s=%s):Error=%d", buffer, envNames[i], path, GetLastError());
			}
			else
			{
				DBGLOG("genenv('%s') failed", envNames[i]);
			}
		}
		//load jvm.dll from default searching path
		module = LoadLibrary(JVM_DLL);
		SetDllDirectory(NULL);
	}
	if(module != NULL)
	{
		if(GetModuleFileName(module, buffer, MAX_PATH))
		{
			DBGLOG(JVM_DLL" is loaded(%p):path='%s')", module, buffer);
		}
		else
		{
			DBGLOG(JVM_DLL" is loaded(%p), but Failed to GetModuleFileName!(ErrorCode:%#X)", module, GetLastError());
		}
		return TRUE;
	}
	return FALSE;
#else
	return TRUE;
#endif
}
//Implements
int JNI_init(JNIEnv **env, const char *classPath)
{
	if (string_isNullOrSpace(classPath))
		return JNI_ERR;
	string_list_p jars = string_list_new();
	string_list_add(jars, classPath);
	int ret = JNI_init2(env, jars);
	string_list_free(jars);
	return ret;
}
int JNI_init2(JNIEnv **env, string_list_ro jars)
{
	int ret = JNI_OK;
	thread_id curThread = GetCurrentThreadId();
	if (jars == NULL || jars->count <= 0)
	{
		return JNI_ERR;
	}
	
	if(g_jvm == NULL)
	{
		JavaVMInitArgs args;
		JavaVMOption optionClassPath;
		const char *joined = strings_join(jars->items, jars->count, CLASSPATH_FILE_SPLITTER);
		char clsbuff[10240 + 20];
		SYSLOG("Initializing JVM with class file =%s in thread-%lu", joined, curThread);
		//try to load jvm.dll
		if(!load_jvm())
		{
			ERRLOG("Failed to load %s", JVM_DLL);
			return JNI_ERR;
		}
		//option for class path
		sprintf(clsbuff, "%s%s", OPTION_CLASSPATH, joined);
		optionClassPath.optionString = clsbuff;//"-Djava.class.path=c:\\nlJavaSDK2\\nlJavaSDK2.jar";
		//initialize args
		args.version = JNI_VERSION;
		args.nOptions = 1;
		args.options = &optionClassPath;
		args.ignoreUnrecognized = 0;
		//create JVM
		DBGLOG("JNI_CreateJavaVM returns %d", JNI_CreateJavaVM(&g_jvm, (void**)env, &args));
		if(g_jvm == NULL)
			return JNI_ERR;
		g_threadId = curThread;
		SYSLOG("==>JVM is created in thread-%lu", curThread);
	}
	else
	{
		SYSLOG("JVM hash been created in thread-%lu", g_threadId);
		DBGLOG("Creating JNIEnv from JVM(CreatedThreadId=%lu) for current thread-%lu", g_threadId, curThread);
		switch((ret = (*g_jvm)->GetEnv(g_jvm, (void **)env, JNI_VERSION)))
		{
			case JNI_EDETACHED:
				//attached current thread to JVM
				DBGLOG("AttachCurrentThread() returns %d", (*g_jvm)->AttachCurrentThread(g_jvm, (void**)env, NULL));
				if(env == NULL)
				{
					ERRLOG("Failed to Get JNIEnv from JVM(%lu) for Thread-%lu ", g_threadId, curThread);
					return ret;
				}
				break;
			case JNI_EVERSION:
				ERRLOG("Java Version %x is not supported ", JNI_VERSION);
				return ret;
			case JNI_OK:
			default:
				DBGLOG("Current Thread(%d) HAS BEEN attached to JVM thread(%lu)", curThread, g_threadId);
				break;
		}
	}

	SYSLOG("JNIEnv=%d", *env);
	return JNI_OK;
}

void JNI_detach()
{
	if(g_jvm!=NULL)
	{
		thread_id curThread = GetCurrentThreadId();
		DBGLOG("():Detaching current thread(%lu) from JVM(Created in thread-%lu)", curThread, g_threadId);
		if(!THREAD_EQUAL(curThread, g_threadId))
		{
			DBGLOG("==>DetachCurrentThread returns %d", (*g_jvm)->DetachCurrentThread(g_jvm));
		}
	}
}

int JNI_make_global(jobject *global, JNIEnv *env, jobject obj)
{
	if(obj != NULL)
	{
		if(JNI_EVAL(*global = (*env)->NewGlobalRef(env, obj)))
		{
			(*env)->DeleteLocalRef(env, obj);
			return JNI_OK;
		}
	}
	return JNI_ERR;
}

int JNI_getClass_global(jclass *cls, JNIEnv *env, const char *clsName)
{
	DBGLOG("Searching class-'%s')", clsName);
	if((*cls) != NULL)
	{
		DBGLOG("==>%s already loaded(%d)", clsName);
		return JNI_OK;
	}
	{
		jclass localCls;
		if(JNI_EVAL(localCls = (*env)->FindClass(env, clsName)))
		{
			//globalCls = localCls;
			if(JNI_EVAL(*cls = (*env)->NewGlobalRef(env, localCls)))
			{
				(*env)->DeleteLocalRef(env, localCls);
				DTLLOG("==>DONE");
				return JNI_OK;
			}
		}
		return JNI_ERR;
	}
}

int JNI_getMethod_global(jmethodID *methodID, JNIEnv *env, const jclass cls, const char *methodName, const char *sig)
{
	DBGLOG("Searching Method-(%d, \"%s\",\"%s\")", cls, methodName, sig);
	if(cls == NULL)
		return JNI_ERR;
	if((*methodID) != NULL)
	{
		DBGLOG("method-%s already loaded(%d)", methodName, methodID);
		return JNI_OK;
	}
	{
		jmethodID localMethod;
		if(JNI_EVAL(localMethod = (*env)->GetMethodID(env, cls, methodName, sig)))
		{
			//globalMethod = localMethod;
			if(JNI_EVAL(*methodID = (jmethodID)((*env)->NewGlobalRef(env, (jobject)localMethod))))
			{
				(*env)->DeleteLocalRef(env, (jobject)localMethod);
				DTLLOG("==>DONE");
				return JNI_OK;
			}
		}
		return JNI_ERR;
	}
}

int JNI_checkException(JNIEnv *env, const char *call, const void *ret, const char *func, const char *file, int line, BOOL checkNull)
{
	jthrowable exception = (*env)->ExceptionOccurred(env);
	(*env)->ExceptionClear(env);
	if(exception != NULL)
	{
		extern jclass excepCls;
		extern jmethodID excepToStrMethod;
		extern jmethodID excepPrintSTMethod;
		nxl_log(TO_LOGFILE | TO_DBGVIEW, PROJECT_NAME "!ERROR!%s():Exception happened during executing %s at FILE-%s LINE-%d", func, call, file, line);
		if (excepCls != NULL&&excepGetMsgMethod != NULL)
		{
			const char *msg;
			jstring jmsg;
			////getMessage
			if (JNI_EVAL(jmsg = (*env)->CallObjectMethod(env, exception, excepGetMsgMethod)))
			{
				if (JNI_EVAL(msg = (*env)->GetStringUTFChars(env, jmsg, NULL)))
				{
					nxl_log(TO_LOGFILE | TO_DBGVIEW | TO_CONSOLE, PROJECT_NAME "!ERROR!%s():Message:%s", func, msg);
					//clean
					(*env)->ReleaseStringUTFChars(env, jmsg, msg);

				}
				DELETE_REF(jmsg);
			}

			//ToString
			if (JNI_EVAL(jmsg = (*env)->CallObjectMethod(env, exception, excepToStrMethod)))
			{
				if (JNI_EVAL(msg = (*env)->GetStringUTFChars(env, jmsg, NULL)))
				{
					nxl_log(TO_LOGFILE | TO_DBGVIEW | TO_CONSOLE, PROJECT_NAME "!ERROR!%s():Exception:%s", func, msg);
					//clean
					(*env)->ReleaseStringUTFChars(env, jmsg, msg);
				}
				(*env)->DeleteLocalRef(env, jmsg);
			}

			//PrintStackTrace
			(*env)->CallObjectMethod(env, exception, excepPrintSTMethod);
			//
			(*env)->DeleteLocalRef(env, exception);
		}
		return JNI_ERR;
	}
	if(checkNull && ret==NULL)
	{
		nxl_log(TO_LOGFILE|TO_DBGVIEW|TO_CONSOLE, PROJECT_NAME "!ERROR!%s():%s returns NULL", func, call);
		return JNI_ERR;
	}
	//nxl_log(TO_LOGFILE | TO_DBGVIEW | TO_CONSOLE, PROJECT_NAME "!%s:Success", call);
	return JNI_OK;
}

//Java Exception
jclass excepCls = NULL;
jmethodID excepGetMsgMethod = NULL;
jmethodID excepToStrMethod = NULL;
jmethodID excepPrintSTMethod = NULL;

const char *JNI_get_class_name(JNIEnv *env, jobject obj)
{
	const char *str;
	static char ret[255];
	jstring jstr;
	jclass cls = (*env)->GetObjectClass(env, obj);
	// First get the class object
	jmethodID mid = (*env)->GetMethodID(env, cls, "getClass", "()Ljava/lang/Class;");
	jobject clsObj = (*env)->CallObjectMethod(env, obj, mid);

	// Now get the class object's class descriptor
	cls = (*env)->GetObjectClass(env, clsObj);

	// Find the getName() method on the class object
	mid = (*env)->GetMethodID(env, cls, "getName", "()Ljava/lang/String;");

	// Call the getName() to get a jstring object back
	jstr = (jstring)(*env)->CallObjectMethod(env, clsObj, mid);

	// Now get the c string from the java jstring object
	str = (*env)->GetStringUTFChars(env, jstr, NULL);
	strcpy_s(ret, sizeof(ret), str);
	(*env)->ReleaseStringUTFChars(env, jstr, str);
	DELETE_REF(jstr);
	DELETE_REF(cls);
	DELETE_REF(clsObj);

	return ret;
}