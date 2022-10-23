package com.nextlabs.drm.bbextension;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class BBExtensionApp {
	private static final Logger logger = LogManager.getLogger(BBExtensionApp.class);
	private static final String LINE_SEPARATOR = System.getProperty("line.separator");

	public static void main(String[] args) {
		if (args.length != 2) {
			logger.error(() -> "Invalid number of arguments: " + args.length + ". Required exactly 2 arguments.");
			System.exit(1);
		} else {
			String action = args[0];
			String parameter = args[1];
			logger.info(LINE_SEPARATOR);
			logger.info(() -> "Action: " +  action);
			logger.info(() -> "Parameter: " +  parameter);
			try {
				BBExtensionExecutor bbExtensionExecutor = new BBExtensionExecutor(action, parameter);
				bbExtensionExecutor.execute();
			} catch (Exception e) {
				logger.error("Exception in main: ", e);
				System.exit(1);
			}
			System.exit(0);
		}
	}
}
