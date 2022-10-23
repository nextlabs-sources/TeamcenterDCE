
#pragma once
//#undef LOG4CPLUS_ENABLE
//
// Define LOG4CPLUS_ENABLE macro to enable log4 in the module
//
#if defined(_MSC_VER)
#define LOG4_DISABLE_WARNING() \
    __pragma (warning (push)) \
    __pragma (warning (disable:4127))           

#define LOG4_RESTORE_WARNING() \
    __pragma (warning (pop))

#else
#define LOG4_DISABLE_WARNING() /* empty */
#define LOG4_RESTORE_WARNING() /* empty */
#endif

#define LOG4_NOTHING() \
    LOG4_DISABLE_WARNING() \
    do { } while (0) \
    LOG4_RESTORE_WARNING()

#if !defined(LOG4CPLUS_ENABLE)
#define LOG_INITIALIZE(configFile) LOG4_NOTHING()
#define LOG_INITIALIZE2(logger, configFile) LOG4_NOTHING()
#define LOG_SHUTDOWN() LOG4_NOTHING()

#define LOG_DEBUG(logEvent) LOG4_NOTHING()
#define LOG_DEBUG_STR(logEvent) LOG4_NOTHING()
#define LOG_DEBUG_FMT(fmt, ...) LOG4_NOTHING()

#define LOG_INFO(logEvent) LOG4_NOTHING()
#define LOG_INFO_STR(logEvent) LOG4_NOTHING()
#define LOG_INFO_FMT(...) LOG4_NOTHING()

#define LOG_WARN(logEvent) LOG4_NOTHING()
#define LOG_WARN_STR(logEvent) LOG4_NOTHING()
#define LOG_WARN_FMT(...) LOG4_NOTHING()

#define LOG_ERROR(logEvent) LOG4_NOTHING()
#define LOG_ERROR_STR(logEvent) LOG4_NOTHING()
#define LOG_ERROR_FMT(...) LOG4_NOTHING()
#define CHK_ERROR_RETURN(eval, retVal, fmt, ...) \
	if (eval) \
	{ \
		return retVal; \
	}

#define CHK_ERROR(eval, fmt, ...) LOG4_NOTHING()
#else
#include <Windows.h>
#include <fileapi.h>
#include <ShlObj.h>

#include <log4cplus\logger.h>
#include <log4cplus\configurator.h>
#include <log4cplus\streams.h>
#ifndef NDEBUG
#include <log4cplus\helpers\loglog.h>
#endif
#undef LOG4CPLUS_HAVE_FUNCSIG_MACRO
#include <log4cplus\loggingmacros.h>
#include <log4cplus/tstring.h>

#if defined(_PRJNAME)
#define __LOG_CATEGORY__ LOG4CPLUS_TEXT("[") LOG4CPLUS_TEXT(_PRJNAME) LOG4CPLUS_TEXT("]")
#else
#define __LOG_CATEGORY__ LOG4CPLUS_TEXT("")
#endif

#define LOG_INITIALIZE(configFile) RMXLogger::Initialize(configFile)
#define LOG_INITIALIZE2(logger, configFile) RMXLogger::Initialize2(logger, configFile)
#define LOG_INITIALIZE3() RMXLogger::Initialize3()
#define LOG_SHUTDOWN() RMXLogger::Shutdown()

#define LOG_HEADER() \
LOG_INFO_STR(L"***********************************************************"); \
LOG_INFO_STR(L"*****       NextLabs Rights Management eXtension       ****"); \
LOG_INFO_STR(L"***********************************************************");

#define LOG_FOOTER() \
LOG_INFO_STR(L""); \
LOG_INFO_STR(L""); \

#define LOG4_CALL(logLevel, logEvent) LOG4CPLUS_##logLevel##(RMXLogger::GetLogger(), __LOG_CATEGORY__ << logEvent)
#define LOG4_CALL_STR(logLevel, logEvent) LOG4CPLUS_##logLevel##_STR(RMXLogger::GetLogger(), __LOG_CATEGORY__ logEvent)
#define LOG4_CALL_FMT(logLevel, fmt, ...) LOG4CPLUS_##logLevel##_FMT(RMXLogger::GetLogger(), __LOG_CATEGORY__ fmt, __VA_ARGS__)

