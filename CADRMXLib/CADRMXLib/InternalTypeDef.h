#pragma once

static const char* RMXL_LOGIN_PASSCODE = "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}";

#define SDL2RMXL_RESULT(sdlRet) RMXL_RESULT2(sdlRet.GetCode(), RMXUtils::s2ws(sdlRet.GetMsg()).c_str())
#define RMXL_PTRAS(Ptr, Type)            ((Type *)(Ptr))

#define RMXL_ERROR_RETURN(eval, result) \
	if (eval) \
	{ \
		LOG_ERROR_FMT(LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()) __ERRORFMT__, result.ToString().c_str()); \
		return (RMXResult)result; \
	}
#define __ERRORFMT__ LOG4CPLUS_TEXT(" (error: %s)")
#define RMXL_ERROR_RETURN_FMT(eval, result, fmt, ...) \
	if (eval) \
	{ \
		LOG_ERROR_FMT(fmt __ERRORFMT__, __VA_ARGS__, result.ToString().c_str()); \
		return (RMXResult)result; \
	}
