package com.nextlabs.drm.bbextension;

import com.nextlabs.bbrmx.BBRMXInstance;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.nio.file.Path;
import java.nio.file.Paths;

public class BBRMXInstanceTest {

    private static BBRMXInstance rmxInstance;
    private static final Logger logger = LogManager.getLogger(BBRMXInstanceTest.class);
    private static String inputFilePath;

    public static void main(String[] args) {
        try {
            rmxInstance = BBRMXInstance.getInstance();
        } catch (Exception ex) {
            logger.error("Failed to start RightsManager object", ex);
        }

        String action = args[0];
        inputFilePath = args[1];
        switch (action) {
            case "encrypt":
                logger.info("Encrypt Action: ");
                executeEncrypt();
                break;
            case "readMetadataStr":
                logger.info("Check Metadata Str: ");
                executeReadMetadataStr();
                break;
            default:
                break;
        }
    }

    public static void executeEncrypt() {
        Path file = Paths.get(inputFilePath);
        String outputFolder = String.valueOf(file.getParent());

        if (!rmxInstance.isFileProtected(inputFilePath)) {
            logger.info("File is not protected");
        }

        // Valid Tags
        // {"ip_classification":["secret","super-secret"], "ITAR":["TAR1"]}
        // {"ip_classification":["secret","super-secret"]}
        // {"ip_classification":["secret"]}

        if (rmxInstance.protectFile(inputFilePath, outputFolder, "{\"ip_classification\":[\"secret\",\"super-secret\"], \"ITAR\":[\"TAR1\"]}", true)) {
            logger.info("Protect Success");
            logger.info("Output folder: " + outputFolder);
        }
    }

    public static void executeReadMetadataStr() {
        if (rmxInstance.isFileProtected(inputFilePath)) {
            logger.info("File is protected");
        }

        String metadata = rmxInstance.readNxlTags(inputFilePath);
        logger.info("Metadata read from " + inputFilePath);
        logger.info(metadata);
    }
}
