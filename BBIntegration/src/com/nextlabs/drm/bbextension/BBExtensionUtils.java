package com.nextlabs.drm.bbextension;

import java.io.*;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import org.json.JSONArray;
import org.json.JSONTokener;

class BBExtensionUtils {
	private static final Logger logger = LogManager.getLogger(BBExtensionUtils.class);

	private BBExtensionUtils() { }

	static void writeToJsonFile(JSONArray jsonInput, String jsonFilePath) {
		try (BufferedWriter bw = new BufferedWriter(new FileWriter(jsonFilePath))) {
			jsonInput.write(bw);
		} catch (IOException ioe) {
			logger.error("Failed to write JSON file: ", ioe);
		}
	}

	static JSONArray readFromJsonFile(String jsonFilePath) {
		try {
			InputStream stream = new FileInputStream(jsonFilePath);
			JSONTokener jsonParser = new JSONTokener(stream);
			return new JSONArray(jsonParser);
		} catch (FileNotFoundException ex) {
			logger.error(() -> "Failed to load JSON File: " + jsonFilePath, ex);
			return new JSONArray();
		}
	}
}
