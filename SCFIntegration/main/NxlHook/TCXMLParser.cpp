#include "TCXMLParser.h"

const char HEX2DEC[256] =
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

	/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

TCXMLParser::TCXMLParser() {

}

TCXMLParser::~TCXMLParser() {

}

bool TCXMLParser::endWith(char * input, const std::string & suffix)
{
	const std::string dataStr(input);
	return dataStr.size() >= suffix.size() &&
		dataStr.compare(dataStr.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string TCXMLParser::uriDecode(const std::string & sSrc) {
	// Note from RFC1630:  "Sequences which start with a percent sign
	// but are not followed by two hexadecimal characters (0-9, A-F) are reserved
	// for future extension"

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
				&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
		*pEnd++ = *pSrc++;

	std::string sResult(pStart, pEnd);
	delete[] pStart;
	return sResult;
}

bool TCXMLParser::parseTCXML_Export(std::string & fmsTicket, std::string & transientVolumePath)
{
	std::cout << fmsTicket;
	std::string decodedTicket = uriDecode(fmsTicket);
	std::cout << "\n";
	std::cout << decodedTicket << "\n";
	int offset = 0;
	Section sections[11] = {
		Section(std::string("HASH"), 64),
		Section(std::string("VER"), 4),
		Section(std::string("ACCESS_METHOD"), 1),
		Section(std::string("BINARY_FLAG"), 1),
		Section(std::string("FILE_GUID"), 32),
		Section(std::string("T_DELETE_FLAG"), 1),
		Section(std::string("T_EXPIRATION"), 19),
		Section(std::string("T_SITE_ID"), 32),
		Section(std::string("T_USER_ID"), 32),
		Section(std::string("T_VOLUMN_ID"), 32),
		Section(std::string("T_DEFAULT_URL"), 0),
	};

	std::string url;

	//for (Section sec : sections) {
	for (int i = 0; i < sizeof(sections) / sizeof(sections[0]); i++) {
		if (offset > decodedTicket.length()) break;

		if (sections[i].len == 0) {
			url = decodedTicket.substr(offset);
			std::cout << sections[i].name << "=" << url << "\n";
			break;
		}
		else {
			std::cout << sections[i].name << "=" << decodedTicket.substr(offset, sections[i].len) << "\n";
			offset += sections[i].len;
		}
	}
		
	if (url.find(";") == 0) {
		url = url.substr(1);
		std::cout << "URL = " << url << "\n";
	}
	if (url.find("http") == 0) {
		// 4-tier
		url = url.substr(url.find(";") + 1);
		std::cout << "HTTP TcXML File. URL = " << url << "\n";
	}
	if (url.find("\\") == 0) {
		// relative path, looking for rootDir from args
		std::string rootDir = transientVolumePath;
		std::cout << "Relative Path TcXML File. RootDir = " << rootDir << "\n";
			
		url = rootDir + url;
		std::cout << "Relative Path TcXML File. URL = " << url << "\n";
	}

	if (FILE *file = fopen(url.c_str(), "r")) {
		fclose(file);
	} else {
		//return false;
	}

	std::string xmlBak = url;
	xmlBak.append(".bak");
	std::cout << "BAK = " << xmlBak << "\n";

	if (FILE *file = fopen(xmlBak.c_str(), "r")) {
		fclose(file);
		remove(xmlBak.c_str());
	}
	
	// need to review to make sure it is portable
	//std::experimental::filesystem::copy(url, xmlBak)
	std::ifstream in(url.c_str(), std::ios_base::in | std::ios_base::binary);
	std::ofstream out(xmlBak.c_str(), std::ios_base::out | std::ios_base::binary);
	const int BUF_SIZE = 4096;
	char buf[BUF_SIZE];
	do {
		in.read(&buf[0], BUF_SIZE);
		out.write(&buf[0], in.gcount());
	} while (in.gcount() > 0);
	in.close();
	out.close();

	//rapidxml::file<> xmlFile("c:\\temp\\trans5fb38f3000001d0400007958.xml.bak"); // xml file. Get ambigious segfault otherwise.
	//std::ofstream modifiedXmlFile("c:\\temp\\test.xml");
	rapidxml::file<> xmlFile(xmlBak.c_str()); // xml file. Get ambigious segfault otherwise.
	std::ofstream modifiedXmlFile(url.c_str());

	rapidxml::xml_document<> doc;

	doc.parse<rapidxml::parse_full | rapidxml::parse_no_data_nodes | rapidxml::parse_declaration_node | rapidxml::parse_comment_nodes>(xmlFile.data());
	//doc.parse<rapidxml::parse_full | rapidxml::parse_declaration_node | rapidxml::parse_comment_nodes>(xmlFile.data());
	rapidxml::xml_node<> *root_node = doc.first_node("TCXML", 0, false);


	for (rapidxml::xml_node<> *imanfile_node = root_node->first_node("ImanFile", 0, false); imanfile_node != nullptr; imanfile_node = imanfile_node->next_sibling("ImanFile", 0, false)) {
		if (endWith(imanfile_node->first_attribute("original_file_name")->value(), ".prt.nxl")) {
			std::string originalFileName(imanfile_node->first_attribute("original_file_name")->value());
			std::string modifiedFileName = originalFileName;
			modifiedFileName = modifiedFileName.erase(modifiedFileName.find_last_of('.'), std::string::npos);

			char* numBuff = doc.allocate_string(modifiedFileName.c_str());
			imanfile_node->first_attribute("original_file_name")->value(numBuff);
		}
	}

	modifiedXmlFile << doc;
	modifiedXmlFile.close();

	return true;
}
