package com.nextlabs.drm.rmx.templatemodifier;

/*
 * Created on November 18, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.IOException;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.xml.sax.SAXException;

import com.nextlabs.drm.rmx.templatemodifier.configuration.ModifierConstants;

public class TemplateModifierApp {
	
	private static final Logger logger = LogManager.getLogger("TMLOGGER");
	
	public TemplateModifierApp() {
		
	}
	
	public static void main (String[] args) {
		TemplateModifierApp templateModifierApp = new TemplateModifierApp();
		
		String action = null;
		String template = null;
		String inputfile = null;
		
		if (args.length != 6) {
			templateModifierApp.printUsage();
			System.exit(0);
		}
		
		try {
			for (int i = 0; i < args.length; i++) {
				if (args[i].equals("-a")) {
					String argAction = args[i + 1].trim();
					
					if (argAction.equals(ModifierConstants.ACT_CUSTOMIZE)) {
						action = argAction;
					} else if (argAction.equals(ModifierConstants.ACT_UNDO)) {
						action = argAction;
					} else if (argAction.equals(ModifierConstants.ACT_RESTORE)) {
						action = argAction;
					}
				} else if (args[i].equals("-t")) {
					template = args[i + 1].trim();
				} else if (args[i].equals("-i")) {
					inputfile = args[i + 1].trim();
				}
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			logger.error("Error: invalid input arguments");
			System.exit(1);
		}
		
		if (action == null || template == null || inputfile == null) {
			templateModifierApp.printUsage();
			System.exit(0);
		} else {
			Map<String, List<String>> mapDataset = null;
			
			try {
				InputParser inputParser = new InputParser(inputfile);
				mapDataset = inputParser.parse();
			} catch (ParserConfigurationException | SAXException | IOException e) {
				logger.error(e.getMessage());
				System.exit(1);
			}
			
			if (mapDataset == null) {
				logger.error("Error: there is no dataset to customize");
				System.exit(1);
			}
			
			switch (action) {
				case ModifierConstants.ACT_CUSTOMIZE:
					templateModifierApp.modify(template, mapDataset);
					break;
				case ModifierConstants.ACT_UNDO:
					templateModifierApp.undo(template, mapDataset);
					break;
				case ModifierConstants.ACT_RESTORE:
					templateModifierApp.restore(template, mapDataset);
			}
		}
	}
		
	private void printUsage() {
		System.out.println("Usage: templatemod -a action -t template -i inputfile");
		System.out.println("");
		System.out.println("Options:");
		System.out.println("\t-a\t\taction: customize/undo/restore. undo: remove customize changes. restore: restore from backup copy.");
		System.out.println("\t-t\t\ttemplate: template to be customized, i.e. foundation");
		System.out.println("\t-i\t\tinputfile: input file that specifies what to be/have been customized");
	}
	
	private void modify(String templateName, Map<String, List<String>> mapDataset) {
		try {
			// modify template
			logger.info("Modifying template ...");
			modifyTemplate(templateName, mapDataset);
			logger.info("Modify template is complete");
			
			// modify baseline
			logger.info("Modifying baseline ...");
			modifyBaseline(templateName, mapDataset);
			logger.info("Modify baseline is complete");
			
			// modify definition file
			Iterator<Entry<String, List<String>>> it = mapDataset.entrySet().iterator();
			while (it.hasNext()) {
				Map.Entry<String, List<String>> pair = (Map.Entry<String, List<String>>) it.next();
				
				logger.info("Modifying definition file for {} ...", pair.getKey());
				try {
					modifyDefinitionFile(pair.getKey(), pair.getValue());
				} catch (IOException ioe) {
					logger.error("Error is caught while modifying definition file {}", ioe.getMessage());
				}
				logger.info("Modify definition file for {} is complete", pair.getKey());
			}
		} catch (TransformerException | ParserConfigurationException | SAXException | 
				IOException | NullPointerException | SecurityException e) {
			logger.error("Error is caught while processing the files " + e.getMessage());
		}
	}
	
	private void modifyTemplate(String templateName, Map<String, List<String>> mapDataset) throws 
		TransformerException, ParserConfigurationException, SAXException, 
		IOException, NullPointerException, SecurityException {
		
		TemplateModifier tmpModifier = new TemplateModifier(ModifierConstants.TEMPLATE_FILE_PATH, templateName + "_template");
		
		tmpModifier.backupCurrentVersion();
		tmpModifier.modifyCurrentVersion(mapDataset);
	}
	
	private void modifyBaseline(String templateName, Map<String, List<String>> mapDataset) throws 
		TransformerException, ParserConfigurationException, SAXException, 
		IOException, NullPointerException, SecurityException {
		
		TemplateModifier tmpModifier = new TemplateModifier(ModifierConstants.BASELINE_FILE_PATH, templateName + "_tcbaseline");
		
		tmpModifier.backupCurrentVersion();
		tmpModifier.modifyCurrentVersion(mapDataset);
	}
	
	private void modifyDefinitionFile(String typeName, List<String> namedRef) 
			throws IOException {
		DefinitionFileModifier dfModifier = new DefinitionFileModifier(typeName);
		
		dfModifier.backupCurrentVersion();
		if (typeName.equals("Text") && namedRef.contains("Text")) {
			dfModifier.modifyCurrentVersion_Text(namedRef);
		} else {
			dfModifier.modifyCurrentVersion(namedRef);
		}
	}
	
	private void restore(String templateName, Map<String, List<String>> mapDataset) {
		try {
			// modify template
			logger.info("Restoring template ...");
			restoreTemplate(templateName);
			logger.info("Restore template is complete");
			
			// modify baseline
			logger.info("Restoring baseline ...");
			restoreBaseline(templateName);
			logger.info("Restore baseline is complete");
			
			// modify definition file
			Iterator<Entry<String, List<String>>> it = mapDataset.entrySet().iterator();
			while (it.hasNext()) {
				Map.Entry<String, List<String>> pair = (Map.Entry<String, List<String>>) it.next();
				
				logger.info("Restoring definition file for " + pair.getKey() + " ...");
				try {
					restoreDefinitionFile(pair.getKey());
				} catch (IOException ioe) {
					logger.error("Error is caught while restoring definition file " + ioe.getMessage());
				}
				logger.info("Restore definition file for " + pair.getKey() + " is complete");
			}
		} catch (IOException | NullPointerException | SecurityException e) {
			logger.error("Error is caught while processing the files " + e.getMessage());
		}
	}
	
	private void restoreTemplate(String templateName) 
			throws IOException, NullPointerException, SecurityException {
		TemplateModifier tmpModifier = new TemplateModifier(ModifierConstants.TEMPLATE_FILE_PATH, templateName + "_template");
		
		tmpModifier.restoreFromLastBackup();
	}
	
	private void restoreBaseline(String templateName) 
			throws IOException, NullPointerException, SecurityException {
		TemplateModifier tmpModifier = new TemplateModifier(ModifierConstants.BASELINE_FILE_PATH, templateName + "_tcbaseline");
		
		tmpModifier.restoreFromLastBackup();
	}
	
	private void restoreDefinitionFile(String typeName) 
			throws IOException {
		DefinitionFileModifier dfModifier = new DefinitionFileModifier(typeName);
		
		dfModifier.restoreFromLastBackup();
	}
	
	private void undo(String templateName, Map<String, List<String>> mapDataset) {
		try {
			// modify template
			logger.info("Undoing template ...");
			undoTemplate(templateName);
			logger.info("Undo template is complete");
			
			// modify baseline
			logger.info("Undoing baseline ...");
			undoBaseline(templateName);
			logger.info("Undo baseline is complete");
			
			// modify definition file
			Iterator<Entry<String, List<String>>> it = mapDataset.entrySet().iterator();
			while (it.hasNext()) {
				Map.Entry<String, List<String>> pair = (Map.Entry<String, List<String>>) it.next();
				
				logger.info("Undoing definition file for {} ...", pair.getKey());
				try {
					undoDefinitionFile(pair.getKey());
				} catch (IOException ioe) {
					logger.error("Error is caught while undoing definition file {}", ioe.getMessage());
				}
				logger.info("Undo definition file for {} is complete", pair.getKey());
			}
		} catch (TransformerException | ParserConfigurationException | SAXException | 
				IOException | NullPointerException | SecurityException e) {
			logger.error("Error is caught while processing the files {}", e.getMessage());
		}
	}
	
	private void undoTemplate(String templateName) throws 
		TransformerException, ParserConfigurationException, SAXException, 
		IOException, NullPointerException, SecurityException {
		TemplateModifier tmpModifier = new TemplateModifier(ModifierConstants.TEMPLATE_FILE_PATH, templateName + "_template");
		
		tmpModifier.backupCurrentVersion();
		tmpModifier.undoModification();
	}
	
	private void undoBaseline(String templateName) throws 
		TransformerException, ParserConfigurationException, SAXException, 
		IOException, NullPointerException, SecurityException {
		TemplateModifier tmpModifier = new TemplateModifier(ModifierConstants.BASELINE_FILE_PATH, templateName + "_tcbaseline");
		
		tmpModifier.backupCurrentVersion();
		tmpModifier.undoModification();
	}
	
	private void undoDefinitionFile(String typeName) 
			throws IOException {
		DefinitionFileModifier dfModifier = new DefinitionFileModifier(typeName);
		
		dfModifier.backupCurrentVersion();
		if (typeName.equals("Text")) {
			dfModifier.undoModification_Text();
		} else {
			dfModifier.undoModification();
		}
	}

}
