package com.nextlabs.drm.bbextension;

import org.json.JSONObject;

class NxlFile {
    private final String filePath;
    private final String metadataStr;

    NxlFile(String filePath, String metadataStr) {
        this.filePath = filePath;
        this.metadataStr = metadataStr;
    }

    NxlFile(JSONObject jsonNxlFile) {
        this.filePath = (String) jsonNxlFile.get("nxl_file");
        this.metadataStr = (String) jsonNxlFile.get("metadata");
    }

    JSONObject toJSONObject() {
        // Convert NXL File path and metadata information to JSON format
        JSONObject nxlFileJson = new JSONObject();
        nxlFileJson.put("nxl_file", filePath);
        nxlFileJson.put("metadata", metadataStr);
        return nxlFileJson;
    }

    String getFilePath() {
        return filePath;
    }

    String getMetadataStr() {
        return metadataStr;
    }
}
