package com.nextlabs.drm.rmx.translator;

/**
 * Created on October 6, 2015
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.nextlabs.common.shared.Constants.TokenGroupType;
import com.nextlabs.drm.rmx.configuration.ConfigManager;
import com.nextlabs.drm.rmx.configuration.ConfigPropertiesDTO;
import com.nextlabs.nxl.RightsManager;

public class Translator {
	
	public static final String EXT_NXL_FILE = ".nxl";
	public static final String CHARSET = "UTF-8";
	
	private static final String NXLACTION_PROTECT = "protect";
	private static final String NXLACTION_UPDATETAG = "updateTag";
	private static final String NXLACTION_REMOVEPROTECTION = "unprotect";
	private static final String NXLACTION_REFRESHDUID = "refreshduid";
	
	public static final int SUCCESS_CODE = 1;
	public static final int INVALID_CODE = 0;
	
	private final RightsManager manager;
	private final ConfigPropertiesDTO configProp;
	private final Logger logger;
	
	public Translator() throws Exception {
		File baseFile = new File(ClassLoader.getSystemClassLoader().getResource(".").toURI());
		
		logger = LoggerFactory.getLogger(Translator.class);
		logger.debug("jarPath: {}", baseFile.getAbsolutePath());
				
		try {
			ConfigManager configManager = new ConfigManager.Builder(baseFile).buildConfig();
			configProp = configManager.getConfigPropertiesDTO();
			
			manager = new RightsManager(configProp.getRouterURL(), configProp.getAppID(), configProp.getAppKey());
			logger.info("SkyDRM URL:{}, AppID: {}, TenantName: {}", 
					configProp.getRouterURL(), configProp.getAppID(), configProp.getTenantName());
		} catch (Exception ex) {
			logger.error("Failed to initialed PolicyControllerDetails: {}", ex.getMessage());
			throw ex;
		}
	}
	
	// 1: for success case
	// 0: for invalid case
	public int translate(String nxlAction, String inputPath, String outputPath, 
			String filename, Map<String, String[]> tagMap) throws Exception {
		logger.info("RMX Translation started");
		int return_code = SUCCESS_CODE;
		
		String inputFilePath = inputPath + File.separator + filename;
		
		// bajaj hotfix 20170928
		if (nxlAction.equals(NXLACTION_PROTECT) && isNXLFile(inputFilePath)) {
			// something wrong when command is protect but file is protected
			// bajaj scenario: display name has no nxl extension, but volume file has
			// hotfix: swap the action from protect to updatetag
			logger.info("==> SWAP action from {} to {}", NXLACTION_PROTECT, NXLACTION_UPDATETAG);
			nxlAction = NXLACTION_UPDATETAG;
		}
		
		if (nxlAction.equalsIgnoreCase(NXLACTION_UPDATETAG)) {
			logger.info("Source file {} is NXL protected", filename);
			
			// bajaj hotfix 20170928
			String outputFilePath = outputPath + File.separator + appendExtension(filename);
			
			Map<String, String[]> tagsToUpdate = new HashMap<>();
			
			// read metadata first
			boolean isTagUpdated = true;
			
			try {
				// read metadata first
				Map<String, String[]> readMeta = manager.readTags(inputFilePath);
				isTagUpdated = setupTagsToUpdate(readMeta, tagMap, tagsToUpdate);
			} catch (Exception ex) {
				logger.error("Failed to read metadata from the protected file {}", inputFilePath);
				throw ex;
			}
			
			
			File fileInput = new File(inputFilePath);
			File fileOutput = new File(outputFilePath);
			
			try {
				copyFileUsingFileChannels(fileInput, fileOutput);
			} catch (IOException ioe) {
				logger.error("Failed to copy the protected file {} to {}", inputFilePath, outputFilePath);
				throw ioe;
			}
			
			if (isTagUpdated) {
				try {
					logger.debug("Updating the tags of protected file {}", outputFilePath);
					manager.updateTags(outputFilePath, tagsToUpdate, null, TokenGroupType.TOKENGROUP_SYSTEMBUCKET, null);
				} catch (Exception ex) {
					logger.error("Failed to update the protected file {}", outputFilePath);
					throw ex;
				}
			}
		} else if (nxlAction.equalsIgnoreCase(NXLACTION_REMOVEPROTECTION)) {
			if (isNXLFile(inputFilePath)) {
				logger.info("Source file {} is NXL protected", filename);
				
				String outputFilePath = outputPath + File.separator + truncateExtension(filename); // truncate nxl file extension
				
				try {				
					logger.debug("Decrypting the protected file {} to {}", inputFilePath, outputFilePath);
					manager.decrypt(inputFilePath, outputFilePath, null, TokenGroupType.TOKENGROUP_SYSTEMBUCKET);
				} catch (Exception ex) {
					logger.error("Failed to decrypt the protected file {} to {}", inputFilePath, outputFilePath);
					throw ex;
				} finally {
					// remove the source file from result folder
					//File file = new File(inputFilePath);
					//if (!file.delete()) {
					//	logger.debug("Failed to remove the encrypted file " + file.getAbsolutePath());
					//}
				}
			} else {
				logger.info("Removing protection on a nonprotected file {}. Action is skipped", filename);
				return_code = INVALID_CODE;
			}
		} else if (nxlAction.equalsIgnoreCase(NXLACTION_REFRESHDUID)) {
			if (isNXLFile(inputFilePath)) {
				logger.info("Source file {} is NXL protected", filename);
				
				String outputFilePath = outputPath + File.separator + appendExtension(filename);
				
				try {
					logger.debug("Refresh DUID from the protected file {} to {}", inputFilePath, outputFilePath);
					manager.copyNXLwithNewDUID(inputFilePath, outputFilePath, null, TokenGroupType.TOKENGROUP_SYSTEMBUCKET);
				} catch (Exception ex) {
					logger.error("Fialed to refresh DUID from the protected file {} to {}", inputFilePath, outputFilePath);
					throw ex;
				}
			} else {
				logger.info("Refresh DUID on a nonprotected file {}. Action is skipped", filename);
				return_code = INVALID_CODE;
			}
		} else {
			logger.info("Source file {} is not NXL protected", filename);
			
			String outputFilePath = outputPath + File.separator + filename + EXT_NXL_FILE;
			
			try {
				logger.debug("Encrypting the source file {} to {}", inputFilePath, outputFilePath);
				manager.encrypt(inputFilePath, outputFilePath, null, null, tagMap, null, TokenGroupType.TOKENGROUP_SYSTEMBUCKET);
			} catch (Exception ex) {
				logger.error("Failed to encrypt the source file {} to {}", inputFilePath, outputFilePath);
				throw ex;
			}
		}
		
		logger.info("RMX Translation completed");
		
		return return_code;
	}
	
	private String truncateExtension(String filename) {
		int indexLastDot = filename.toLowerCase().lastIndexOf(EXT_NXL_FILE);
		
		if (indexLastDot > 0) {
			return filename.substring(0, indexLastDot);
		} else {
			return filename; 
		}
	}
	
	// bajaj hotfix 20170928
	private String appendExtension(String filename) {
		int indexLastDot = filename.toLowerCase().lastIndexOf(EXT_NXL_FILE);
		
		if (indexLastDot > 0) {
			return filename;
		} else {
			// append nxl
			return filename + EXT_NXL_FILE;
		}
	}
	
	private boolean setupTagsToUpdate(Map<String, String[]> existingMetaData, Map<String, String[]> tagMap, 
			Map<String, String[]> tagsToUpdate) {
		Iterator<Entry<String,String[]>> it = existingMetaData.entrySet().iterator();
		boolean isTagUpdated = false;
		
		while (it.hasNext()) {
			Entry<String, String[]> entry = it.next();
			String key = entry.getKey();
			String[] value = entry.getValue();
			
			if (tagMap.containsKey(key) && !isTwoListEqual(Arrays.asList(value), Arrays.asList(tagMap.get(key)))) {
				isTagUpdated = true;
				// sync metadata with business object
				tagsToUpdate.put(key, tagMap.get(key));	
			} else {
				// retain existing non business object metadata, i.e. defined in policy obligation
				tagsToUpdate.put(key, value);
			}
		}
		
		return isTagUpdated;
	}
	
	private boolean isTwoListEqual(List<String> list1, List<String> list2) {
		if (list1 == null && list2 == null)
			return true;
		
		if ((list1 == null && list2 != null) ||
				(list1 != null && list2 == null) ||
				(list1.size() != list2.size()))
			return false;
		
		return new HashSet<>(list1).equals(new HashSet<>(list2));
	}
	
	private static void copyFileUsingFileChannels(File source, File dest)
	        throws IOException {
	    try (
	    		FileChannel inputChannel = new FileInputStream(source).getChannel();
	    		FileChannel outputChannel = new FileOutputStream(dest).getChannel();) {
	        outputChannel.transferFrom(inputChannel, 0, inputChannel.size());
	    }
	}
	
	
	public Map<String, String[]> loadTagsFromFile(File file) {
		Map<String, String[]> tagMap = new HashMap<>();
		
		if (file.exists()) {
			try (
					InputStreamReader isr = new InputStreamReader(new FileInputStream(file), CHARSET);
					BufferedReader br = new BufferedReader(isr);) {
				
				String line = br.readLine();
				while (line != null) {
					String[] kvp = line.trim().split("=");
					
					if (kvp.length >= 2) {
						String[] values = kvp[1].split(",");
						for (int i = 0; i < values.length; i++) {
							values[i] = values[i].trim();
						}
						
						tagMap.put(kvp[0], values);
					} else {
						logger.debug("Bad format key value pair - {}", line);
					}
					
					line = br.readLine();
				}
				
			} catch (FileNotFoundException e) {
				logger.error(e.getMessage(), e);
			} catch (IOException e) {
				logger.error(e.getMessage(), e);
			}
		}
		
		return tagMap;
	}
	
	// bajaj hotfix 20170928
	private boolean isNXLFile(String filePath) {		
		File file = new File(filePath);
		boolean isNxl = false;
		
		try {
			logger.info("Is {} NXL protected by verifying file header", filePath);
			
			if (file.exists()) {
				isNxl = manager.isNXL(filePath);
				
				logger.info("{} isNXLProtected={}", filePath, isNxl);
			}
		} catch (Exception ex) {
			logger.error("Failed to verify the input file: {}", ex.getMessage(), ex);
			isNxl = false;
		}
		
		if (!isNxl)
			logger.info("{} is not NXL protected", filePath);
		
		return isNxl;
	}

}
