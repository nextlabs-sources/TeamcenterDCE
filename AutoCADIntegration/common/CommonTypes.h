#pragma once

#include <string>

const static wchar_t* ENV_RMX_ROOT = L"AUTOCAD_RMX_ROOT";
const static wchar_t* RMX_TOOLS_FOLDERNAME = L"tools";
const static wchar_t* EXE_NXL_HELPER = L"Nxlhelper.exe";

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

template<typename T>
class RMXControllerBase {
public:

	static T& GetInstance() {
		static T s_inst;
		return s_inst;
	};

	virtual void Start() = 0;
	virtual void Stop() = 0;

protected:
	RMXControllerBase() {};
	~RMXControllerBase() {};

	RMX_DISABLE_COPY(RMXControllerBase)
};

#define DEFINE_RMXCONTROLLER_GET(className, funName) \
inline className& funName() \
{ \
	return className##::GetInstance(); \
}

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

#define HAS_BIT(flags, mask) ((flags) & (mask)) == (mask)

struct ICaseKeyLess
{
	bool operator()(const std::wstring& path1, const std::wstring& path2) const
	{
		return (_wcsicmp(path1.c_str(), path2.c_str()) < 0);
	}

	bool operator()(const std::string& path1, const std::string& path2) const
	{
		return (_stricmp(path1.c_str(), path2.c_str()) < 0);
	}
};
