#include "RMXTagHlp.h"

#include <memory>
#include <set>

#include "RMXUtils.h"

#include <SDLResult.h>
#include <SDLTypeDef.h>
#include <SDLUser.h>
#include "json.hpp"

using namespace std;

//
// Namespace to provide all internal facilities
//

namespace RMXLib
{
	static TTagPriorityDefs g_tagPriorityDefs;
	// TODO: Support configuration? 
	// Case insensitive
	static set<string, TagKeyComp> g_tagMergeBlacklist{
		"revision_number",
		"last_mod_user",
		"last_mod_date",
		"ref_types",
		"ref_names" };

	//
	// Class CTagCollection
	//
	string CTagCollection::str() const
	{
		return m_mergedTags;
	}

	TTagList CTagCollection::toList()
	{
		if (m_tagList.empty() && !m_mergedTags.empty())
			PopulateToList();

		return m_tagList;
	}

	TEvalAttributeList CTagCollection::toEvalAttrs()
	{
		if (m_evalAttrs.empty() && !m_initialTags.empty())
			PopulateToEvalAttrs();

		return m_evalAttrs;
	}

	void CTagCollection::PopulateToList(const std::string& tags, bool excludeBlackList)
	{
		const nlohmann::json& root = nlohmann::json::parse(tags);
		if (!root.is_object())
		{
			return;
		}
		for (auto itTag = root.begin(); itTag != root.end(); ++itTag)
		{
			bool inBlackList = g_tagMergeBlacklist.find(itTag.key()) != g_tagMergeBlacklist.end();
			if (excludeBlackList && inBlackList)
			{
				// Some of tags do not support to merge, except the first give tags
				// which may from the main source file, and always carry all tags.
				continue;
			}
			std::vector<std::string> tagValues = itTag.value().get<std::vector<std::string>>();

			for (const auto& newValue : tagValues)
			{
				auto& tagValueList = m_tagList[itTag.key()];
				if (inBlackList)
				{
					// Some of tags do not support to merge, except the first give tags
					// which may from the main source file, and always carry all tags with ORIGINAL VALUES. 
					tagValueList.push_back(newValue);
				}
				else
				{
					// Remove duplicated values.
					auto existValueItr = std::find_if(tagValueList.begin(), tagValueList.end(), [&newValue](const string& val)
					{
						return _stricmp(newValue.c_str(), val.c_str()) == 0;
					});
					if (existValueItr == tagValueList.end())
						tagValueList.push_back(newValue);
				}			
			}
		}
	}

	void CTagCollection::PopulateToEvalAttrs()
	{
		PopulateToList();
		for (const auto& tag : m_tagList)
		{
			for (const auto& val : tag.second)
			{
				m_evalAttrs.push_back(std::make_pair(RMXUtils::s2ws(tag.first), RMXUtils::s2ws(val)));
			}
		}
	}

	void CTagCollection::PopulateToList()
	{
		size_t nextPos = 0;
		while (nextPos < m_initialTags.length())
		{
			size_t startPos = m_initialTags.find_first_of('{', nextPos);
			if (startPos == string::npos)
				break;

			size_t endPos = m_initialTags.find_first_of('}', startPos + 1);
			if (endPos == string::npos)
				break;

			const string& tagObj = m_initialTags.substr(startPos, endPos - startPos + 1);
			// Always carry all tags from the first tag json object, regardless if blacklist is defined.
			// The purpose is the main is protected, should take its all tags. If main file(Assembly?) is 
			// unprotected, can give '{}' as dummy tag string.
			PopulateToList(tagObj, nextPos > 0? true : false);

			nextPos = endPos;
		}
	}

} // namespace RMXLib
