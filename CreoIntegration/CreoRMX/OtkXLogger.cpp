
#include <cstdarg>
#include <vector>
#include <iostream>
#include <fstream>
#include "OtkXLogger.h"

using namespace std;

#define LOGFILE_HEADER _T("NXL ")
#define LOGFILE_TIME _T(__DATE__)_T(" ")##_T(__TIME__)_T(" ")
#define va_copy(destination, source) ((destination) = (source))

static const TCHAR* LOG_FILENAME = _T("TcDRM_Creo.log");
static const TCHAR *LOGTYPE_DEBUG_STR = _T("[Debug] ");
static const TCHAR *LOGTYPE_WARN_STR = _T("[WARNING] ");
static const TCHAR *LOGTYPE_INFO_STR = _T("[INFO] ");
static const TCHAR *LOGTYPE_ERROR_STR = _T("[ERROR] ");

COtkXLogger::COtkXLogger()
{
	
}

COtkXLogger& COtkXLogger::getInstance()
{
	static COtkXLogger s_instance;
	return s_instance;
}

COtkXLogger::~COtkXLogger(void)
{
}

tstring COtkXLogger::Format(LogLevel type, const TCHAR *formatMsg, ...) const
{
	va_list vl;
	va_start(vl, formatMsg);
	tstring retMsg = Format(type, formatMsg, vl);
	va_end(vl);
	return retMsg;
}

tstring COtkXLogger::Format(LogLevel type, const TCHAR *formatMsg, va_list vl) const
{
	tstring msg(LOGFILE_HEADER);
	switch (type)
	{
	case COtkXLogger::LogLevel_Debug:
		msg.append(LOGTYPE_DEBUG_STR);
		break;
	case COtkXLogger::LogLevel_Warn:
		msg.append(LOGTYPE_WARN_STR);
		break;
	case COtkXLogger::LogLevel_Error:
		msg.append(LOGTYPE_ERROR_STR);
		break;
	case COtkXLogger::LogLevel_Info:
		msg.append(LOGTYPE_INFO_STR);
		break;
	default:
		break;
	}

	static const int DEFAULT_BUFFERSIZE = 1024;
	vector<TCHAR> buffer(DEFAULT_BUFFERSIZE);
	
	int size = _vsntprintf(&buffer[0], buffer.size() - 1, formatMsg, vl);	
	if( size >= DEFAULT_BUFFERSIZE)
	{
		buffer.resize(size + 1);
		va_list vlCopy;
		va_copy(vlCopy, vl);
		_vsntprintf(&buffer[0], buffer.size(), formatMsg, vlCopy);
		va_end(vlCopy);
	}

	msg.append(&buffer[0]);

	return msg;
}

void COtkXLogger::LogToConsole(const tstring& msg)
{	
	OutputDebugString(msg.c_str());
}

void COtkXLogger::LogToFile(const tstring& msg)
{
	std::wofstream logFile(LOG_FILENAME, ios_base::out | ios_base::app);
	if (logFile.fail())
	{
		tstring message = Format(LogLevel_Error, _T("Failed to open file %s\n"), LOG_FILENAME);
		OutputDebugString(message.c_str());
		return;
	}
	logFile << LOGFILE_TIME << msg.c_str() << endl;
	logFile.close();
}

void COtkXLogger::Debug(const TCHAR* formatMsg, ...)
{
	va_list vl;
	va_start(vl, formatMsg);
	tstring msg = Format(LogLevel_Debug, formatMsg, vl);
	va_end(vl);

	LogToConsole(msg);	
}

void COtkXLogger::Warn(const TCHAR* formatMsg, ...)
{
	va_list vl;
	va_start(vl, formatMsg);
	tstring msg = Format(LogLevel_Warn, formatMsg, vl);
	va_end(vl);
	LogToConsole(msg);
	LogToFile(msg);

}

void COtkXLogger::Info(const TCHAR* formatMsg, ...)
{
	va_list vl;
	va_start(vl, formatMsg);
	tstring msg = Format(LogLevel_Warn, formatMsg, vl);
	va_end(vl);
	LogToConsole(msg);
	LogToFile(msg);
}

void COtkXLogger::Error(const TCHAR* formatMsg, ...)
{
	va_list vl;
	va_start(vl, formatMsg);
	tstring msg = Format(LogLevel_Warn, formatMsg, vl);
	va_end(vl);
	LogToConsole(msg);
	LogToFile(msg);
}