#define LOG_TRACE(logEvent) LOG4_CALL(TRACE, logEvent)
#define LOG_TRACE_STR(logEvent) LOG4_CALL_STR(TRACE, logEvent)
#define LOG_TRACE_FMT(fmt, ...) LOG4_CALL_FMT(TRACE, fmt, __VA_ARGS__)

#define LOG_DEBUG(logEvent) LOG4_CALL(DEBUG, logEvent)
#define LOG_DEBUG_STR(logEvent) LOG4_CALL_STR(DEBUG, logEvent)
#define LOG_DEBUG_FMT(fmt, ...) LOG4_CALL_FMT(DEBUG, fmt, __VA_ARGS__)

#define LOG_INFO(logEvent) LOG4_CALL(INFO, logEvent)
#define LOG_INFO_STR(logEvent) LOG4_CALL_STR(INFO, logEvent)
#define LOG_INFO_FMT(...) LOG4_CALL_FMT(INFO, __VA_ARGS__)

#define LOG_WARN(logEvent) LOG4_CALL(WARN, logEvent)
#define LOG_WARN_STR(logEvent) LOG4_CALL_STR(WARN, logEvent)
#define LOG_WARN_FMT(...) LOG4_CALL_FMT(WARN, __VA_ARGS__)

#define LOG_ERROR(logEvent) LOG4_CALL(ERROR, logEvent)
#define LOG_ERROR_STR(logEvent) LOG4_CALL_STR(ERROR, logEvent)
#define LOG_ERROR_FMT(...) LOG4_CALL_FMT(ERROR, __VA_ARGS__)

#if !defined(LOG4CPLUS_DISABLE_ERROR)	
	#define CHK_ERROR_RETURN(eval, retVal, fmt, ...) \
	if (eval) \
	{ \
		LOG_ERROR_FMT(fmt, __VA_ARGS__); \
		return retVal; \
	}

	#define CHK_ERROR(eval, fmt, ...) \
	if (eval) \
	{ \
		LOG_ERROR_FMT(fmt, __VA_ARGS__); \
	}
#else
	#define CHK_ERROR_RETURN(eval, retVal, fmt, ...) \
	if (eval) \
	{ \
		return retVal; \
	}

	#define CHK_ERROR(eval, fmt, ...) LOG4_NOTHING()
#endif // ! LOG4CPLUS_DISABLE_ERROR

namespace RMXLogger
{
	const static wchar_t* DEFAULT_CONFIG_FILENAME = L"log4cplus.properties";

	static log4cplus::tstring s_loggerName = L"";
	static log4cplus::tstring s_defaultConfigFileDir = L"";

	#define DBGLOG(fmt, ...) \
	{ \
		wchar_t sMsgBuf[1024]; \
		swprintf_s(sMsgBuf, 1024, __LOG_CATEGORY__ fmt, __VA_ARGS__); \
		OutputDebugStringW(sMsgBuf); \
		log4cplus::tcout << sMsgBuf << L"\n"; \
	}

	static inline log4cplus::Logger GetLogger()
	{
		return s_loggerName.empty() ? log4cplus::Logger::getRoot() : log4cplus::Logger::getInstance(s_loggerName);
	}

	//
	// Initialize must be called before doing any logging.
	// It is recommended to call it in main or entry point of process/dll.
	static inline void Initialize2(const log4cplus::tstring& loggerName, const log4cplus::tstring& configFile)
	{
		log4cplus::initialize();

		// Enable internal debug logging
#ifndef NDEBUG
		log4cplus::helpers::LogLog::getLogLog()->setInternalDebugging(true);
#endif // !NDEBUG

		struct _stat status;
		if (_wstat(configFile.c_str(), &status) != 0)
		{
			DBGLOG(L"[Error] Cannot initialize logger system (error: configuration file not found '%s')", configFile.c_str());
		}
		log4cplus::PropertyConfigurator::doConfigure(configFile);

		s_loggerName = loggerName;
		DBGLOG(L"[INFO] Logger system initialized");
		LOG_DEBUG(L"Logger system initialized. Configure file: " << configFile.c_str());
	}

