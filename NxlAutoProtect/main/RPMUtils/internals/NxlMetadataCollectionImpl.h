#pragma once
#include <stdafx.h>
#include <internals/Utilities.h>
#include <internals/NxlMetadataPriorities.h>

class NxlMetadataCollectionImpl
{
public:
	inline int count() const {
		return _map.size();
	}
	int populate(const NxlMetadataCollectionImpl &impl, bool checkduplication)
	{
		int nadded = 0;
		for (auto i : impl._map)
		{
			internal_add(i.first, i.second, checkduplication);
		}
		return nadded;
	}
	inline void clear()
	{
		_map.clear();
		_str.clear();
	}
	inline std::pair<const char *, const char *> at(int index) {
		const char *key = nullptr;
		const char *val = nullptr;
		if (index >= 0 && index < count())
		{
			auto iter = _map.begin();
			while (index > 0) {
				iter++;
				index--;
			}
			key = iter->first.c_str();
			val = iter->second.c_str();
		}
		return std::make_pair(key, val);
	}
	const char * to_string()
	{
		// {"ip_classification":["secret"], "gov_classification":["confidential"]};
		//TODO:combine same key?
		std::stringstream ss;
		ss << "{";
		auto keyIter = _map.begin();
		while (keyIter != _map.end())
		{
			//new key found
			auto key = keyIter->first;
			ss << "\"" << key << "\":[";
			auto range = _map.equal_range(key);
			auto k = range.first;
			//get all values with same key
			while (true)
			{
				ss << "\"" << k->second << "\"";
				if (++k != range.second) {
					ss << ", ";
				}
				else {
					break;
				}

			}
			ss << "]";
			//move to next key
			keyIter = range.second;
			if (keyIter != _map.end()) {
				ss << ", ";
			}
			else {
				break;
			}
		}
		ss << "}";
		return (_str = ss.str()).c_str();
	}
	int from_string(const char * tagstr)
	{
		int iprocessed = 0;
		std::string tagString(tagstr);
		DBGLOG("Loading...'%S'", tagstr);
		while (iprocessed < tagString.length())
		{
			auto iKeyStart = tagString.find_first_of('\"', iprocessed);
			TRACELOG("iKeyStart=%d", iKeyStart);
			if (iKeyStart == std::string::npos)
				break;
			auto iKeyEnd = Utilities::find_first_of_unescaped(tagString, '\"', iKeyStart + 1);
			TRACELOG("iKeyEnd=%d", iKeyEnd);
			if (iKeyEnd == std::string::npos)
				break;
			const std::string key = tagString.substr(iKeyStart + 1, iKeyEnd - iKeyStart - 1);

			auto closeBracket = Utilities::find_first_of_unescaped(tagString, ']', iKeyEnd + 1);
			TRACELOG("closeBracket=%d", closeBracket);
			if (closeBracket == std::string::npos)
				break;
			//search values between iValuesStart and iValuesEnd
			iprocessed = iKeyEnd+1;
			while (iprocessed < closeBracket)
			{
				auto iValueStart = tagString.find_first_of('\"', iprocessed + 1);
				TRACELOG("iValueStart=%d", iValueStart);
				if (iValueStart == std::string::npos)
					break;
				if (iValueStart > closeBracket)
					break;
				auto iValueEnd = Utilities::find_first_of_unescaped(tagString, '\"', iValueStart + 1);
				TRACELOG("iValueEnd=%d", iValueEnd);
				if (iKeyEnd == std::string::npos)
					break;
				//found a value
				const std::string value = tagString.substr(iValueStart + 1, iValueEnd - iValueStart - 1);
				if (internal_add(key, value, true))
				{
					DBGLOG("[%d]Key='%S' Value='%S'", count(), key.c_str(), value.c_str());
				}
				else
				{
					DBGLOG("[Duplicated]Key='%S' Value='%S'", key.c_str(), value.c_str());
				}

				iprocessed = iValueEnd;
			}
			iprocessed = closeBracket;
		}
		DBGLOG("Loaded %d tags", count());
		return count();
	}
	inline bool internal_add(const std::string &key, const std::string &val, bool checkDuplication)
	{
		if (key.empty() || val.empty()) return false;
		if (checkDuplication)
		{
			auto values = _map.equal_range(key);
			for (auto i = values.first; i != values.second; i++)
			{
				if (i->second == val) {
					return false;
				}
			}
		}
		_map.insert(std::make_pair(key, val));
		return true;
	}
	NxlMetadataCollectionImpl ReduceWithPriorities(	const NxlMetadataPriorities &priorities) const
	{
		NxlMetadataCollectionImpl reduced;
		for (auto keyIter = _map.begin();
			keyIter != _map.end();)
		{
			//new key found
			auto key = keyIter->first;
			//check priority definition for this key
			auto keyDef = priorities.find(key);//TODO:TBD:multiple definitions
			std::vector<std::string> priorityDef;
			if (keyDef != priorities.end()) {
				//has priority definition for this key
				priorityDef = keyDef->second;
			}
			else
			{
				//DBGLOG("[key-'%S']:no priority definition", key.c_str());
			}
			//get all values with same key
			auto range = _map.equal_range(key);
			std::string valueWithHighestPriority;
			int highest = priorityDef.size();//higher index has lower priority
			for (auto k = range.first; k != range.second; k++) 
			{
				int priority = Utilities::index_of(priorityDef, k->second);
				if (priority < 0) {
					//not exist, add anyway
					if (reduced.internal_add(k->first, k->second, true))
					{
						DBGLOG("[key-'%S' val-'%S']:no priority definition(ADDED)", key.c_str(), k->second.c_str());
					}
					else
					{
						DBGLOG("[key-'%S' val-'%S']:no priority definition(DUPLICATED)", key.c_str(), k->second.c_str());
					}
				}
				else
				{
					//has priority definition, search for highest one
					DBGLOG("[key-'%S' val-'%S']:Priority=%d Highest=%d", key.c_str(), k->second.c_str(), priority, highest);
					if (priority < highest) {
						//higher priority
						highest = priority;
						valueWithHighestPriority = k->second;
					}
				}
			}
			if (reduced.internal_add(key, valueWithHighestPriority, false))
			{
				DBGLOG("Added(key-'%S' val-'%S'):HighestPriority=%d(ADDED)", key.c_str(), valueWithHighestPriority.c_str(), highest);
			}
			//move to next key
			keyIter = range.second;
		}
		return reduced;
	}
private:
	std::unordered_multimap<std::string, std::string> _map;
	std::string _str;
};
