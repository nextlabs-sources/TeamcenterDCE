#include <nxl_action_handler.h>
#include <dce_mem.h>
#include <dce_log.h>
#include <tc_utils.h>
#include <tc/tc_arguments.h>
#include <utils_string_list.h>


int register_event_handler(const char *objectName, const char *propName, const char *msgName, METHOD_action_type_t eventType, const char *eventName, METHOD_function_t handler, const char *caller)
{
	METHOD_id_t method;
	TC_argument_list_t *args;
	int ret = -1;
	if (propName != NULL)
	{
		DBGLOG("%s():Registering Property handler for %s event of %s/%s/%s...", caller,
			eventName, objectName, propName, msgName);
		ITK_EVAL(METHOD_find_prop_method(objectName, propName, msgName, &method));
	}
	else
	{
		DBGLOG("%s():Registering Object handler for %s event of %s/%s...", caller,
			eventName, objectName, msgName);
		ITK_EVAL(METHOD_find_method(objectName, msgName, &method));
	}
	TC_ALLOC(args, TC_argument_list_t);
	TC_ALLOCN(args->arguments, TC_argument_t, USER_ARGS_LENGTH);
	//object name
	args->arguments[USER_ARGS_INDEX_OBJ_NAME].type = POM_string;
	args->arguments[USER_ARGS_INDEX_OBJ_NAME].array_size = 1;
	args->arguments[USER_ARGS_INDEX_OBJ_NAME].val_union.str_value = TC_DUP(objectName);
	//property name
	args->arguments[USER_ARGS_INDEX_PROP_NAME].type = POM_string;
	args->arguments[USER_ARGS_INDEX_PROP_NAME].array_size = 1;
	args->arguments[USER_ARGS_INDEX_PROP_NAME].val_union.str_value = TC_DUP(propName);
	//message name
	args->arguments[USER_ARGS_INDEX_MSG_NAME].type = POM_string;
	args->arguments[USER_ARGS_INDEX_MSG_NAME].array_size = 1;
	args->arguments[USER_ARGS_INDEX_MSG_NAME].val_union.str_value = TC_DUP(msgName);
	//event name
	args->arguments[USER_ARGS_INDEX_EVENT_NAME].type = POM_string;
	args->arguments[USER_ARGS_INDEX_EVENT_NAME].array_size = 1;
	args->arguments[USER_ARGS_INDEX_EVENT_NAME].val_union.str_value = TC_DUP(eventName);
	args->number_of_arguments = USER_ARGS_LENGTH;
	if (method.id != NULL
		&& ITK_EVAL(METHOD_add_action(method, eventType, handler, args)))
	{
		SYSLOG("Registered '%s'.'%s' Success(%d)", objectName, propName, *((int *)(method.id)));
		ret = ITK_ok;
	}
	MOCK_FREE(args->arguments);
	MOCK_FREE(args);
	return ret;
}
static string_list_p suppressed_messages = NULL;
void suppress_message_handler(const char *msgName, logical suppress)
{
	if (suppressed_messages == NULL) {
		suppressed_messages = string_list_new();
	}
	int imsg = string_list_index_of(suppressed_messages, msgName);
	if (imsg >= 0) {
		if (!suppress)
		{
			string_list_removeAt(suppressed_messages, imsg);
			DBGLOG("Suppress Message-'%s'=FALSE", msgName);
		}
	}
	else if (suppress) {
		string_list_add(suppressed_messages, msgName);
		DBGLOG("Suppress Message-'%s'=TRUE", msgName);
	}
}
logical is_message_suppressed(METHOD_message_t *msg) {
	const char *messageName = GET_MSGNAME(msg);
	if (suppressed_messages != NULL
		&& messageName != NULL
		&& string_list_index_of(suppressed_messages, messageName) >= 0)
	{
		DBGLOG("Message Handling for '%s' is suppressed", messageName);
		return TRUE;
	}
	return FALSE;
}