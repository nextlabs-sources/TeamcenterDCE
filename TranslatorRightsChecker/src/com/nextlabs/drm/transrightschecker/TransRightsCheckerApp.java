package com.nextlabs.drm.transrightschecker;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class TransRightsCheckerApp {
	private static final Logger logger = LogManager.getLogger(TransRightsCheckerApp.class);

	//SonarQube Toke for this project
	//da315e91bc6d404e4d12cc3fff5de5fe1696e114

	public static void main(String[] args) {
		try {
			String inputPath = TransRightsCheckerUtils.getInputPathFromArgs(args);
			// Start Decryption + Policy Evaluation process
			TransRightsCheck translator = new TransRightsCheck();
			translator.processDecryption(inputPath);
		} catch (Exception ex) {
			logger.error("Error when process TranslatorRightsCheck: ", ex);
			// Exit Status: Error
			System.exit(1);
		}
		// Exit Status: Normal
		System.exit(0);
	}
}
