#pragma once

#include "RMXTypeDef.h"

// Forward declarations
class ISDRmUser;

namespace RMXLib
{
	class CTagCollection
	{
	public:
		CTagCollection(const std::string& tags) : m_initialTags(tags), m_mergedTags(tags) {}
		
		std::string str() const;
		TTagList toList();
		TEvalAttributeList toEvalAttrs();
	
	private:
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
