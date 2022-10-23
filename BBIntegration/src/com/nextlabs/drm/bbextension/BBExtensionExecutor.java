package com.nextlabs.drm.bbextension;

import java.io.*;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import org.json.JSONArray;
import org.json.JSONObject;

import com.nextlabs.bbrmx.BBRMXInstance;

class BBExtensionExecutor {
	private static final Logger logger = LogManager.getLogger(BBExtensionExecutor.class);
	private final String action;
	private final String parameter;
	private final BBRMXInstance rmxInstance;

	BBExtensionExecutor(String action, String parameter) throws Exception {
		this.action = action;
		this.parameter = parameter;
		this.rmxInstance = BBRMXInstance.getInstance();
	}

	void execute() {
		if (action.equalsIgnoreCase("encrypt_post_open") || action.equalsIgnoreCase("encrypt_post_generate")) {
			processEncryption();
		} else if (action.equalsIgnoreCase("retrieve_tag")) {
			processRetrieveTag();
		} else if (action.equalsIgnoreCase("add_rpm_dir")) {
			processAddRPMDir();
		} else if (action.equalsIgnoreCase("add_trusted_process")) {
			processAddTrustedProcess();
        } else {
			logger.error(() -> "UNDEFINED action: " + action);
        }
	}

	private void processRetrieveTag() {
		// In this process, parameter will be metadata tag file location
		File metadataTagFile = new File(parameter);
		File workingFolder = metadataTagFile.getParentFile();
		logger.info(() -> "Briefcase Browser Working Folder: " + workingFolder.getPath());

		// Construct JSONArray of NXL File
		// Each entry will have "nxl_file" and "metadata" entry
		JSONArray allNXLFileJSON = new JSONArray();
		List<String> allNXLFile = getNXLFilesFromDirectory(workingFolder);
		for (String nxlFilePath : allNXLFile) {
			// Only perform DRM command on .prt files for now
			if (!nxlFilePath.contains(".prt")) {
				continue;
			}

			String metadata = readMetaData(nxlFilePath);
			NxlFile nxlFile = new NxlFile(nxlFilePath, metadata);

			allNXLFileJSON.put(nxlFile.toJSONObject());
		}

		BBExtensionUtils.writeToJsonFile(allNXLFileJSON, parameter);
	}

	private void processEncryption() {
		// In this process, parameter will be metadata tag file location
		JSONArray allNXLFileJSON = BBExtensionUtils.readFromJsonFile(parameter);

		for(int i = 0; i < allNXLFileJSON.length(); i ++) {
			JSONObject jsonNXLFile = allNXLFileJSON.getJSONObject(i);
			NxlFile nxlFile = new NxlFile(jsonNXLFile);

			 String nxlFilePath = nxlFile.getFilePath();
			 String metadataStr = nxlFile.getMetadataStr();
			 boolean isFileReadOnly = !new File(nxlFilePath).canWrite();

			 encryptFile(nxlFilePath, metadataStr);


			if(isFileReadOnly && new File(nxlFilePath).setReadOnly()){
				logger.debug(() -> "Successfully set File read-only attribute for " + nxlFilePath);
			}
		}
	}

	private void encryptFile(String inputFilePath, String metadataStr) {
		String NXL_EXTENSION = ".nxl";
		if (rmxInstance.isFileProtected(inputFilePath)) {
			logger.info(() -> "Input File " + inputFilePath + " is already protected file.");
			if(new File(inputFilePath).renameTo(new File(inputFilePath + NXL_EXTENSION))) {
				logger.debug("Encrypted file is renamed to match file name in RPM folder");
			}
			return ;
		}

		// Need a safe Non RPM folder (with user permission) for the program to manipulate file extension
		Path safeNonRPM = Paths.get(System.getenv("USERPROFILE"), "NonRPM");
		File safeNonRPMDir = new File(safeNonRPM.toString());
		if (! safeNonRPMDir.exists()){
			safeNonRPMDir.mkdir();
		}
		String safeNonRPMFolder = safeNonRPM.toString();

		if (action.equalsIgnoreCase("encrypt_post_generate")) {
			if (rmxInstance.protectFile(inputFilePath, safeNonRPMFolder, metadataStr, false)) {
				logger.info(() -> "Protect File: " + inputFilePath);
			}
		} else {
			if (rmxInstance.protectFile(inputFilePath, safeNonRPMFolder, metadataStr, true)) {
				logger.info(() -> "Protect File: " + inputFilePath + NXL_EXTENSION);
			}
		}
	}

	private List<String> getNXLFilesFromDirectory(File directory) {
		List<String> nxlFileList = new ArrayList<>();

		File[] fileList = directory.listFiles();
		if (fileList == null)
			return nxlFileList;

		for (File file : fileList) {
			String filePath = file.getAbsolutePath();

			if (file.isFile() && rmxInstance.isFileProtected(filePath)) {
				nxlFileList.add(filePath);
			}
		}
		return nxlFileList;
	}

	private String readMetaData(String inputFilePath) {
		return rmxInstance.readNxlTags(inputFilePath);
	}

	private void processAddRPMDir() {
		// In this action, the second parameters will be RPM folder path
		if (rmxInstance.addRPMDir(parameter)) {
			logger.info(() -> "Successfully added RPM Dir: " + parameter);
		}
	}

	private void processAddTrustedProcess() {
		// In this action, the second parameters will be process ID (long)
		long pid = Long.parseLong(parameter);
		if (rmxInstance.addTrustedProcess(pid)) {
			logger.info(() -> "Added process " + pid + " as trusted process");
		}
	}
}
