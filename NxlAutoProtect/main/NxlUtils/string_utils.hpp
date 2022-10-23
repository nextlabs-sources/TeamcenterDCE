#pragma once
#include <stdio.h>
#include <algorithm>
#include <cctype>
#ifdef WNT
#include <direct.h>
#endif
#include <sys/stat.h>

inline bool iequals(const std::string& a, const std::string& b)
{
	size_t sz = a.size();
	if (b.size() != sz)
		return false;
	for (size_t i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}

inline std::string & ltrim(std::string &str) {
	//str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char ch) {return !std::isspace(ch); }));
	auto first = str.begin();
	for (; first != str.end(); first++)
	{
		if (!std::isspace(*first))
		{
			break;
		}
	}
	str.erase(str.begin(), first);
	return str;
}

inline std::string & rtrim(std::string &str) {
	//str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch) {return !std::isspace(ch); }).base(), str.end());
	auto last = str.rbegin();
	for (; last != str.rend(); last++) {
		if (!std::isspace(*last))
			break;
	}
	str.erase(last.base(), str.end());
	return str;
}
inline std::string & trim(std::string &str) {
	ltrim(str);
	rtrim(str);
	return str;
}
inline std::string trim_copy(std::string str) {
	ltrim(str);
	rtrim(str);
	return str;
}
inline std::string &dequote(std::string &str) {
	if (str.length() < 2
		|| str.front()!='\"'
		||str.back() !='\"') return str;
	str.erase(0, 1);
	str.erase(str.size() - 1);
	return str;
}

inline bool begin_with(const std::string &searchIn, const std::string &prefix) {
	return searchIn.size() >= prefix.size()
		&& searchIn.compare(0, prefix.size(), prefix) == 0;
}
inline std::string FindDirectoryInPath(std::string filePath) {
	auto lastBackSlash = std::find_if(filePath.rbegin(), filePath.rend(), [](char ch) {return ch == PATH_DELIMITER_CHAR; });
	if (lastBackSlash == filePath.rend()) return std::string();
	++lastBackSlash;
	filePath.erase(lastBackSlash.base(), filePath.end());
	return filePath;
}
inline std::string FindFileNameInPath(std::string filePath) {
	auto lastBackSlash = std::find_if(filePath.rbegin(), filePath.rend(), [](char ch) {return ch == PATH_DELIMITER_CHAR; });
	if (lastBackSlash == filePath.rend()) return filePath;
	filePath.erase(filePath.begin(), lastBackSlash.base());

	return filePath;
}

inline std::string RemoveNxlExtension(const std::string &file) {
	auto ilastdot = file.find_last_of('.');
	if (ilastdot == std::string::npos)
		return file;
	if (iequals(".nxl", file.substr(ilastdot))) {
		return file.substr(0, ilastdot);
	}
	return file;
}
inline bool file_exists(const std::string& file) {
	struct stat buffer;
	return (stat(file.c_str(), &buffer) == 0);
}
inline bool dir_exists(const std::string& dir) {
	struct stat s = { 0 };

	return !stat(dir.c_str(), &s) && (s.st_mode & S_IFDIR);
}

template<typename TArgs>
TArgs &AppendPath(TArgs& first, const TArgs& second) {
	if(first.back() != PATH_DELIMITER_CHAR)
		first += PATH_DELIMITER;
	first += second;
	return first;
}

#if _MSC_VER > 1600 || defined(__linux__)
template<typename T, typename... TArgs>
T &AppendPath(T& first, const T& second, const TArgs&... args) {
	AppendPath(first, second);
	return AppendPath(first, args...);
}


// Modify multiple strings. Here we use parameters pack by `...T`
template<typename TFirst, typename... TArgs>
TFirst BuildPath(const TFirst& first, const TFirst& second, const TArgs&... args)
{
	TFirst ret(first);
	AppendPath(ret, second, args...);
	return ret;
}
#else
inline std::string &BuildPath(std::string &first, const std::string &second) {
	return AppendPath(first, second);
}
inline std::string BuildPath(const std::string &first, const std::string &second) {
	std::string  ret(first);
	return AppendPath(ret, second);
}
#endif


/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
inline void mkpath(const std::string &path)
{
	size_t pos = 0;
	do
	{
		pos = path.find_first_of("\\/", pos + 1);
		auto sub = path.substr(0, pos);
		if (!dir_exists(sub))
		{
#ifdef WNT
			_mkdir(sub.c_str());
#else
			mkdir(sub.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
		}
	} while (pos != std::string::npos);
}


