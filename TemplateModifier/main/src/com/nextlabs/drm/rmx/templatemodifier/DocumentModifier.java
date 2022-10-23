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
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.Scanner;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.templatemodifier.configuration.ModifierConstants;

public abstract class DocumentModifier {
	
	private static final Logger logger = LogManager.getLogger("TMLOGGER");
	private String tcDataPath;
	private String filepath;
	private String filename;
	private String filetype;
	
	private String mainVerFilename;
	private File mainVerFile;
	
	public DocumentModifier(String filepath, String filename, String filetype) 
			throws NullPointerException, SecurityException {
		String path = System.getenv(ModifierConstants.TC_DATA_ENVVAR);
		
		if (path == null) {
			throw new NullPointerException("TC_DATA is not set");
		}
		
		setTcDataPath(path);
		
		this.filepath = filepath;
		this.filename = filename;
		this.filetype = filetype;
		
		mainVerFilename = filename.toLowerCase() + "." + filetype;
		mainVerFile = new File(getTcDataPath() + File.separator + filepath + File.separator + mainVerFilename);
	}
	
	/**
	 * Check whether the current template/definition file is customized
	 * with *.<file ext>.nxl
	 * @return boolean
	 * @throws FileNotFoundException
	 */
	public boolean isCurVerNXLModified() throws FileNotFoundException {
		boolean isNXLModified = false;
		
		Scanner scanner = new Scanner(mainVerFile);
		
		while (scanner.hasNextLine()) {
			if (scanner.nextLine().matches(ModifierConstants.MOD_LINE_REGEX)) {
				isNXLModified = true;
				break;
			}
		}
		
		if (scanner != null)
			scanner.close();
		
		return isNXLModified;
	}
	
	public void backupCurrentVersion() throws IOException {
		String backupVerFilename = mainVerFilename + "." + getVersionNumber(filepath);
		File backupVerFile = new File(getTcDataPath() + File.separator + filepath + File.separator + backupVerFilename);
		
		try {
			logger.debug("  Creating a new backup file {}", backupVerFile.getAbsolutePath());
			if (backupVerFile.createNewFile()) {
				logger.debug("  New backup file is created successfully");
				ModifierUtility.copyFileUsingFileChannels(mainVerFile, backupVerFile);
			} else {
				throw new IOException("Error: failed to create a new backup file " + backupVerFile.getAbsolutePath());
			}
		} catch (IOException e) {
			logger.error("DocumentModifier.backupCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
	}
	
	public void restoreFromLastBackup() throws IOException {
		String backupVerFilename = mainVerFilename + "." + (getVersionNumber(filepath) - 1); 
		File backupVerFile = new File(getTcDataPath() + File.separator + filepath + File.separator + backupVerFilename);
		
		try {
			if (!backupVerFile.exists()) {
				throw new IOException("Error: failed to restore from the backup file " + backupVerFile.getAbsolutePath());
			}
			
			logger.debug("  Deleting and recreating the main file {}", mainVerFile.getAbsolutePath());
			if (mainVerFile.delete() && mainVerFile.createNewFile()) {
				logger.debug("  Main file is deleted and recreated successfully");
				ModifierUtility.copyFileUsingFileChannels(backupVerFile, mainVerFile);
				logger.info("  Restore from {} is complete", backupVerFile.getAbsolutePath());
			
				if (ModifierConstants.ARCHIVED_BACKUP_FILES) {
					File archiveFolder = new File(getTcDataPath() + File.separator + filepath + File.separator + "nxl_cust_archive");
					File archiveFile = new File(archiveFolder.getAbsolutePath() + File.separator + backupVerFile.getName());
					
					if (!archiveFolder.exists()) {
						logger.debug("  Creating archive folder {}", archiveFolder.getAbsolutePath());
						boolean result = archiveFolder.mkdir();
						logger.debug("  Successfully create the archive folder {} is {}", archiveFolder.getAbsolutePath(), result);
					}
					
					logger.info("  Successfully archive the backup file {} is {}", backupVerFile.getName(), backupVerFile.renameTo(archiveFile));
				} else if (ModifierConstants.REMOVED_BACKUP_FILES) {
					logger.info("  Successfully remove the backup file {} is {}", backupVerFile.getName(), backupVerFile.delete());
				}
			} else {
				throw new IOException("Error: failed to restore from the backup file " + backupVerFile.getAbsolutePath());
			}
		} catch (IOException e) {
			logger.error("DocumentModifier.backupCurrentVersion() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		}
	}

	public String getTcDataPath() {
		return tcDataPath;
	}

	public void setTcDataPath(String tcDataPath) {
		this.tcDataPath = tcDataPath;
	}
	
	private int getVersionNumber(String filepath) {
		File file = new File(tcDataPath + File.separator + filepath);
		
		String[] filenameList = file.list(new FilenameFilter() {
			public boolean accept(File dir, String name) {
				if (name.matches(filename.toLowerCase() + "." + filetype + ".\\d+"))
					return true;
				else
					return false;
			}
		});
		
		// with the assumption no external party will mess up the file version naming part
		int count = filenameList.length + 1;
		
		// file name checking to prevent duplicate file name
		isNameDuplicate:
		while(true) {
			for (String filename : filenameList) {
				if (filename.equals(filename.toLowerCase() + "." + filetype + "." + count)) {
					count++;
					continue isNameDuplicate;
				}
			}
			
			break isNameDuplicate;
		}
		
		return count;
	}

}
