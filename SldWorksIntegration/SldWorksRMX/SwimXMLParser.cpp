#include "stdafx.h" 
#include "SwimXMLParser.h"
#include "FileUtils.h"
#include <PathEx.h>
#include "SwUserAuthorization.h"
#include "SwUtils.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <stdexcept>

using namespace xercesc;
SwimXMLParser::SwimXMLParser() {
	try
	{
		XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure	
	}
	catch (XMLException& e)	
	{
		char* message = XMLString::transcode(e.getMessage());	
		LOG_ERROR("XML toolkit initialization error: " << message);
		XMLString::release(&message);
		// throw exception here to return ERROR_XERCES_INIT
	}
	m_ConfigFileParser = new XercesDOMParser;
	
	// Configure DOM parser.
	m_ConfigFileParser->setValidationScheme(XercesDOMParser::Val_Never);
	m_ConfigFileParser->setDoNamespaces(false);
	m_ConfigFileParser->setDoSchema(false);
	m_ConfigFileParser->setLoadExternalDTD(false);
}

SwimXMLParser::~SwimXMLParser() {
	// Free memory
	delete m_ConfigFileParser;

	// Terminate Xerces

	try
	{
		XMLPlatformUtils::Terminate();  // Terminate after release of memory
	}
	catch (xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		LOG_ERROR("XML ttolkit teardown error: " << message);
		XMLString::release(&message);
	}

}

bool IsValidFile(const string& xmlpath) {

	struct stat fileStatus;
	errno = 0;
	if (stat(xmlpath.c_str(), &fileStatus) == -1) // ==0 ok; ==-1 error
	{
		if (errno == ENOENT)      // errno declared by include file errno.h
			LOG_ERROR("Path file_name does not exist, or path is an empty string." << xmlpath.c_str());
		else if (errno == ENOTDIR)
			LOG_ERROR("A component of the path is not a directory." << xmlpath.c_str());
		else if (errno == ELOOP)
			LOG_ERROR("Too many symbolic links encountered while traversing the path." << xmlpath.c_str());
		else if (errno == EACCES)
			LOG_ERROR("Permission denied." << xmlpath.c_str());
		else if (errno == ENAMETOOLONG)
			LOG_ERROR("File can not be read" << xmlpath.c_str());
		return false;
	}
	return true;
}

