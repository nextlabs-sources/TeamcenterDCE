package com.nextlabs.tc.ldif.utility;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class UserMapping {
    private static final Logger logger = LogManager.getLogger("TCLDIFLOGGER");

    String userMappingFile;
    Properties userMap;

    public UserMapping(String userMappingFile) {
        this.userMappingFile = userMappingFile;
        loadMappingFile();
    }

    public void loadMappingFile() {
        if (userMappingFile != null && !userMappingFile.isEmpty()) {
            try (InputStream inputStream = new FileInputStream(userMappingFile)) {
                userMap = new Properties();
                userMap.load(inputStream);
            } catch (IOException ex) {
                logger.error("Error in loading User Mapping: ", ex);
                userMap = null;
            }
        } else {
            userMap = null;
        }
    }

    public String get(String tcUserName) {
        if (userMap == null)
            return tcUserName;

        if (userMap.containsKey(tcUserName)) {
            String mappingUserName = userMap.getProperty(tcUserName);
            logger.info(() -> "Find mapping for " + tcUserName + " to " + mappingUserName);
            return mappingUserName;
        } else {
            return tcUserName;
        }
    }
}
