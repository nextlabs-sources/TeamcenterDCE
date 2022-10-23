package com.nextlabs.drm.tonxlfile;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public class TranslatedFile {
	
	private final String fileName;	// absolute file path
	private final String fileFormat;
	private final String namedReference;
	private final boolean isNXLFile; // fix bug 53542
	
	public TranslatedFile(String fileName, String fileFormat, String namedReference, boolean isNXLFile) {
		this.fileName = fileName;
		this.fileFormat = fileFormat;
		this.namedReference = namedReference;
		this.isNXLFile = isNXLFile;
	}

	public String getFileName() {
		return fileName;
	}

	public String getFileFormat() {
		return fileFormat;
	}

	public String getNamedReference() {
		return namedReference;
	}
	
	public boolean isNXLFile() {
		return isNXLFile;
	}

}