	//
	// Initialize must be called before doing any logging.
	// It is recommended to call it in main or entry point of process/dll.
	static inline void Initialize(const log4cplus::tstring& configFile)
	{
		// Root logger in use
		Initialize2(L"", configFile);
	}
	
	static inline log4cplus::tstring getEnv(const  log4cplus::tstring& envName)
	{
		wchar_t* envVar;
		size_t requiredSize;
		std::wstring retVar;
		_wgetenv_s(&requiredSize, NULL, 0, envName.c_str());
		if (requiredSize > 0)
		{
			envVar = new wchar_t[requiredSize * sizeof(wchar_t)];
			if (envVar)
			{
				// Get the value of the environment variable.
				if (_wgetenv_s(&requiredSize, envVar, requiredSize, envName.c_str()) == 0)
					retVar = envVar;
				delete[] envVar;
			}
		}
		return retVar;
	}

	//
	// Initialize must be called before doing any logging.
	// It is recommended to call it in main or entry point of process/dll.
	static inline void Initialize3()
	{
#ifdef ENV_LOGCONFIG_DIR
		// Root logger in use
		std::wstring configFile = RMXLogger::getEnv(ENV_LOGCONFIG_DIR);
		if (!configFile.empty())
		{
			if (configFile.find_last_of(L"/\\") != configFile.length() - 1)
			{
				configFile += L"\\";
			}
			configFile += DEFAULT_CONFIG_FILENAME;
			Initialize2(L"", configFile);
		}		
#else
		DBGLOG(L"[ERROR] Cannot initialize logger system (error: ENV_LOGCONFIG_DIR not defined)");
#endif
	}

	//
	// close and remove all appenders in all the loggers
	static inline void Shutdown()
	{
		log4cplus::Logger::shutdown();
	}

	// Helper class to provide log guard facilities when function is entering, and automatically log when exiting
	// e.g: 
	// 1. Define a macro like #define OTKCALL_LOG_ENTER RMXLogHlp::CallLogGuard otkLogGuard(L"CREO CALL", LOG4CPLUS_TEXT(LOG4CPLUS_MACRO_FUNCTION()));
	// 2. Call macro in function like void Foo() { OTKCALL_LOG_ENTER DoSth(); }
	// 3. Result log: 
	//		Entering CREO CALL - Foo()
	//		Do sth log if any...
	//		Exiting CREO CALL - Foo()
	class CallLogGuard
	{
	public:
		CallLogGuard(const log4cplus::tstring& domain, const log4cplus::tstring& funcName, const log4cplus::tstring& msg = log4cplus::tstring(), bool tracelevel = false)
			: m_domain(domain), m_funcName(funcName), m_msg(msg), m_traceLevel(tracelevel)
		{
			// For backwards-compatibility, support both debug and trace. 
			// For performance optimazation, it's recommended to use trace level.
			if (tracelevel) 
				LOG_TRACE_FMT(L"Entering %s - %s %s", m_domain.c_str(), m_msg.c_str(), m_funcName.c_str());
			else
				LOG_DEBUG_FMT(L"Entering %s - %s %s", m_domain.c_str(), m_msg.c_str(), m_funcName.c_str());
		}
		~CallLogGuard()
		{
			if (m_traceLevel)
				LOG_TRACE_FMT(L"Exiting %s - %s %s", m_domain.c_str(), m_msg.c_str(), m_funcName.c_str());
			else
				LOG_DEBUG_FMT(L"Exiting %s - %s %s", m_domain.c_str(), m_msg.c_str(), m_funcName.c_str());
		}
	private:
		log4cplus::tstring m_domain;
		log4cplus::tstring m_funcName;
		log4cplus::tstring m_msg;
		bool m_traceLevel;
	};
};

#endif // LOG4CPLUS_ENABLE