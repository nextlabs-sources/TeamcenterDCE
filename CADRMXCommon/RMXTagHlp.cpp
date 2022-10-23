#include "RMXTagHlp.h"

#include <memory>
#include <set>

#include "RMXLogger.h"
#include "RMXUtils.h"

#include <SDLResult.h>
#include <SDLTypeDef.h>
#include <SDLUser.h>
#include <nlohmann/json.hpp>

using namespace std;

//
// Namespace to provide all internal facilities
//
namespace TagHlp
{
	//
	// Global variables
	//
	static const wchar_t* TAGOBLIGATION_NAME = L"TCDRM_TAGCONFIG";
	static const wchar_t* TAGRESOURCE_TYPE = L"nxcadrmx"; // hard-coded in CC.
	static const vector < pair<wstring, wstring> > TAG_ATTRIBUTE = { make_pair(wstring(L"configuration"), wstring(L"TeamcenterRMX")) };

	static const wchar_t* TAGKEY_PARAM = L"TagKey";
	static const wchar_t* TAGVALUE_PARAM = L"ValueList";

	// Rename for standard-alone CAD RMX w/o "Teamcenter".
	static const wchar_t* TAGOBLIGATION_NAME2 = L"EDRM_TAGCONFIG"; // hard-coded in CC 
	static const vector < pair<wstring, wstring> > TAG_ATTRIBUTE2 = { make_pair(wstring(L"configuration"), wstring(L"CADRMX")) };

	bool GetTagPriorityDefsByObligation(
		ISDRmUser* pDRmUser,
		const std::vector<std::pair<std::wstring, std::wstring>> &attrs,
		const wstring& obligationName,
		TTagPriorityDefs& tagPriorityDefs)
	{
		if (pDRmUser == nullptr)
			return false;

		// Since RMD v2021.05, need to pass resource name and resource type
		// Previouly, these 2 arguments are always ignored.
		wchar_t szProcessName[MAX_PATH];
		if (GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0)
		{
			CHK_ERROR_RETURN(true, false, L"Cannot get the file name of current process (error: %u)", GetLastError());
			return false;
		}

		// fso
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_OBLIGATION_INFO>>> rightsAndObligations;
		SDWLResult result = pDRmUser->GetResourceRightsFromCentralPolicies(szProcessName, TAGRESOURCE_TYPE, attrs, rightsAndObligations);
		if (!result)
		{
			LOG_ERROR_FMT(L"Failed to get EDRM_TAGCONFIG obligation for resource type: %s (error: %s)", TAGRESOURCE_TYPE, result.ToString().c_str());
			return false;
		}

		for (const auto& rightOb : rightsAndObligations)
		{
			if (rightOb.first != RIGHT_CLASSIFY)
				continue;

			for (const auto& ob : rightOb.second)
			{
				if (_wcsicmp(ob.name.c_str(), obligationName.c_str()) != 0)
					continue;

				string key;
				string strValues;
				TTagValueList values;
				for (const auto& option : ob.options)
				{
					// Must have value
					if (option.first.empty() || option.second.empty())
						continue;

					if (_wcsicmp(option.first.c_str(), TAGKEY_PARAM) == 0)
					{
						key = RMXUtils::ws2s(option.second);
					}
					else if (_wcsicmp(option.first.c_str(), TAGVALUE_PARAM) == 0)
					{
						strValues = RMXUtils::ws2s(option.second);
						stringstream tokValues(strValues);
						string val;
						// Tokenizing value of ValueList param by ';'
						while (getline(tokValues, val, ';'))
						{
							values.push_back(val);
						}
					}
					if (!key.empty() && !values.empty())
					{
						// Only take the first one
						if (tagPriorityDefs.find(key) != tagPriorityDefs.end())
						{
							LOG_WARN_FMT(L"Duplicated 'TagKey' defined in %s obligation (TagKey: %s, ValueList: %s). Ignored", obligationName.c_str(),
								RMXUtils::s2ws(key).c_str(), RMXUtils::s2ws(strValues).c_str());
						}
						else
						{
							tagPriorityDefs[key] = values;
							LOG_INFO_FMT(L"'%s' obligation found in policy: \n\t-TagKey: %s\n\t-ValueList: %s)", obligationName.c_str(),
								RMXUtils::s2ws(key).c_str(), option.second.c_str());
						}
						key.clear();
						strValues.clear();
						values.clear();
					}
				}

			}
		}

		return true;
	}