bool SwimXMLParser::ParseSwimInputXML(const string &xmlpath, InputXMLObj &inputXMLObj) {

	// Test to see if the file is ok.

	if (!IsValidFile(xmlpath))
		return false;

	try
	{
		m_ConfigFileParser->parse(xmlpath.c_str());

		// no need to free this pointer - owned by the parent parser object
		xercesc::DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();

		// Get the top-level element: Name is "swim_user_exit". No attributes for "root"

		DOMElement* elementRoot = xmlDoc->getDocumentElement();
		if (!elementRoot) {
			LOG_ERROR("Empty XML document :" << xmlpath.c_str());
			return false;
		}

		bool saveJTFiles = false;

		DOMNodeList *auxFileNodeList = elementRoot->getElementsByTagName(XMLString::transcode("auxiliary_files"));
		if (auxFileNodeList != NULL && auxFileNodeList->getLength() > 0) {
			DOMElement* auxElem = dynamic_cast<DOMElement*>(auxFileNodeList->item(0));
			DOMNodeList* cad2pdmList = auxElem->getElementsByTagName(XMLString::transcode("cadtopdm_control"));
			const  XMLSize_t cad2pdm_count = (int)cad2pdmList->getLength();
			if (cad2pdm_count > 0) {
				for (XMLSize_t xx = 0; xx < cad2pdm_count; ++xx) {
					DOMElement* element = dynamic_cast<DOMElement*>(cad2pdmList->item(xx));
					XMLCh* labelNameNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("label"));
					char *tmplabelNameNode = XMLString::transcode(labelNameNodeAttr);
					string labelNameNode(tmplabelNameNode);
					if (!labelNameNode.compare("Save JT Files")) {
						XMLCh* labelValueNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("value"));
						char *tmpLabelValueNode = XMLString::transcode(labelValueNodeAttr);
						string labelValueNode(tmpLabelValueNode);
						if (!labelValueNode.compare("yes")) {
							saveJTFiles = true;
						}
						XMLString::release(&tmpLabelValueNode);
					}
					XMLString::release(&tmplabelNameNode);
				}
			}
		}

		inputXMLObj.saveJTFlag = saveJTFiles;



		DOMNodeList *nl = elementRoot->getElementsByTagName(XMLString::transcode("model"));
		const  XMLSize_t nodeCount = nl->getLength();

		if (nodeCount > 0) {
			for (XMLSize_t xx = 0; xx < nodeCount; ++xx)
			{
				SwimInputXMLObject inputObj;

				DOMElement* element = dynamic_cast<DOMElement*>(nl->item(xx));   
				XMLCh* nameNodeAttr = (XMLCh* )element->getAttribute(XMLString::transcode("name"));
				char *tmpNameNode = XMLString::transcode(nameNodeAttr);
				string nameNode(tmpNameNode);
				inputObj.name = nameNode;
				XMLString::release(&tmpNameNode);
				//XMLString::release(&nameNodeAttr);

				XMLCh* saveNodeAttr = (XMLCh *)element->getAttribute(XMLString::transcode("save"));
				char *tmpSaveNode = XMLString::transcode(saveNodeAttr);
				string saveNode(tmpSaveNode);
				inputObj.saveFlag = saveNode;
				XMLString::release(&tmpSaveNode);
				//XMLString::release(&saveNodeAttr);

				XMLCh* selectedNodeAttr = (XMLCh *)element->getAttribute(XMLString::transcode("selected"));
				char *tmpSelectedNode = XMLString::transcode(selectedNodeAttr);
				string selectedNode(tmpSelectedNode);
				inputObj.selectedFlag = selectedNode;
				XMLString::release(&tmpSelectedNode);
				//XMLString::release(&selectedNodeAttr);

				XMLCh* typeNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("type"));
				char *tmpTypeNode = XMLString::transcode(typeNodeAttr);
				string typeNode(tmpTypeNode);
				inputObj.type = typeNode;
				XMLString::release(&tmpTypeNode);
				//XMLString::release(&typeNodeAttr);

				XMLCh* firstSaveNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("first_save"));
				char *tmpFirstSaveNode = XMLString::transcode(firstSaveNodeAttr);
				string firstSaveNode(tmpFirstSaveNode);
				inputObj.first_save = firstSaveNode;
				XMLString::release(&tmpFirstSaveNode);
				//XMLString::release(&firstSaveNodeAttr);

				DOMNodeList* itemRevNode = element->getElementsByTagName(XMLString::transcode("item_revision_id"));
				if (itemRevNode != NULL && itemRevNode->getLength() > 0) {
					DOMElement* itemRevUID = dynamic_cast<DOMElement*>(itemRevNode->item(0));
					XMLCh* itemRevUIDAttr = (XMLCh*)itemRevUID->getAttribute(XMLString::transcode("item_rev_uid"));
					char *tmpItemRevUIDNode = XMLString::transcode(itemRevUIDAttr);
					string ItemRevUIDNode(tmpItemRevUIDNode);
					inputObj.item_rev_uid = ItemRevUIDNode;
					XMLString::release(&tmpItemRevUIDNode);
					//XMLString::release(&itemRevUIDAttr);
				}

				DOMNodeList* itemNameNode = element->getElementsByTagName(XMLString::transcode("item_name"));
				if (itemNameNode != NULL && itemNameNode->getLength() > 0) {
					DOMElement* itemNameElem = dynamic_cast<DOMElement*>(itemNameNode->item(0));
					XMLCh* itemNameEnablementNodeAttr = (XMLCh*)itemNameElem->getAttribute(XMLString::transcode("enabled"));
					char *tmpItemNameEnablementNode = XMLString::transcode(itemNameEnablementNodeAttr);
					string itemNameEnablementNode(tmpItemNameEnablementNode);
					inputObj.item_name_enablement = itemNameEnablementNode;
					XMLString::release(&tmpItemNameEnablementNode);
					//XMLString::release(&itemNameEnablementNodeAttr);

				}


				DOMNodeList* fileNameNode = element->getElementsByTagName(XMLString::transcode("file_name"));
				if (fileNameNode != NULL && fileNameNode->getLength() > 0) {
					DOMElement* fileNameElem = dynamic_cast<DOMElement*>(fileNameNode->item(0));
					XMLCh* fileNameNodeAttr = (XMLCh*)fileNameElem->getAttribute(XMLString::transcode("value"));
					char *tmpFileNameNode = XMLString::transcode(fileNameNodeAttr);
					string fileName(tmpFileNameNode);
					inputObj.file_name = fileName;
					XMLString::release(&tmpFileNameNode);
					//XMLString::release(&fileNameNodeAttr);

				}

				if (!typeNode.compare("sldasm") || !typeNode.compare("slddrw")) {
					DOMNodeList* childModelNode = element->getElementsByTagName(XMLString::transcode("child_models"));
					if (childModelNode != NULL && childModelNode->getLength() > 0) {
						DOMElement* childElem = dynamic_cast<DOMElement*>(childModelNode->item(0));
						DOMNodeList* childList = childElem->getElementsByTagName(XMLString::transcode("child"));
						const  XMLSize_t child_count = (int)childList->getLength();
						if (child_count > 0) {
							for (XMLSize_t j = 0; j < child_count; ++j) {
								DOMElement* childElem = dynamic_cast<DOMElement*>(childList->item(j));
								XMLCh* childNameNodeAttr = (XMLCh*)childElem->getAttribute(XMLString::transcode("name"));
								char *tmpChildNameNode = XMLString::transcode(childNameNodeAttr);
								string childNameNode(tmpChildNameNode);
								XMLString::release(&tmpChildNameNode);

								XMLCh* childTypeNodeAttr = (XMLCh*)childElem->getAttribute(XMLString::transcode("type"));
								char *tmpChildTypeNode = XMLString::transcode(childTypeNodeAttr);
								string childTypeNode(tmpChildTypeNode);
								XMLString::release(&tmpChildTypeNode);

								string compName = childNameNode + "." + childTypeNode;
								inputObj.child_components.push_back(compName);
							}
						}
					}
				}
				inputXMLObj.modelDocList.push_back(inputObj);
			}
		}
	}
	catch (xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		LOG_ERROR("Error parsing file: " << message);
		XMLString::release(&message);
		return false;
	}

	return true;
}


