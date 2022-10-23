#pragma once

#include <string>
#include <vector>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

struct SwimInputXMLObject {
	std::string saveFlag;
	std::string selectedFlag;
	std::string name;
	std::string type;
	std::string file_name;
	std::vector<std::string> child_components;
	std::string first_save;
	std::string item_rev_uid;
	std::string item_name_enablement;
};


struct SwimResponseXMLObject {
	std::string master;
	std::string name;
	std::string new_item_id;
	std::string new_model_name;
	std::string type;
};


struct InputXMLObj {
	InputXMLObj() :saveJTFlag(false) {

	}
	~InputXMLObj() {

	}
	bool saveJTFlag;
	std::vector<SwimInputXMLObject> modelDocList;
};

class SwimXMLParser {
public:
	SwimXMLParser();
	~SwimXMLParser();

	bool ParseSwimInputXML(const std::string &xmlpath, InputXMLObj &inputXMLObj);
	bool ParseSwimResponseXML(const std::string &xmlpath, vector<SwimResponseXMLObject> &responseObjList);
	bool UpdateSwimRegistry(const std::string& xmlpath);
	void WriteXML(xercesc::DOMDocument* xmlDoc, const std::string & filePath);
private:
	xercesc::XercesDOMParser *m_ConfigFileParser;

};


