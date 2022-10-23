#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
//#include <experimental/filesystem>
#include <locale>
#include <stdio.h>
#include "Section.h"

class TCXMLParser {
private:
	bool endWith(char* input, const std::string &suffix);
	std::string uriDecode(const std::string & sSrc);

public:
	TCXMLParser();
	~TCXMLParser();

	bool parseTCXML_Export(std::string &fmsTicket, std::string &transientVolumePath);
	//bool parseTCXML_Export(std::string &fmsTicket);
};