//bool SwimXMLParser::ParseSwimResponseXML(const string &xmlpath, vector<SwimResponseXMLObject> &responseObjList) {
//	LOG_INFO("xmlpath = " << xmlpath.c_str());
//
//	if (!IsValidFile(xmlpath))
//		return false;
//
//	try
//	{
//		m_ConfigFileParser->parse(xmlpath.c_str());
//
//		// no need to free this pointer - owned by the parent parser object
//		xercesc::DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();
//
//		// Get the top-level element: Name is "save_operation_response". No attributes for "root"
//
//		DOMElement* elementRoot = xmlDoc->getDocumentElement();
//		if (!elementRoot) {
//			LOG_ERROR("Empty XML document :" << xmlpath.c_str());
//			return false;
//		}
//
//		DOMNodeList *nl = elementRoot->getElementsByTagName(XMLString::transcode("model"));
//		const  XMLSize_t nodeCount = nl->getLength();
//
//		if (nodeCount > 0) {
//			for (XMLSize_t xx = 0; xx < nodeCount; ++xx)
//			{
//				SwimResponseXMLObject swimRespObj;
//				DOMElement* element = dynamic_cast<DOMElement*>(nl->item(xx));
//
//				XMLCh* masterNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("master"));
//				char *tmpMasterNode = XMLString::transcode(masterNodeAttr);
//				string masterNode(tmpMasterNode);
//				swimRespObj.master = masterNode;
//				XMLString::release(&tmpMasterNode);
//				//XMLString::release(&masterNodeAttr);
//
//
//				XMLCh* nameNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("name"));
//				char *tmpNameNode = XMLString::transcode(nameNodeAttr);
//				string nameNode(tmpNameNode);
//				swimRespObj.name = nameNode;
//				XMLString::release(&tmpNameNode);
//				//XMLString::release(&nameNodeAttr);
//
//				XMLCh* itemIdNodeAttr = (XMLCh *)element->getAttribute(XMLString::transcode("new_item_id"));
//				char *tmpItemIdNode = XMLString::transcode(itemIdNodeAttr);
//				string itemIdNode(tmpItemIdNode);
//				swimRespObj.new_item_id = itemIdNode;
//				XMLString::release(&tmpItemIdNode);
//				//XMLString::release(&itemIdNodeAttr);
//
//				XMLCh* newModelNameNodeAttr = (XMLCh *)element->getAttribute(XMLString::transcode("new_model_name"));
//				char *tmpNewModelNameNode = XMLString::transcode(newModelNameNodeAttr);
//				string newModelName(tmpNewModelNameNode);
//				swimRespObj.new_model_name = newModelName;
//				XMLString::release(&tmpNewModelNameNode);
//				//XMLString::release(&newModelNameNodeAttr);
//
//				XMLCh* typeNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("type"));
//				char *tmpTypeNode = XMLString::transcode(typeNodeAttr);
//				string typeNode(tmpTypeNode);
//				swimRespObj.type = typeNode;
//				XMLString::release(&tmpTypeNode);
//				//XMLString::release(&typeNodeAttr);
//
//				responseObjList.push_back(swimRespObj);
//
//			}
//		}
//	}
//	catch (xercesc::XMLException& e)
//	{
//		char* message = xercesc::XMLString::transcode(e.getMessage());
//		LOG_ERROR("Error parsing file: " << message);
//		XMLString::release(&message);
//		return false;
//	}
//
//	return true;
//
//}

