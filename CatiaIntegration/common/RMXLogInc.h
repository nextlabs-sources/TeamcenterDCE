
#pragma once

#if !defined(LOG4CPLUS_ENABLE) 
#define LOG4CPLUS_ENABLE
#endif

#if !defined(ENV_LOGCONFIG_DIR)
#define ENV_LOGCONFIG_DIR L"CATIA_RMX_ROOT"
#endif

#include <RMXLogger.h>

#define RMXLOG_BEGIN() LOG_INITIALIZE3(); \
LOG_HEADER();

#define RMXLOG_END() LOG_FOOTER(); \
LOG_SHUTDOWN();

#define LOG_ENTER_CALL(msg) RMXLogger::CallLogGuard acLogGuard(L"", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()), L"", true)
