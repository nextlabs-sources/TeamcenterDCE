package com.nextlabs.drm.rmx.translator;

/**
 * Created on October 6, 2015
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TranslatorApp {
	
	private static final String ARGS_FILE = "argsFile";	// parameter name of arguments file
	
	private static final int EXIT_CODE_NORMAL = 0;
	private static final int EXIT_CODE_ERROR = 1;	// partial error will also exit with error code, which causes admin console to show terminal
	
	// fix bug 38138
	// special character that can't be used in filename (Windows platform)
	private static final String SRC_LIST_DELIMITER = ":";
	
	private Logger logger;
	
	public TranslatorApp() {
		logger = LoggerFactory.getLogger(Translator.class);
	}
	
	public int processRequest(String[] args) {
		boolean isOverallSuccess = true;
		Translator translator = null;
		
		try {
			translator = new Translator();
		} catch (Exception ex) {
			logger.error(ex.getMessage(), ex);
			return EXIT_CODE_ERROR;
		}
		
		logger.debug("args count: {}", args.length);
		if (args.length < 4) {
			logger.error("invalid arguments");
			return EXIT_CODE_ERROR;
		}
		
		String inputPath = args[0];
		String filename = args[1];
		String outputPath = args[2];
		String nxlAction = args[3];
		
		logger.debug("input file path: {}", inputPath);
		logger.debug("input file name: {}", filename);
		logger.debug("output file path: {}", outputPath);
		logger.debug("working directory: {}", inputPath);
		logger.debug("nxl action: {}", nxlAction);
		
		// loading tags
		Map<String, String[]> tagMap = new HashMap<>();
		
		if (args.length > 4) {
			logger.debug("external tags file: {}", args[4]);
			
			int tagStartIndex = 4;
			String[] kvp = args[4].split("=");
			
			if (kvp.length >= 2 && kvp[0].equals(ARGS_FILE)) {
				String tagFileName = kvp[1];
				
				if (tagFileName != null && tagFileName.length() > 0) {
					logger.debug("loading tags from file: {} ...", tagFileName);
					File tagFile = new File(inputPath + File.separator + tagFileName);
					
					if (tagFile.exists()) {
						tagMap = translator.loadTagsFromFile(tagFile);
					} else {
						logger.debug("cannot find tags file {}", tagFile.getAbsolutePath());
					}
				} else {
					logger.error("{} is not specified!", ARGS_FILE);
				}
				
				tagStartIndex += 1;
			}
			
			if (args.length > tagStartIndex) {
				logger.debug("loading {} tags from command line args...", (args.length - tagStartIndex));
				int argsCount = 0;
				
				for (int i = tagStartIndex; i < args.length; i++) {
					String[] keyValue = args[i].trim().split("=");
					logger.debug("   args[{}] --> {}", i, args[i]);
					
					if (keyValue.length >= 2) {
						String[] values = keyValue[1].split(",");
						for (int j = 0; j < values.length; j++) {
							values[j] = values[j].trim();
						}
						
						tagMap.put(keyValue[0], values);
						argsCount++;
					}
				}
				
				logger.debug("loaded {} tags from command line args", argsCount);
			}
		}
		
		// read the file info to be processed from input file
		List<String> lstFileInfo = new ArrayList<>();
		List<String> lstFileProtected = new ArrayList<>();
		try {
			lstFileInfo = readFromInfoFile(inputPath + File.separator + filename);
		} catch (IOException e) {
			logger.error(e.getMessage(), e);
			
			return EXIT_CODE_ERROR;
		}
		
		
		logger.debug("found {} files to be secured in file {}", lstFileInfo.size(), filename);
		
		for (String fileInfoLine : lstFileInfo) {
			// 0 staging filename, 1 named ref, 2 iman original filename, 3 iman filename
			String[] fileInfos = fileInfoLine.split(SRC_LIST_DELIMITER);
			String filenameToSecure = fileInfos[0];
			
			File file = new File(inputPath + File.separator + filenameToSecure);
			
			if (!file.exists() || file.length() <= 0) {
				logger.debug("file {} doesn't exist or is empty!", file.getAbsolutePath());
				isOverallSuccess = false;
				continue;
			}
			
			logger.debug("processing {} ...", file.getAbsolutePath());

			try {
				if (translator.translate(
						nxlAction, inputPath, outputPath, filenameToSecure, tagMap) == Translator.SUCCESS_CODE) {
					// only add the protected fileinfo to the list
					lstFileProtected.add(fileInfoLine);
				}
			} catch (Exception ex) {
				logger.error("caught exception: {} while processing {}", ex.getMessage(), filenameToSecure, ex);
				
				if (removeErrorNXLFile(outputPath, filenameToSecure)) {
					logger.debug("removed the NXL file from result folder.");
				}
				
				isOverallSuccess = false;
			}
		}
		
		try {
			logger.debug("updating the info file...");
			// if overall success is not true, write the protected fileinfo to the filelist.txt
			writeToInfoFile(lstFileProtected, inputPath + File.separator + filename);
		} catch (IOException e) {
			logger.error("caught exception while updating the info file: {}", e.getMessage(), e);
		}
		
		if (isOverallSuccess) {
			return EXIT_CODE_NORMAL;
		} else {
			return EXIT_CODE_ERROR;
		}
	}
	
	private List<String> readFromInfoFile(String fullPath) throws IOException {
		List<String> lstFileInfo = new ArrayList<>();
		
		try (
				InputStreamReader isr = new InputStreamReader(
						new FileInputStream(fullPath), Translator.CHARSET);
				BufferedReader br = new BufferedReader(isr);) {
			String readLine = br.readLine();
			while (readLine != null) {
				lstFileInfo.add(readLine);
				
				readLine = br.readLine();
			}
		}
		
		return lstFileInfo;
	}
	
	private void writeToInfoFile(List<String> lstFileInfo, String fullPath) throws IOException {
		// backup exiting file
		File file = new File(fullPath);
		File orgFile = new File(fullPath + ".org");
		
		if (!file.renameTo(orgFile))
			logger.error("failed to rename {} to {}.org", fullPath, fullPath);
		
		try (
				OutputStreamWriter osw = new OutputStreamWriter(
						new FileOutputStream(fullPath), Translator.CHARSET);
				BufferedWriter bw = new BufferedWriter(osw);) {			
			for (String fileInfo : lstFileInfo) {
				bw.write(fileInfo);
				bw.newLine();
			}
			
			bw.flush();
		}
	}
	
	private boolean removeErrorNXLFile(String path, String filename) {
		String fullPath = path + File.separator + filename;
		
		if (filename.toLowerCase().lastIndexOf(Translator.EXT_NXL_FILE) < 0 ) {
			fullPath += Translator.EXT_NXL_FILE;
		}
		
		File file = new File(fullPath);
		if (file.exists()) {
			try {
				logger.debug("removing {} caused the NXL file can be corrupted due to failure while applying the protection.", 
						fullPath);
				return file.delete();
			} catch (SecurityException se) {
				return false;
			}
		} else {
			return false;
		}
	}

	public static void main(String[] args) {		
		TranslatorApp translatorApp = new TranslatorApp();
		
		int exit_code = translatorApp.processRequest(args);
		
		System.exit(exit_code);
	}

}
