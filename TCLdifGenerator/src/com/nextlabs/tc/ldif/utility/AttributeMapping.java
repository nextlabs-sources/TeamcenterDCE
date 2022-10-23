package com.nextlabs.tc.ldif.utility;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class AttributeMapping {
    private static final Logger logger = LogManager.getLogger("TCLDIFLOGGER");

    String attributeMappingFile;
    Properties attributeMap;

    public AttributeMapping(String attributeMappingFile) {
        this.attributeMappingFile = attributeMappingFile;
        loadMappingFile();
    }

    public void loadMappingFile() {
        if (attributeMappingFile != null && !attributeMappingFile.isEmpty()) {
            try (InputStream inputStream = new FileInputStream(attributeMappingFile)) {
                attributeMap = new Properties();
                attributeMap.load(inputStream);
            } catch (IOException ex) {
                logger.error("Error in loading Attribute Map: ", ex);
                attributeMap = null;
            }
        } else {
            attributeMap = null;
        }
    }

    public String get(String attributeName) {
        if (attributeMap == null)
            return attributeName;

        if (attributeMap.containsKey(attributeName)) {
            String mappingAttrName = attributeMap.getProperty(attributeName);
            logger.debug(() -> "Find mapping for " + attributeName + " to " + mappingAttrName);
            return mappingAttrName;
        } else {
            return attributeName;
        }
    }
}
