#pragma once
#include <nxl_auto_protect_exports.h>
#include <tccore/method.h>

//
//Debug message handler
//
#define DBG_STR(x) DBGLOG("==>%s='%s'", #x, (x))
#define DBG_INT(x) DBGLOG("==>%s=%d", #x, (x))
#define DEBUG_MSG(m) \
{	\
	DBG_INT(*((int *)(m->method.id)));	\
	DBG_INT(m->object);	\
	DBG_INT(m->object_tag);	\
	DBG_STR(m->prop_name);	\
	if(m->user_args != NULL)	\
	{	\
		int i;	\
		for(i=0; i<m->user_args->number_of_arguments; i++)	\
		{	\
			if(m->user_args->arguments[i].type == POM_string)	\
			{	\
				DBGLOG("==>%s->user_args[%d/%d]=%s", #m, i+1, m->user_args->number_of_arguments, m->user_args->arguments[i].val_union.str_value);	\
			}	\
			else	\
			{	\
				DBGLOG("==>%s->user_args[%d/%d]=%d", #m, i+1, m->user_args->number_of_arguments, m->user_args->arguments[i].val_union.int_value);	\
			}	\
		}	\
	}	\
}

#define USER_ARGS_INDEX_OBJ_NAME 0
#define USER_ARGS_INDEX_PROP_NAME 1
#define USER_ARGS_INDEX_MSG_NAME 2
#define USER_ARGS_INDEX_EVENT_NAME 3
#define USER_ARGS_LENGTH 4

#define GET_MSGNAME(msg) ((msg->user_args != NULL && msg->user_args->number_of_arguments > USER_ARGS_INDEX_MSG_NAME && msg->user_args->arguments[USER_ARGS_INDEX_MSG_NAME].type == POM_string)	\
	? msg->user_args->arguments[USER_ARGS_INDEX_MSG_NAME].val_union.str_value	\
	: "")


#ifdef __cplusplus
extern "C" {
#endif
extern DCEAPI void suppress_message_handler(const char *msgName, logical suppress);
extern DCEAPI logical is_message_suppressed(METHOD_message_t *msg);

extern DCEAPI int register_event_handler(const char *objectName, const char *propName, const char *msgName, METHOD_action_type_t eventType, const char *eventName, METHOD_function_t handler, const char *caller);

extern DCEAPI int nxl_debug(METHOD_message_t* msg, va_list args);
#ifdef __cplusplus
}
#endif

#define REGISTER_EVENT_HANDLER(objectName, propName, msgName, eventType, handler) register_event_handler(objectName, propName, msgName, eventType, #eventType, handler, __FUNCTION__)