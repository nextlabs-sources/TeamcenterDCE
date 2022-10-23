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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.nextlabs.drm.rmx.templatemodifier.configuration.ModifierConstants;

public class InputParser {
	
	private static final Logger logger = LogManager.getLogger("TMLOGGER");
	private String filePath;
	
	public InputParser(String filePath) {
		this.filePath = filePath;
	}
	
	public Map<String, List<String>> parse() throws ParserConfigurationException, 
		SAXException, IOException {
		
		Map<String, List<String>> mapDataset = new HashMap<>();
		File inputFile = new File(filePath);
		
		if (!inputFile.exists()) {
			throw new FileNotFoundException("File not found for " + inputFile.getAbsolutePath());
		}
		
		DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
		dbFactory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
		
		logger.debug("  Parsing input file ...");
		try {
			DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(inputFile);
			doc.getDocumentElement().normalize();
			
			NodeList nlstDataset = doc.getElementsByTagName(ModifierConstants.TC_DATASET);
			
			for (int i = 0; i < nlstDataset.getLength(); i++) {
				Node nDataset = nlstDataset.item(i);
				
				if (nDataset.getNodeType() == Node.ELEMENT_NODE) {
					Element eDataset = (Element) nDataset;
					String typeName = eDataset.getAttribute(ModifierConstants.ATTR_TYPE_NAME);
					
					NodeList nlstDatasetRef = eDataset.getElementsByTagName(
							ModifierConstants.TC_DATASET_REFERENCE);
					List<String> lstRefName = new ArrayList<>();
						
					for (int j = 0; j < nlstDatasetRef.getLength(); j++) {
						Node nDatasetRef = nlstDatasetRef.item(j);
						
						if (nDatasetRef.getNodeType() == Node.ELEMENT_NODE) {
							Element eDatasetRef = (Element) nDatasetRef;
							String refName = eDatasetRef.getAttribute(ModifierConstants.ATTR_NAME);
							
							logger.debug("  Adding reference {} to type {}", refName, typeName);
							lstRefName.add(refName);
						}
					} // end of nlstDatasetRef loop
					
					if (!lstRefName.isEmpty())
						mapDataset.put(typeName, lstRefName);
				}
			} // end of nlstDataset loop
		} catch (ParserConfigurationException | SAXException | IOException e) {
			logger.error("InputParser.parse() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
		
		return mapDataset;
	}
	
}
