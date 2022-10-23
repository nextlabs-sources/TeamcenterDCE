#include "stdafx.h"
#include <RPMUtils.h>
#include <stdlib.h>

inline char *string_dup(const char *str)
{
	return str != nullptr ? strdup(str) : nullptr;
}

inline void string_free(char *str) {
	if (str != nullptr) {
		free(str);
	}
}

ResultInfo::ResultInfo(DWORD code, const char * msg, const char * func, const char * file, int line):_code(code), _innerError(nullptr), _line(line)
{
	_msg = string_dup(msg);
	_func = string_dup(func);
	_file = string_dup(file);
}

ResultInfo::ResultInfo(const ResultInfo &inner, DWORD code, const char *msg, const char *func, const char *file, int line):ResultInfo(code, msg, func, file, line)
{
	_innerError = new ResultInfo(inner);
}

ResultInfo::~ResultInfo()
{
	clear();
}

void ResultInfo::clear()
{
	_code = 0;
	string_free(_msg);
	_msg = nullptr;
	string_free(_func);
	_func = nullptr;
	string_free(_file);
	_file = nullptr;
	_line = 0;
	if (_innerError != nullptr)
	{
		delete _innerError;
		_innerError = nullptr;
	}
}

void ResultInfo::copy_from(const ResultInfo & err)
{
	if (this == &err) return;
	//copy
	_msg = string_dup(err._msg);
	_func = string_dup(err._func);
	_file = string_dup(err._file);
	_line = err._line;
	_code = err._code;
	if (err._innerError != nullptr)
	{
		_innerError = new ResultInfo(*(err._innerError));
	}
}

ResultInfo::ResultInfo(const ResultInfo &err):ResultInfo()
{
	copy_from(err);
}
ResultInfo & ResultInfo::operator=(const ResultInfo &err)
{
	clear();
	copy_from(err);
	return *this;
}
