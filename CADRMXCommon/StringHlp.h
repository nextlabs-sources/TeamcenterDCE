//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Mar 2022
//////////////////////////////////////////////////////////////////////////////
//


#pragma once
#ifndef __STRINGHLP_H__
#define __STRINGHLP_H__

#include <algorithm>
#include <locale>         // std::locale, std::ctype, std::use_facet
#include <string>

namespace RMXCore
{
	namespace StringHlp
	{
		/*!
		* Coverts bite string to wide string
		*
		*/
		inline std::wstring s2ws(const char* szStr)
		{
			if (szStr == nullptr || strlen(szStr) == 0)
				return std::wstring();

			int len = (int)strlen(szStr) + 1;
			int size = MultiByteToWideChar(CP_UTF8, 0, szStr, len, 0, 0);
			wchar_t* buf = new wchar_t[size];
			MultiByteToWideChar(CP_UTF8, 0, szStr, len, buf, size);
			std::wstring r(buf);
			delete[] buf;
			return r;
		}

		/*!
		* Coverts wide string to bite string
		*
		*/
		inline std::string ws2s(const wchar_t* szStr)
		{
			if (szStr == nullptr || wcslen(szStr) == 0)
				return std::string();

			int len = (int)wcslen(szStr) + 1;
			int size = WideCharToMultiByte(CP_UTF8, 0, szStr, len, 0, 0, 0, 0);
			char* buf = new char[size];
			WideCharToMultiByte(CP_UTF8, 0, szStr, len, buf, size, 0, 0);
			std::string r(buf);
			delete[] buf;
			return r;
		}

		/*!
		 * Coverts bite string to wide string
		 *
		 */
		inline std::wstring s2ws(const std::string& str)
		{
			return s2ws(str.c_str());
		}

		/*!
		 * Coverts wide string to bite string
		 *
		 */
		inline std::string ws2s(const std::wstring& wstr)
		{
			return ws2s(wstr.c_str());
		}
		
		inline std::wstring bstr2ws(BSTR& bs)
		{
			std::wstring ws(bs, SysStringLen(bs));
			return ws;
		};

		inline BSTR ws2bstr(const std::wstring& ws)
		{
			BSTR bs = SysAllocStringLen(ws.data(), (UINT)ws.size());
			return bs;
		};

		/*!
		* Converts the string c to lower case if a lower case form is defined by this locale
		*/
		template<typename CharT>
		inline void ToLower(std::basic_string<CharT>& s, const std::locale& loc = std::locale()) {
			auto& f = std::use_facet<std::ctype<CharT>>(loc);
			std::transform(s.begin(), s.end(), s.begin(),
				[&](CharT c) -> CharT { return f.tolower(c); });
		}

		/*!
		* Converts the string c to upper case if a lower case form is defined by this locale
		*/
		template<typename CharT>
		inline void ToUpper(std::basic_string<CharT>& s, const std::locale& loc = std::locale()) {
			auto& f = std::use_facet<std::ctype<CharT>>(loc);
			std::transform(s.begin(), s.end(), s.begin(),
				[&](CharT c) -> CharT { return f.toupper(c); });
		}

		/*!
		* Replace all occurrences of the string by new contents with case insensitive
		*/
		template<class CharT>
		inline void IReplaceAllString(std::basic_string<CharT>& s, const std::basic_string<CharT>& oldValue, const std::basic_string<CharT>& newValue)
		{
			size_t pos = 0;
			std::basic_string<CharT> lStr(s);
			std::basic_string<CharT> lOldValue(oldValue);
			RMXUtils::ToLower<CharT>(lStr);
			RMXUtils::ToLower<CharT>(lOldValue);
			while ((pos = lStr.find(lOldValue, pos)) != std::basic_string<CharT>::npos) {
				lStr.replace(pos, oldValue.length(), newValue);
				s.replace(pos, oldValue.length(), newValue);
				pos += newValue.length();
			}
		}

		/*!
		* Find the first string in case insensitive
		*/
		template<class CharT>
		inline size_t IFindSubString(const std::basic_string<CharT>& str, const std::basic_string<CharT>& subStr, size_t pos = 0)
		{
			std::basic_string<CharT> lStr(str);
			std::basic_string<CharT> lSubStr(subStr);
			RMXUtils::ToLower<CharT>(lStr);
			RMXUtils::ToLower<CharT>(lSubStr);
			return lStr.find(lSubStr, pos);
		}

		/*!
		* Performs a case-insensitive comparison of strings.
		*/
		inline bool ICompare(const char* sz1, const char* sz2) {
			return (_stricmp(sz1, sz2) == 0);
		}

		/*!
		* Performs a case-insensitive comparison of strings.
		*/
		inline bool ICompare(const wchar_t* sz1, const wchar_t* sz2) {
			return (_wcsicmp(sz1, sz2) == 0);
		}

		/*!
		* Performs a case-insensitive comparison of strings.
		*/
		template<class CharT>
		inline bool ICompare(const std::basic_string<CharT>& s1, const std::basic_string<CharT>& s2) {
			return ICompare(s1.c_str(), s2.c_str());
		}

		/*!
		* Format args according to the format string, and return the result as a string
		*/
		inline std::string FormatString(const char* format, ...)
		{
			va_list vl;
			int     len = 0;
			std::string s;

			va_start(vl, format);
			len = _vscprintf_l(format, 0, vl) + 1;
			static const int DEFAULT_BUFFERSIZE = 1024;
			std::vector<char> buffer(DEFAULT_BUFFERSIZE);
			size_t size = _vsnprintf_s(&buffer[0], buffer.size() - 1, _TRUNCATE, format, vl);
			if (size >= DEFAULT_BUFFERSIZE)
			{
				buffer.resize(size + 1);
				va_list vlCopy;
				va_copy(vlCopy, vl);
				_vsnprintf_s(&buffer[0], buffer.size(), _TRUNCATE, format, vlCopy);
				va_end(vlCopy);
			}

			s.append(&buffer[0]);

			va_end(vl);

			return std::move(s);
		}

		/*!
		* Format args according to the format string, and return the result as a string
		*/
		inline std::wstring FormatString(const wchar_t* format, ...)
		{
			va_list vl;
			int     len = 0;
			std::wstring s;

			va_start(vl, format);
			static const int DEFAULT_BUFFERSIZE = 1024;
			std::vector<wchar_t> buffer(DEFAULT_BUFFERSIZE);

			size_t size = _vsnwprintf_s(&buffer[0], buffer.size() - 1, _TRUNCATE, format, vl);
			if (size >= DEFAULT_BUFFERSIZE)
			{
				buffer.resize(size + 1);
				va_list vlCopy;
				va_copy(vlCopy, vl);
				_vsnwprintf_s(&buffer[0], buffer.size(), _TRUNCATE, format, vlCopy);
				va_end(vlCopy);
			}

			s.append(&buffer[0]);

			va_end(vl);

			return std::move(s);
		}

		template<typename CharT>
		inline void TrimLeft(std::basic_string<CharT>& s)
		{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
				}));
		}

		template<typename CharT>
		inline void TrimRight(std::basic_string<CharT>& s)
		{
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
				}).base(), s.end());
		}

		template<typename CharT>
		inline std::basic_string<CharT> Trim(std::basic_string<CharT>& s)
		{
			// Trim leading and trailing space
			TrimLeft<CharT>(s);
			TrimRight<CharT>(s);
		}
	} //namespace StringHlp
}; //namespace RMXCore

#endif /* __STRINGHLP_H__ */