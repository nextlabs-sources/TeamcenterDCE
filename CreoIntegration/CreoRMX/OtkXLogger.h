#pragma once

#include <common_inc.h>

#define ENABLE_CALLCONTEXT 1 // Enable to print more details about functin call.

#ifdef ENABLE_CALLCONTEXT
	#define __CALLCONTEXT__ _T("(")##_T(__FILE__)##_T(":")##_T(__LINE_STR__)##_T(" ")##_T(__FUNCTION__)##_T(")")	
#else
	#define __CALLCONTEXT__ _T("")
#endif

#define LOG_DEBUG(fmt, ...) COtkXLogger::getInstance().Debug(fmt##__CALLCONTEXT__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) COtkXLogger::getInstance().Info(fmt##__CALLCONTEXT__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) COtkXLogger::getInstance().Warn(fmt##__CALLCONTEXT__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) COtkXLogger::getInstance().Error(fmt##__CALLCONTEXT__, ##__VA_ARGS__)

class COtkXLogger
{
public:
	static COtkXLogger& getInstance();

	// Interface for different level log
	void Debug(const TCHAR *msg, ...);
	void Warn(const TCHAR *msg, ...);
	void Info(const TCHAR *msg, ...);
	void Error(const TCHAR *msg, ...);

private:
	COtkXLogger();
	~COtkXLogger();

	COtkXLogger(const COtkXLogger&) {};
	COtkXLogger& operator = (const COtkXLogger&) {};

private:
	enum LogLevel
	{
		LogLevel_Debug,
		LogLevel_Warn,
		LogLevel_Error,
		LogLevel_Info
	};
	tstring Format(LogLevel type, const TCHAR *formatMsg, va_list vl) const;
	tstring Format(LogLevel type, const TCHAR *formatMsg, ...) const;
	void LogToConsole(const tstring& msg);
	void LogToFile(const tstring& msg);
};

