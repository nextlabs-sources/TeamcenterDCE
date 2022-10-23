#include <RPMUtils.h>
#include <string>

class NxlMetadataCollectionImpl
{
public:
	std::string _tagString;
};
/*
	NxlMetadataCollection
*/
NxlMetadataCollection::NxlMetadataCollection()
{
	_impl = new NxlMetadataCollectionImpl();
}

NxlMetadataCollection::NxlMetadataCollection(const NxlMetadataCollection & tags):NxlMetadataCollection()
{
	_impl->_tagString = tags.str();
}

NxlMetadataCollection::NxlMetadataCollection(const char *str): NxlMetadataCollection()
{
	_impl->_tagString = str;
}

NxlMetadataCollection::~NxlMetadataCollection()
{
	delete _impl;
}

NxlMetadataCollection & NxlMetadataCollection::operator=(const NxlMetadataCollection & tags)
{
	_impl->_tagString = tags.str();
	return *this;
}

const char *NxlMetadataCollection::join(const NxlMetadataCollection & tobemerged)
{
	_impl->_tagString += tobemerged.str();
	return str();
}

const char * NxlMetadataCollection::str() const
{
	return _impl->_tagString.c_str();
}
size_t NxlMetadataCollection::len() const
{
	return _impl->_tagString.length();
}