	void GetAllTagPriorityDefs(ISDRmUser* pDRmUser, TTagPriorityDefs& tagPriorityDefs)
	{
		tagPriorityDefs.clear();
		// Since RMX5.2, rebrand as EDRM instead of TeamcenterDRM.
		GetTagPriorityDefsByObligation(pDRmUser, TAG_ATTRIBUTE2, TAGOBLIGATION_NAME2, tagPriorityDefs);

		// Backwards-compatibility: Support old configuration in policy.
		GetTagPriorityDefsByObligation(pDRmUser, TAG_ATTRIBUTE, TAGOBLIGATION_NAME, tagPriorityDefs);
	}

} // namespace TagHlp

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

	void CTagCollection::Merge(ISDRmUser* pDRmUser)
	{
		// Only merge the tags in case there are multiple tags separated by {}
		size_t nextPos = m_initialTags.find_first_of('{', 0);
		if (nextPos == string::npos || m_initialTags.find_first_of('{', nextPos + 1) == string::npos)
		{
			return;
		}
		
		// Populate as map structure
		PopulateToList();

		// Read the priority definitions from policy obligation
		TagHlp::GetAllTagPriorityDefs(pDRmUser, g_tagPriorityDefs);

		// Scan all tag values and determine the higher priority ones which will be taken into account.
		// Priority is defined in policy obligation like "top-secret;supper-secret;secret;notsecret"
		// e.g.: {"ip_classification":["secret"], "gov_classification":["confidential", "test"]}{"ip_classification":["top-secret"]}
		// Expected result: {"ip_classification":["top-secret"], "gov_classification":["confidential"]}
		for (const auto& prioDef : g_tagPriorityDefs)
		{
			auto tagIter = m_tagList.find(prioDef.first);
			if (tagIter != m_tagList.end() && tagIter->second.size() > 1)
			{
				const TTagValueList& prioValues = prioDef.second;
				size_t lastPriority = prioValues.size();
				string higherPrioValue;
				TTagValueList undefinedPrioValues;
				for (const auto& checkValue : tagIter->second)
				{
					// Case sensitive?
					auto prioValueIter = std::find_if(prioValues.cbegin(), prioValues.cend(), [&checkValue](const string &prioValue)
					{
						return _stricmp(checkValue.c_str(), prioValue.c_str()) == 0;
					});
					if (prioValueIter == prioValues.end())
					{
						// priority is undefined for this tag value
						undefinedPrioValues.push_back(checkValue);
						continue;
					}
					// Tag is sorted by priority. Smaller index, higher priority.
					size_t currPriority = std::distance(prioValues.begin(), prioValueIter);
					if (currPriority < lastPriority)
					{
						higherPrioValue = checkValue;
						lastPriority = currPriority;
					}
				}

				// Finally, generate new value list
				tagIter->second.clear();
				if (!higherPrioValue.empty())
					tagIter->second.push_back(higherPrioValue);
				if (undefinedPrioValues.size() > 0)
					tagIter->second.insert(tagIter->second.end(), undefinedPrioValues.begin(), undefinedPrioValues.end());
			}
		}
		
		// Re-populate the json string according to the merged tag list
		PopulateToString();

		if (m_initialTags != m_mergedTags)
			LOG_INFO_FMT(L"Tag merged:\n\t-From %s\n\t-To %s", RMXUtils::s2ws(m_initialTags).c_str(), RMXUtils::s2ws(m_mergedTags).c_str());
	}

	void CTagCollection::PopulateToList(const std::string& tags, bool excludeBlackList)
	{
		const nlohmann::json& root = nlohmann::json::parse(tags);
		if (!root.is_object())
		{
			LOG_ERROR_FMT(L"Invalid json tag. Ignore: %s", RMXUtils::s2ws(tags).c_str());
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

	void CTagCollection::PopulateToString()
	{
		try {
			nlohmann::json root = nlohmann::json::object();
			for (const auto& tag : m_tagList)
			{
				root[tag.first] = nlohmann::json::array();
				for (const auto& val : tag.second)
				{
					root[tag.first].push_back(val);
				}
			}

			m_mergedTags = root.dump();
		}
		catch (std::exception &e) {
			LOG_ERROR_FMT(L"Exception when exporting tags to json (error: %s)", RMXUtils::s2ws(e.what()).c_str());
		}
		catch (...) {
			// The JSON data is NOT correct
			LOG_ERROR_STR(L"Unkknown exception when exporting tags to json");
		}
	}

	//
	// API
	//
	bool MergeTags(ISDRmUser* pDRmUser, const std::string& tags, std::string& newTags)
	{
		if (pDRmUser == nullptr || tags.empty())
			return false; // Wrong input

		CTagCollection tagColl(tags);
		tagColl.Merge(pDRmUser);
		newTags = tagColl.str();

		return true;
	}

} // namespace RMXLib
