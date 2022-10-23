package com.nextlabs.drm.transrightschecker;

import java.io.File;
import java.util.*;

import com.nextlabs.common.shared.Constants;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.configuration.ConfigManager;
import com.nextlabs.drm.rmx.configuration.ConfigPropertiesDTO;
import com.nextlabs.drm.transrightschecker.config.NextLabsAuthorizationAgent;
import com.nextlabs.nxl.RightsManager;

import static com.nextlabs.drm.transrightschecker.config.NextLabsPolicyConstants.*;

class TransRightsCheck {
	private static final Logger logger = LogManager.getLogger(TransRightsCheck.class);

	private RightsManager manager;
	private Properties policyEvaluationProperties;
	private NextLabsAuthorizationAgent agent;

	TransRightsCheck() {
		try {
		    initializeRightsManager();
		    initializeNextLabsAuthorizationAgent();
		} catch (Exception ex) {
			logger.error("Fail to initialize TransRightsCheck object: ", ex);
		}
	}

	private void initializeRightsManager() throws Exception {
        // Initialize ConfigManager. Find config.properties file in %RMX_ROOT%\config folder
        String configDirectory = System.getenv("RMX_ROOT") + File.separator + "config";
        ConfigManager configManager = new ConfigManager.Builder(new File(configDirectory)).buildConfig();
        ConfigPropertiesDTO configProperties = configManager.getConfigPropertiesDTO();

        // Initialize SkyDRM Rights Manager base on those config parameters
        manager = new RightsManager(configProperties.getRouterURL(), configProperties.getAppID(), configProperties.getAppKey());
        logger.info("SkyDRM URL: " + configProperties.getRouterURL());
        logger.info("AppID: " + configProperties.getAppID());
    }

    private void initializeNextLabsAuthorizationAgent() {
        // Load policy evaluation properties (Location in %RMX_ROOT%\TranslatorRightsCheck\)
        String policyEvaConfigFile = System.getenv("RMX_ROOT") + File.separator + "TranslatorRightsChecker" + File.separator + "policy_evaluation.properties";
        policyEvaluationProperties = TransRightsCheckerUtils.loadPropertiesFromFile(policyEvaConfigFile);

        // Get instance of NextLabs Authorization Agent
		agent = NextLabsAuthorizationAgent.getInstance();
        logger.info("Successfully initialize NextLabsAuthorizationAgent");
    }

	void processDecryption(String inputPath) {
		File path = new File(inputPath);
		if (path.isFile()) {
            processDecryptionForSingleFile(inputPath);
		} else if (path.isDirectory()) {
            processDecryptionForDirectory(inputPath);
		}
	}

	private void processDecryptionForSingleFile(String inputFilePath) {
        logger.info("Translator Rights Checker for single file: " + inputFilePath);
        decryptFileWithRightCheck(inputFilePath);
    }

	private void processDecryptionForDirectory(String inputDirectory) {
        File directory = new File(inputDirectory);
        List<String> nxlFiles = getNXLFilesFromDirectory(directory);
        logger.info("Translator Rights Checker for a directory: " + inputDirectory);

        for (String filePath : nxlFiles) {
            decryptFileWithRightCheck(filePath);
        }
    }

	private List<String> getNXLFilesFromDirectory(File directory) {
		List<String> nxlFileList = new ArrayList<>();

		File[] fileList = directory.listFiles();
		if (fileList == null)
			return nxlFileList;

		for (File file : fileList) {
			String filePath = file.getAbsolutePath();
			if (file.isFile() && manager.isNXL(filePath)) {
				nxlFileList.add(filePath);
			}
		}
		return nxlFileList;
	}

	private void decryptFileWithRightCheck(String inputFilePath) {
	    logger.info("Processing " + inputFilePath);
		// Decrypt on server-side, input file should always have .nxl extension
	    if (inputFilePath.endsWith(".nxl") && policyEvaluation(inputFilePath))
	        decryptFile(inputFilePath);
    }

	private void decryptFile(String inputFilePath) {
		String outputFilePath = inputFilePath.substring(0, inputFilePath.length() - 4);
		try {
			manager.decrypt(inputFilePath, outputFilePath, null, Constants.TokenGroupType.TOKENGROUP_SYSTEMBUCKET);
			logger.info("Decrypted file is: " + outputFilePath);
		} catch (Exception ex) {
			logger.error("Failed to decrypt protected file: ", ex);
		}
	}

	private boolean policyEvaluation(String inputFilePath) {
		// Metadata will pass to Policy Evaluation process (only choose from policy_evaluation.properties)
		Map<String, String[]> classifyMetadata = new HashMap<>();
		// Get File's metadata for policy evaluation
		Map<String, String[]> metadata = getFileMetaData(inputFilePath);

		String[] evaluationAttrs = policyEvaluationProperties.getProperty(NXL_EVALUATION_ATTRIBUTES).split(",");
		for (String evalAttribute : evaluationAttrs) {
			String[] values = metadata.get(evalAttribute);
			if (null != values && values.length > 0) {
				classifyMetadata.put(evalAttribute, values);
			}
		}

		String displayUserName = policyEvaluationProperties.getProperty(DISP_USER_NAME);
		String pdfDefaultAction = policyEvaluationProperties.getProperty(NXL_PDP_DEFAULT_ACTION);
		String pdfDefaultMsg = policyEvaluationProperties.getProperty(NXL_PDP_DEFAULT_MESSAGE);
		return agent.checkNxlPolicy(displayUserName, classifyMetadata, pdfDefaultAction, pdfDefaultMsg);

	}

    private Map<String, String[]> getFileMetaData(String inputFilePath) {
		Map<String, String[]> tags = new HashMap<>();
        try {
            tags = manager.readTags(inputFilePath);
            logger.info("MetaData: " + TransRightsCheckerUtils.buildMetadataString(tags));
        } catch (Exception ex) {
            logger.error("Failed to read metadata: ", ex);
        }
        return tags;
    }
}
