#pragma once
#include <stdafx.h>
#include <codecvt>

class Utilities
{
public:
	// convert UTF-8 string to wstring
	inline static std::wstring utf8_to_wstring(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	}

	// convert wstring to UTF-8 string
	inline static std::string wstring_to_utf8(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(str);
	}
	inline static int index_of(const std::vector<std::string> &v, const std::string &s) {
		if (v.empty()) return -1;
		auto i = std::find(v.begin(), v.end(), s);
		if (i == v.end()) return -1;
		auto pos = i - v.begin();
		return pos;
	}
	inline static std::size_t find_first_of_unescaped(const std::string &str, char c, std::size_t istart)
	{
		bool inquote = false;
		for (std::size_t i = istart; i < str.length(); i++)
		{
			if (!inquote&&str[i] == c) return i;
			switch (str[i])
			{
				case '\\':
					i++;
					break;
				case '\"':
					inquote = !inquote;
					break;
				default:
					break;
			}
		}
		return std::string::npos;
	}
};