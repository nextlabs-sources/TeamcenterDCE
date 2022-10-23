package com.nextlabs.drm.rmx.batchprocess.dto;

import java.io.File;

import com.teamcenter.soa.client.model.strong.ImanFile;

public class TaskFileDTO {
	
	private final String imanFileUid;
	private final String namedRef;
	private final ImanFile imanFile;
	private File localFile;
	
	public TaskFileDTO(String imanFileUid, String namedRef, 
			ImanFile imanFile, File localFile) {
		this.imanFileUid = imanFileUid;
		this.namedRef = namedRef;
		this.imanFile = imanFile;
		this.localFile = localFile;
	}

	public String getImanFileUid() {
		return imanFileUid;
	}
	
	public String getNamedRef() {
		return namedRef;
	}
	
	public ImanFile getImanFile() {
		return imanFile;
	}

	public File getLocalFile() {
		return localFile;
	}

	public void setLocalFile(File localFile) {
		this.localFile = localFile;
	}

}
