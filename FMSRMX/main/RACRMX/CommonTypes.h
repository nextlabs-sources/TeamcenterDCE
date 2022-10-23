#pragma once
#include "windows.h"

#if !defined(RMX_DISABLE_COPY)
#define RMX_DISABLE_COPY(Class) \
private: \
	Class(const Class &) {}; \
    Class &operator=(const Class &) {};
#endif

#if !defined(RMX_SINGLETON_DECLARE)
#define RMX_SINGLETON_DECLARE(Class) \
private: \
	Class(); \
	~Class(); \
	RMX_DISABLE_COPY(Class) \
public: \
	static Class& GetInstance() { \
		static Class s_inst; \
		return s_inst; \
	}
#endif

/*#if !defined(__STRINGIZE)
	#define __STRINGIZE(X) #X
#endif
#if !defined(_STRINGIZE)
	#define _STRINGIZE(X) __STRINGIZE(X)
#endif*/

#if !defined(va_copy)
	#define va_copy(destination, source) ((destination) = (source))
#endif

#if !defined(_W)
#define __WTEXT(x) L ## x
#define _W(x) __WTEXT(x)
#endif

#if defined(_MSC_VER)
#define RMX_SUPPRESS_DOWHILE_WARNING()  \
    __pragma (warning (push))                 \
    __pragma (warning (disable:4127))           

#define RMX_RESTORE_DOWHILE_WARNING()   \
    __pragma (warning (pop))

#else
#define RMX_SUPPRESS_DOWHILE_WARNING() /* empty */
#define RMX_RESTORE_DOWHILE_WARNING() /* empty */

#endif

#if !defined(RMX_DOWHILE_NOTHING)
#define RMX_DOWHILE_NOTHING()                 \
    RMX_SUPPRESS_DOWHILE_WARNING()            \
    do { } while (0)                                \
    RMX_RESTORE_DOWHILE_WARNING()
#endif
