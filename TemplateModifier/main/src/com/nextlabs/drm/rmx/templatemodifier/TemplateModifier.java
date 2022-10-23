package com.nextlabs.drm.rmx.templatemodifier;

/*
 * Created on November 18, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.w3c.dom.Comment;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.nextlabs.drm.rmx.templatemodifier.configuration.ModifierConstants;

public class TemplateModifier extends DocumentModifier {
	
	private static final Logger logger = LogManager.getLogger("TMLOGGER");
	private String templateName;
	
	private String mainVerFilename;
	private File mainVerFile;
	
	public TemplateModifier(String filepath, String templateName) 
			throws NullPointerException, SecurityException {
		super(filepath, templateName, ModifierConstants.TEMPLATE_FILE_EXT);
		this.templateName = templateName;
		
		// input
		mainVerFilename = templateName.toLowerCase() + "." + ModifierConstants.TEMPLATE_FILE_EXT;
		mainVerFile = new File(super.getTcDataPath() + File.separator + filepath + File.separator + mainVerFilename);
	}
	
	public void modifyCurrentVersion(Map<String, List<String>> mapDataset) 
			throws IOException, TransformerException, ParserConfigurationException, SAXException {
		// reading from input and process the data
		logger.debug("  Reading current version of template for {}", templateName);
		try {
			// reading from input and process the data
			Document doc = initDocument(mainVerFile);
			
			NodeList nlstDataset = doc.getElementsByTagName(ModifierConstants.TC_DATASET);
			
			// TcDataset loop
			for (int i = 0; i < nlstDataset.getLength(); i++) {
				Node nDataset = nlstDataset.item(i);
				
				if (nDataset.getNodeType() == Node.ELEMENT_NODE) {
					Element eDataset = (Element) nDataset;
					String typeName = eDataset.getAttribute(ModifierConstants.ATTR_TYPE_NAME);
					
					if (mapDataset.containsKey(typeName)) {
						logger.info("  Modifying {}", eDataset.getAttribute(ModifierConstants.ATTR_TYPE_NAME));
						NodeList nlstDatasetRef = eDataset.getElementsByTagName(
								ModifierConstants.TC_DATASET_REFERENCE);
						List<String> lstRefName = mapDataset.get(typeName);
						
						// TcDatasetReference loop
						for (int j = 0; j < nlstDatasetRef.getLength(); j++) {
							Node nDatasetRef = nlstDatasetRef.item(j);
							
							if (nDatasetRef.getNodeType() == Node.ELEMENT_NODE) {
								Element eDatasetRef = (Element) nDatasetRef;
								String refName = eDatasetRef.getAttribute(ModifierConstants.ATTR_NAME);
								
								if (lstRefName.contains(refName)) {
									NodeList nlstDatasetRefInfo = eDatasetRef.getElementsByTagName(
											ModifierConstants.TC_DATASET_REFERENCE_INFO);
									List<Element> lstNewElement = new ArrayList<Element>();
									
									// handle Text case which is modifying instead of adding new line
									if (typeName.equals("Text") && refName.equals("Text")) {
										// TcDatasetReferenceInfo loop
										for (int k = 0; k < nlstDatasetRefInfo.getLength(); k++) {
											Node nDatasetRefInfo = nlstDatasetRefInfo.item(k);
											
											if (nDatasetRefInfo.getNodeType() == Node.ELEMENT_NODE) {
												Element eDatasetRefInfo = (Element) nDatasetRefInfo;
												String orgFormat = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_FORMAT);
												String orgTemplate = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_TEMPLATE);
												
												if (orgFormat.equals("TEXT") && orgTemplate.equals("*")) {
													eDatasetRefInfo.setAttribute(ModifierConstants.ATTR_FORMAT, "BINARY");
													
													logger.info("    Setting Text element {}={}", ModifierConstants.ATTR_FORMAT,
															ModifierConstants.NXL_FORMAT);
												}
											}
										} // end of TcDatasetReferenceInfo loop
									} else {
										// TcDatasetReferenceInfo loop
										for (int k = 0; k < nlstDatasetRefInfo.getLength(); k++) {
											Node nDatasetRefInfo = nlstDatasetRefInfo.item(k);
											
											if (nDatasetRefInfo.getNodeType() == Node.ELEMENT_NODE) {
												Element eDatasetRefInfo = (Element) nDatasetRefInfo;
												String orgTemplate = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_TEMPLATE);
												
												if (orgTemplate.lastIndexOf("." + ModifierConstants.NXL_EXT) < 0 && orgTemplate.indexOf("*.") >= 0) {
													Element newElement = doc.createElement(ModifierConstants.TC_DATASET_REFERENCE_INFO);
													newElement.setAttribute(ModifierConstants.ATTR_FORMAT, ModifierConstants.NXL_FORMAT);
													newElement.setAttribute(ModifierConstants.ATTR_TEMPLATE, orgTemplate + "." + ModifierConstants.NXL_EXT);
													
													logger.info("    Adding new element {}={}, {}={}.{}", ModifierConstants.ATTR_FORMAT, 
															ModifierConstants.NXL_FORMAT, ModifierConstants.ATTR_TEMPLATE, 
															orgTemplate, ModifierConstants.NXL_EXT);
													
													lstNewElement.add(newElement);
												}
											}
										} // end of TcDatasetReferenceInfo loop
																		
										if (lstNewElement.size() > 0) {
											boolean isNewElementAdded = false;
											Comment comment1 = doc.createComment(ModifierConstants.NXL_COMMENT_START);
											eDatasetRef.appendChild(comment1);
											
											for (Element newElement: lstNewElement) {
												if (!isElementDuplicate(nlstDatasetRefInfo, newElement)) {
													eDatasetRef.appendChild(newElement);
													isNewElementAdded = true;
												}
											}
											
											Comment comment2 = doc.createComment(ModifierConstants.NXL_COMMENT_END);
											eDatasetRef.appendChild(comment2);
											
											// remove the comments if there is no new element added
											if (!isNewElementAdded) {
												eDatasetRef.removeChild(comment1);
												eDatasetRef.removeChild(comment2);
											}
										}
									}
								}
							}
						} // end of TcDatasetReference loop
					}
				}
			} // end of TcDataset loop
			
			// writing to output
			logger.debug("  Writing out modified version of template file for {}", templateName);
			outputDocument(doc, mainVerFile);
		} catch (ParserConfigurationException | SAXException | IOException e) {
			logger.error("TemplateModifier.modifyCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
		
	}
	
	private Document initDocument(File mainVerFile) throws IOException, ParserConfigurationException, SAXException {
		if (!mainVerFile.exists()) {
			throw new FileNotFoundException("File not found for " + mainVerFile.getAbsolutePath());
		}
		
		DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
		dbFactory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
		
		DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
		Document doc = dBuilder.parse(mainVerFile);
		doc.setXmlStandalone(true);
		
		return doc;
	}
	
	private void outputDocument(Document doc, File mainVerFile) throws TransformerException {
		DOMSource source = new DOMSource(doc);
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        transformerFactory.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
        transformerFactory.setAttribute(XMLConstants.ACCESS_EXTERNAL_STYLESHEET, "");
        Transformer transformer;
        
		try {
			transformer = transformerFactory.newTransformer();
			StreamResult result = new StreamResult(mainVerFile.getAbsolutePath());
			
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");
			transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
			transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");
			
			transformer.transform(source, result);
		} catch (TransformerException e) {
			logger.error("TemplateModifier.modifyCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
	}
	
	private boolean isElementDuplicate(NodeList nlstDatasetRefInfo, Element newElement) {
		for (int i = 0; i < nlstDatasetRefInfo.getLength(); i++) {
			Node nDatasetRefInfo = nlstDatasetRefInfo.item(i);
			
			if (nDatasetRefInfo.getNodeType() == Node.ELEMENT_NODE) {
				Element eDatasetRefInfo = (Element) nDatasetRefInfo;
				
				String format = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_FORMAT);
				String template = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_TEMPLATE);
				
				String newFormat = newElement.getAttribute(ModifierConstants.ATTR_FORMAT);
				String newTemplate = newElement.getAttribute(ModifierConstants.ATTR_TEMPLATE);
				
				if (format.equals(newFormat) && template.equals(newTemplate)) {
					logger.info("    Duplicate element is found {}={}, {}={}, the duplicated new element will be removed", 
							ModifierConstants.ATTR_FORMAT, newFormat, ModifierConstants.ATTR_TEMPLATE, newTemplate);
					
					return true;
				}
			}
		}
		
		return false;
	}
	
	public void undoModification() 
			throws IOException, TransformerException, ParserConfigurationException, 
			SAXException {
		// reading from input and process the data
		logger.debug("  Reading current version of template for {}", templateName);
		try {
			// reading from input and process the data
			Document doc = initDocument(mainVerFile);
			
			NodeList nlstDataset = doc.getElementsByTagName(ModifierConstants.TC_DATASET);
			
			// TcDataset loop
			for (int i = 0; i < nlstDataset.getLength(); i++) {
				Node nDataset = nlstDataset.item(i);
				
				if (nDataset.getNodeType() == Node.ELEMENT_NODE) {
					Element eDataset = (Element) nDataset;
					String typeName = eDataset.getAttribute(ModifierConstants.ATTR_TYPE_NAME);
					
					logger.info("  Checking {}", eDataset.getAttribute(ModifierConstants.ATTR_TYPE_NAME));
					NodeList nlstDatasetRef = eDataset.getElementsByTagName(
							ModifierConstants.TC_DATASET_REFERENCE);
					
					// TcDatasetReference loop
					for (int j = 0; j < nlstDatasetRef.getLength(); j++) {
						Node nDatasetRef = nlstDatasetRef.item(j);
						
						if (nDatasetRef.getNodeType() == Node.ELEMENT_NODE) {
							Element eDatasetRef = (Element) nDatasetRef;
							String refName = eDatasetRef.getAttribute(ModifierConstants.ATTR_NAME);
							
							// NXL comment node loop
							NodeList nlstEDatasetRef = eDatasetRef.getChildNodes();
							for (int k = 0; k < nlstEDatasetRef.getLength(); ) {
								Node nEDatasetRef = nlstEDatasetRef.item(k);
								
								// comment node
								if (nEDatasetRef.getNodeType() == Element.COMMENT_NODE) {
									Comment cNode = (Comment) nEDatasetRef;
									
									if (cNode.getData().equals(ModifierConstants.NXL_COMMENT_START) ||
											cNode.getData().equals(ModifierConstants.NXL_COMMENT_END)) {
										logger.info("    Removing Comment node: data={}", cNode.getData());
										
										Node next = cNode.getNextSibling();
										if (next != null && 
												next.getNodeType() == Node.TEXT_NODE && 
														next.getNodeValue().trim().length() == 0) {
											eDatasetRef.removeChild(next);
										}
										
										eDatasetRef.removeChild(cNode);
									} else {
										k++;
									}
								} else if (nEDatasetRef.getNodeType() == Node.ELEMENT_NODE && 
										nEDatasetRef.getNodeName().equals(ModifierConstants.TC_DATASET_REFERENCE_INFO)) {
									
									Element eDatasetRefInfo = (Element) nEDatasetRef;
									String orgFormat = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_FORMAT);
									String orgTemplate = eDatasetRefInfo.getAttribute(ModifierConstants.ATTR_TEMPLATE);
									
									// handle Text case which is modifying instead of adding new line
									if (typeName.equals("Text") && refName.equals("Text")) {
										if (orgFormat.equals("BINARY") && orgTemplate.equals("*")) {
											eDatasetRefInfo.setAttribute(ModifierConstants.ATTR_FORMAT, "TEXT");
											
											logger.info("    Setting Text element {}={}",  ModifierConstants.ATTR_FORMAT, "TEXT");
										}
										
										k++;
									} else {
										if (orgTemplate.lastIndexOf("." + ModifierConstants.NXL_EXT) > 0 && orgTemplate.indexOf("*.") >= 0) {
											logger.info("    Removing TCDataReferenceInfo node: format={}, template={}", orgFormat, orgTemplate);
											
											Node next = eDatasetRefInfo.getNextSibling();
											if (next != null && 
													next.getNodeType() == Node.TEXT_NODE && 
															next.getNodeValue().trim().length() == 0) {
												eDatasetRef.removeChild(next);
											}
											
											eDatasetRef.removeChild(eDatasetRefInfo);
										} else {
											k++;
										}
									}
								} else {
									k++;
								}
							}
						}
					} // end of TcDatasetReference loop
				}
			} // end of TcDataset loop
			
			// writing to output
			logger.debug("  Writing out modified version of template file for {}", templateName);
			outputDocument(doc, mainVerFile);
		} catch (ParserConfigurationException | SAXException | IOException e) {
			logger.error("TemplateModifier.modifyCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
	}

}
