
#pragma once

#if !defined(LOG4CPLUS_ENABLE) 
#define LOG4CPLUS_ENABLE
#endif

#if !defined(ENV_LOGCONFIG_DIR)
#define ENV_LOGCONFIG_DIR L"SLDWORKS_RMX_ROOT"
#endif

#include <RMXLogger.h>

#define RMXLOG_BEGIN() LOG_INITIALIZE3(); \
LOG_HEADER();

#define RMXLOG_END() LOG_FOOTER(); \
LOG_SHUTDOWN();
