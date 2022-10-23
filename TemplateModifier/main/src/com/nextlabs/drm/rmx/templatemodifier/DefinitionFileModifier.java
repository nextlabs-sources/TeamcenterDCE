package com.nextlabs.drm.rmx.templatemodifier;

/*
 * Created on November 18, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.templatemodifier.configuration.ModifierConstants;

public class DefinitionFileModifier extends DocumentModifier {
	
	private static final Logger logger = LogManager.getLogger("TMLOGGER");
	private String typeName;
	
	private String mainVerFilename;
	private File mainVerFile;

	public DefinitionFileModifier(String typeName) 
			throws NullPointerException, SecurityException {
		super(ModifierConstants.DEFINITION_FILE_PATH, typeName, ModifierConstants.DEFINITION_FILE_EXT);
		
		this.typeName = typeName;
		
		// setting up input
		mainVerFilename = typeName.toLowerCase() + "." + ModifierConstants.DEFINITION_FILE_EXT;
		mainVerFile = new File(super.getTcDataPath() + File.separator + 
				ModifierConstants.DEFINITION_FILE_PATH + File.separator + mainVerFilename);
	}
	
	private boolean isInReferencesBody(boolean isInReferencesBody, String line) {
		if (line.startsWith("References\t{")) {
			logger.debug("  Entering References body");
			return true;
		} else if (isInReferencesBody && line.startsWith("}")) {
			logger.debug("  Exit References body");
			return false;
		}
		
		return isInReferencesBody;
	}
	
	private void outputToFile(StringBuilder sbOutput, String methodName) throws IOException {
		logger.debug("  Writing out modified version of definition file for {}", typeName);
		try (BufferedWriter bw = new BufferedWriter(new FileWriter(mainVerFile, false))) {
			bw.write(sbOutput.toString());
		} catch (IOException e) {
			logger.error("DefinitionFileModifier.{}() caught exception: {}", methodName, e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
	}
		
	public void modifyCurrentVersion(List<String> namedReferences) throws IOException {
		if (!mainVerFile.exists()) {
			throw new FileNotFoundException("File not found for " + mainVerFile.getAbsolutePath());
		}
		
		// setting up output
		// can write to temp file or use String builder
		StringBuilder sbOutput = new StringBuilder();
		
		// reading from input and process the data
		logger.debug("  Reading current version of definition file for {}", typeName);
		try (BufferedReader br = new BufferedReader(new FileReader(mainVerFile))) {
			boolean isInReferencesBody = false;
			
			// read from definition file
			String line = br.readLine();

			while (line != null) {
				String newLine = null;
				int lastPos = 0; // record the index position
				
				isInReferencesBody = isInReferencesBody(isInReferencesBody, line);
				
				if (isInReferencesBody) {
					// within References	{ ... } body
					String[] data = line.split("\t");
					String dataBlkToBeReplaced = null;
					String dataBlkForReplace = null;
					boolean modifyNextBlock = false;
					
					for (String dataBlock : data) {
						if (modifyNextBlock) {
							if (!dataBlock.endsWith("." + ModifierConstants.NXL_EXT) && 
									dataBlock.startsWith("*.")) {
								dataBlkToBeReplaced = dataBlock;
								dataBlkForReplace = dataBlock + "." + ModifierConstants.NXL_EXT;
								break;
							}
						}
						
						// remove double quotes
						// check whether the read datablock is in the specified named reference list
						// if yes, we need to modify the data in next block
						if (!modifyNextBlock && namedReferences.contains(dataBlock.replaceAll("\"", ""))) {
							modifyNextBlock = true;
						}
					}
					
					if (dataBlkToBeReplaced != null && dataBlkForReplace != null) {
						newLine = new String(line);
						
						logger.info("  Replacing {} with {}", dataBlkToBeReplaced, dataBlkForReplace);
						newLine = newLine.replace(dataBlkToBeReplaced, dataBlkForReplace);
						
						sbOutput.append(line + System.lineSeparator());
						lastPos = sbOutput.length();
						sbOutput.append(newLine + System.lineSeparator());
					} else {
						sbOutput.append(line + System.lineSeparator());
					}
				} else {
					// not within References	{ ... } body
					sbOutput.append(line + System.lineSeparator());
				}
				
				line = br.readLine();
				// remove added newline if duplicate with next line
				if (newLine != null && newLine.equals(line)) {
					logger.info("  Duplicate line is found {}, the added new line will be removed", newLine);
					sbOutput = sbOutput.replace(lastPos, sbOutput.length(), "");
				}
			} // end of line loop
		} catch (IOException e) {
			logger.error("DefinitionFileModifier.modifyCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
		
		// writing to output
		outputToFile(sbOutput, "modifyCurrentVersion");
	}
	
	private void modifyText(boolean undo, List<String> namedReferences) throws IOException {
		if (!mainVerFile.exists()) {
			throw new FileNotFoundException("File not found for " + mainVerFile.getAbsolutePath());
		}
		
		// setting up output
		// can write to temp file or use String builder
		StringBuilder sbOutput = new StringBuilder();
		
		// reading from input and process the data
		logger.debug("  Reading current version of definition file for {}", typeName);
		try (BufferedReader br = new BufferedReader(new FileReader(mainVerFile))) {
			boolean isInReferencesBody = false;
			
			// read from definition file
			String line = br.readLine();
			String line_nxl_nr = null;

			while (line != null) {
				if (line.startsWith("References\t{")) {
					logger.debug("  Entering References body");
					isInReferencesBody = true;
				} else if (isInReferencesBody && line.startsWith("}")) {
					if (line_nxl_nr != null)
						sbOutput.append(line_nxl_nr + System.lineSeparator());
					
					logger.debug("  Exit References body");
					isInReferencesBody = false;
				}
				
				if (isInReferencesBody) {
					// within References	{ ... } body
					if (!undo) {
						if (namedReferences.contains("Text") && 
								line.matches(ModifierConstants.ORG_TEXT_DEF_REGEX)) {
							line = line.replace("TEXT", "BINARY");
							
							logger.info("  Replacing TEXT with BINARY");
							
							sbOutput.append(line + System.lineSeparator());
						} else if (line.contains(ModifierConstants.NXL_NR)) {
							line_nxl_nr = line;
						} else {
							sbOutput.append(line + System.lineSeparator());
						}
					} else {
						if (line.matches(ModifierConstants.MOD_TEXT_DEF_REGEX)) {
							line = line.replace("BINARY", "TEXT");
							
							logger.info("  Replacing BINARY with TEXT");
							
							sbOutput.append(line + System.lineSeparator());
						}  else if (line.contains(ModifierConstants.NXL_NR)) {
							line_nxl_nr = line;
						} else {
							sbOutput.append(line + System.lineSeparator());
						}
					}
				} else {
					// not within References	{ ... } body
					sbOutput.append(line + System.lineSeparator());
				}
				
				line = br.readLine();
			} // end of line loop
		} catch (IOException e) {
			logger.error("DefinitionFileModifier.modifyCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
		
		// writing to output
		if (!undo)
			outputToFile(sbOutput, "modifyCurrentVersion_Text");
		else
			outputToFile(sbOutput, "undoModification_Text");
	}
	
	public void modifyCurrentVersion_Text(List<String> namedReferences) throws IOException {
		modifyText(false, namedReferences);
	}
	
	public void undoModification() throws IOException {
		if (!mainVerFile.exists()) {
			throw new FileNotFoundException("File not found for " + mainVerFile.getAbsolutePath());
		}
		
		// setting up output
		// can write to temp file or use String builder
		StringBuilder sbOutput = new StringBuilder();
		
		// reading from input and process the data
		logger.debug("  Reading current version of definition file for {}", typeName);
		try (BufferedReader br = new BufferedReader(new FileReader(mainVerFile))) {
			boolean isInReferencesBody = false;
			
			// read from definition file
			String line = br.readLine();

			while (line != null) {
				isInReferencesBody = isInReferencesBody(isInReferencesBody, line);
				
				if (isInReferencesBody) {
					// within References	{ ... } body
					String[] data = line.split("\t");
					boolean removeLine = false;
					
					for (String dataBlock : data) {
						if (dataBlock.matches(ModifierConstants.MOD_LINE_REGEX)) {
							logger.info("  Removing " + line);
							
							removeLine = true;
							break;
						}
					}
					
					if (!removeLine) {
						sbOutput.append(line + System.lineSeparator());
					}
				} else {
					// not within References	{ ... } body
					sbOutput.append(line + System.lineSeparator());
				}
				
				line = br.readLine();
			} // end of line loop
		} catch (IOException e) {
			logger.error("DefinitionFileModifier.modifyCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
		
		// writing to output
		outputToFile(sbOutput, "undoModification");
	}
	
	public void undoModification_Text() throws IOException {
		modifyText(true, null);
	}
	
}
