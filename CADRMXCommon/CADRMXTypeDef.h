#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

//
// Tag types
//
struct TagKeyComp
{
	bool operator()(const std::string& key1, const std::string& key2) const
	{
		return _stricmp(key1.c_str(), key2.c_str()) < 0;
	}
};
typedef std::vector<std::string> TTagValueList;
// Case insensitive map
typedef std::map<std::string, TTagValueList, TagKeyComp> TTagList;
typedef TTagList TTagPriorityDefs;

//
// Evaluate attributes types 
//
typedef std::pair<std::wstring, std::wstring> TEvalAttribute;
typedef std::vector<TEvalAttribute> TEvalAttributeList;
