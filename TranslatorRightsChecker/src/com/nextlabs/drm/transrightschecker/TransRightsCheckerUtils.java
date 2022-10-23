package com.nextlabs.drm.transrightschecker;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Map;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

class TransRightsCheckerUtils {
	private static final Logger logger = LogManager.getLogger(TransRightsCheckerUtils.class);

    private TransRightsCheckerUtils() {}

	static String getInputPathFromArgs(String[] args) {
        // Add a blank new line for new session
        logger.info(System.getProperty("line.separator"));

        String firstPara = args[0];
        String secondPara = "";
        if (args.length > 1)
            secondPara = args[1];

        // if 1st param is -i or -input_dir then 2nd param should be input file path or input folder
        // else the first param should be input file or input folder (without flag option)
        if (firstPara.startsWith("-"))
            return secondPara;
        else {
            return firstPara;
        }
    }

	static String buildMetadataString(Map<String, String[]> metadata) {
        StringBuilder sb = new StringBuilder("{");
        String outputStr;
        // Append "key=[...], " to metaStr
        for (Map.Entry<String, String[]> entry : metadata.entrySet()) {
            sb.append(entry.getKey());
            sb.append("=");
            sb.append(Arrays.toString(entry.getValue()));
            sb.append(", ");
        }
        // Close bracket. The end of the string now is ", ". Need to replace with "}"
        String metaStr = sb.toString();
        if (metaStr.length() >= 2)
            outputStr = metaStr.substring(0, metaStr.length()-2);
        else // In case we have no metadata at all. Output should be "{}"
            outputStr = metaStr;
        outputStr = outputStr + "}";
        return outputStr;
	}

	static Properties loadPropertiesFromFile(String propertyFilePath) {
		Properties props = new Properties();
		try (InputStream input = new FileInputStream(propertyFilePath)) {
            props.load(input);
        } catch (FileNotFoundException fex) {
		    logger.error("Properties file not found: ", fex);
		} catch (IOException ioe) {
			logger.error("Error when loading properties: ", ioe);
		}
		return props;
	}

}
