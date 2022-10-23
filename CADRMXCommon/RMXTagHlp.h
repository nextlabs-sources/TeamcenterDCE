#pragma once

#include "CADRMXTypeDef.h"

// Forward declarations
class ISDRmUser;

namespace RMXLib
{
	/*!
	* Merge tags according to the meta data priority defined in central policies
	*
	* \param pDRmUser: Pointer to SkyDRM user interface
	* \param tags: Json-style string representation containing multiple tags segments . 
	*				e.g.: {"ip_classification":["secret"],  "gov_classification" : ["confidential", "test"]} {"ip_classification":["top-secret"]}
	* \param newTags: Return new json-style string containing the merged tags.
	* \return bool: False if call is failure 
	* Note: 
	* - Ignore if policy not found or input tag string only contain single segment.
	* - The first segment will be treated as main tags. The tags in blacklist will still be taken into merging process.
	* Usage example: 
	* #include <RMXTagHlp.h>
	* string newTags;
	* // Example 1: main tags not specified by {}. last_mode_user included in blacklist will be ignored. 
	* RMXLib::MergeTags(pUser, "{}{"last_mod_user":["tcadmin"],"ip_classification":["secret"]}{"ip_classification":["top-secret"]}, newTags);
	* // Example 2: main tags specified. 
	* RMXLib::MergeTags(pUser, "{"last_mod_user":["tcadmin"],"ip_classification":["secret"]}{"ip_classification":["top-secret"]}, newTags);
	*/
	bool MergeTags(ISDRmUser* pDRmUser, const std::string& tags, std::string& newTags);

	class CTagCollection
	{
	public:
		CTagCollection(const std::string& tags) : m_initialTags(tags), m_mergedTags(tags) {}
		
		std::string str() const;
		TTagList toList();
		TEvalAttributeList toEvalAttrs();

		void Merge(ISDRmUser* pDRmUser);
	
	private:
		void PopulateToString();
		void PopulateToList();
		void PopulateToList(const std::string& tags, bool excludeBlackList);
		void PopulateToEvalAttrs();

	private:
		std::string m_initialTags;
		std::string m_mergedTags;
		TTagList m_tagList;
		TEvalAttributeList m_evalAttrs;
	};
}; // namespace RMXLib
