#include "CppUnitTest.h"
#include <wchar.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#if !defined(__STRINGIZE)
#define __STRINGIZE(X) #X
#endif
#if !defined(_STRINGIZE)
#define _STRINGIZE(X) __STRINGIZE(X)
#endif

#if !defined(_W)
#define __WTEXT(x) L ## x
#define _W(x) __WTEXT(x)
#endif

#define _P(path) std::replace(path.begin(), path.end(), '\\', '\\\\');

#define UTEST_LOG_ENTER Logger::WriteMessage("TEST_METHOD: " __FUNCTION__);
#define UTEST_LOG(msg) Logger::WriteMessage(msg)
#define UTEST_LOG_FMT(fmt, ...) \
	{ \
		wchar_t sMsgBuf[1024]; \
		swprintf_s(sMsgBuf, 1024, fmt, __VA_ARGS__); \
		Logger::WriteMessage(sMsgBuf);\
	}
