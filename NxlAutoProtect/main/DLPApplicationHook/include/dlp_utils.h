#ifndef NXL_DLP_UTILS_H_INCLUDED
#define NXL_DLP_UTILS_H_INCLUDED
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

int error;

#define FILE_DELETE(x) (!(error = unlink(x)))
#define FILE_RENAME(x, y) (error = rename(x, y))==0

#endif//NXL_DLP_UTILS_H_INCLUDED