bool SwimXMLParser::ParseSwimResponseXML(const string& xmlpath, vector<SwimResponseXMLObject>& responseObjList) {
	LOG_INFO("xmlpath = " << xmlpath.c_str());

	if (!IsValidFile(xmlpath))
		return false;

	try
	{
		m_ConfigFileParser->parse(xmlpath.c_str());

		// no need to free this pointer - owned by the parent parser object
		xercesc::DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();

		// Get the top-level element: Name is "save_operation_response". No attributes for "root"

		DOMElement* elementRoot = xmlDoc->getDocumentElement();
		if (!elementRoot) {
			LOG_ERROR("Empty XML document :" << xmlpath.c_str());
			return false;
		}

		DOMNodeList* nl = elementRoot->getElementsByTagName(XMLString::transcode("model"));
		const  XMLSize_t nodeCount = nl->getLength();

		if (nodeCount > 0) {
			for (XMLSize_t xx = 0; xx < nodeCount; ++xx)
			{
				SwimResponseXMLObject swimRespObj;
				DOMElement* element = dynamic_cast<DOMElement*>(nl->item(xx));

				XMLCh* nameNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("name"));
				char* tmpNameNode = XMLString::transcode(nameNodeAttr);
				string nameNode(tmpNameNode);
				swimRespObj.name = nameNode;
				XMLString::release(&tmpNameNode);
				//XMLString::release(&nameNodeAttr);

				XMLCh* newModelNameNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("new_model_name"));
				char* tmpNewModelNameNode = XMLString::transcode(newModelNameNodeAttr);
				string newModelName(tmpNewModelNameNode);
				swimRespObj.new_model_name = newModelName;
				XMLString::release(&tmpNewModelNameNode);
				//XMLString::release(&newModelNameNodeAttr);

				XMLCh* typeNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("type"));
				char* tmpTypeNode = XMLString::transcode(typeNodeAttr);
				string typeNode(tmpTypeNode);
				swimRespObj.type = typeNode;
				XMLString::release(&tmpTypeNode);
				//XMLString::release(&typeNodeAttr);

				responseObjList.push_back(swimRespObj);

			}
		}
	}
	catch (xercesc::XMLException & e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		LOG_ERROR("Error parsing file: " << message);
		XMLString::release(&message);
		return false;
	}

	return true;

}

