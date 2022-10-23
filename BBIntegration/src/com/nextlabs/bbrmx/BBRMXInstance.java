package com.nextlabs.bbrmx;

public class BBRMXInstance {

	public native boolean loadDRmUser();

	public native boolean loadDRmInstance();

	public native boolean protectFile(String inFilePath, String outFolderPath, String tags, boolean withNxlExt);

	public native boolean isFileProtected(String filePath);

	public native String readNxlTags(String nxlFilePath);

	public native boolean addTrustedProcess(long processId);

	public native boolean addRPMDir(String folderPath);

	private static BBRMXInstance instance;

	static {
		System.loadLibrary("BBRMX");
	}

	private BBRMXInstance() throws Exception {
		if (!loadDRmUser())
			throw new Exception("Failed to loadDRmUser");

		if (!loadDRmInstance())
			throw new Exception("Failed to loadDRmInstance");
	}

	public static synchronized BBRMXInstance getInstance() throws Exception {
		if (instance == null)
			instance = new BBRMXInstance();

		return instance;
	}

}