bool SwimXMLParser::UpdateSwimRegistry(const string& xmlpath) {
	LOG_INFO("UpdateSwimRegistry. Xmlpath = " << xmlpath.c_str());

	if (!IsValidFile(xmlpath))
		return false;

	try
	{
		m_ConfigFileParser->parse(xmlpath.c_str());

		// no need to free this pointer - owned by the parent parser object
		xercesc::DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();

		// Get the top-level element: Name is "registry". No attributes for "root"

		DOMElement* elementRoot = xmlDoc->getDocumentElement();
		if (!elementRoot) {
			LOG_ERROR("Empty XML document :" << xmlpath.c_str());
			return false;
		}

		DOMNodeList *nl = elementRoot->getElementsByTagName(XMLString::transcode("cad_file"));
		if (nl == NULL || nl->getLength() == 0)
		{
			LOG_DEBUG("cad_file node not found in registry file" << xmlpath.c_str());
		}

		bool changed = false;
		CPathEx regFilePath(RMXUtils::s2ws(xmlpath.c_str()).c_str());

		const wstring& workspaceDir = regFilePath.GetParentPath();

		for (XMLSize_t xx = 0; xx < nl->getLength(); ++xx)
		{
			DOMElement* element = dynamic_cast<DOMElement*>(nl->item(xx));

			XMLCh* nameNodeAttr = (XMLCh*)element->getAttribute(XMLString::transcode("name"));
			char *tmpNameNode = XMLString::transcode(nameNodeAttr);
			string nameNode(tmpNameNode);
				
			CPathEx cadFilePath(workspaceDir.c_str());
			cadFilePath /= RMXUtils::s2ws(nameNode).c_str();
			if (CPathEx::IsFile(cadFilePath.ToString()))
			{
				bool hasViewRight = CSwUserAuthorization::GetInstance()->CheckUserAuthorization(cadFilePath.ToString(), FileRight::RMX_RIGHT_VIEW);
				if (!_wcsicmp(FileUtils::GetFileExt(RMXUtils::s2ws(nameNode)).c_str(), NXL_FILE_EXT)) {
					//Modify the name value to remove .nxl extension.
					if (hasViewRight)
					{
						string modifiedName = nameNode.substr(0, nameNode.length() - 4);
						element->setAttribute(XMLString::transcode("name"), XMLString::transcode(modifiedName.c_str()));
						LOG_INFO_FMT(L"Removed .nxl from cad_file name attribute: %s", RMXUtils::s2ws(modifiedName).c_str());
						changed = true;
					}
				}
				else
				{
					// bug 62892: Retain .nxl extension for file without view right
					if (!hasViewRight)
					{
						nameNode += ".nxl";
						element->setAttribute(XMLString::transcode("name"), XMLString::transcode(nameNode.c_str()));
						LOG_INFO_FMT(L"Appended .nxl to cad_file name attribute: %s", RMXUtils::s2ws(nameNode).c_str());
						changed = true;
					}

				}
			}
				
			XMLString::release(&tmpNameNode);

		}
		if (changed) {
			WriteXML(xmlDoc, xmlpath);
		}
		else {
			LOG_INFO_FMT(L"XML file already up-to-date: %s", RMXUtils::s2ws(xmlpath).c_str());
		}
	}
	catch (xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		LOG_ERROR("Exception Message is = " << message);
		XMLString::release(&message);
		return false;
	}

	return true;

}

void SwimXMLParser::WriteXML(xercesc::DOMDocument* xmlDoc, const std::string & filePath)
{
	LOG_INFO("Serializing DOM tree back to an XML file = " << filePath.c_str());
	DOMImplementation *implementation = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
	DOMLSSerializer *serializer = ((DOMImplementationLS*)implementation)->createLSSerializer();

	if (serializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
		serializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

	serializer->setNewLine(XMLString::transcode("\r\n"));
	XMLCh *tempFilePath = XMLString::transcode(filePath.c_str());

	XMLFormatTarget *formatTarget = new LocalFileFormatTarget(tempFilePath);

	DOMLSOutput *output = ((DOMImplementationLS*)implementation)->createLSOutput();

	output->setByteStream(formatTarget);
	
	// Write the serialized output to the destination.
	serializer->write(xmlDoc, output);

	// Cleanup.
	serializer->release();
	XMLString::release(&tempFilePath);
	delete formatTarget;
	output->release();
}
