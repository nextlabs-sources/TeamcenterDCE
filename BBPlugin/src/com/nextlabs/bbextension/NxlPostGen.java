package com.nextlabs.bbextension;

import java.io.File;
import java.util.List;

import com.teamcenter.bce.logger.BriefcaseLogger;
import com.teamcenter.bce.model.Briefcase;
import com.teamcenter.bce.model.BriefcaseIdentifier;
import com.teamcenter.bce.extension.PostBriefcaseGeneration;

// Extension Point: This extension point allow for modification of entities that will eventually becomes contents of a briefcase archive.
// At this moment, any protected .prt file is modified. We have to encrypt any file need re-protection. The output package will auto have protected file. 
public class NxlPostGen implements PostBriefcaseGeneration {
	
	private static final BriefcaseLogger logger = new BriefcaseLogger(NxlPostGen.class);

	public NxlPostGen() { }

	@Override
	public void process(List<File> fileList, Briefcase briefcase, BriefcaseIdentifier briefcaseId) {
		File workingDir = briefcase.getWorkingDirectory();
		String metadataTagsJSON = workingDir.getPath() + File.separator + "metadataTags.json";
		String bbExtension = System.getenv("RMX_ROOT") + File.separator + "BBIntegration" + File.separator + "bbExtension.bat";
		try {
			String cmdReProtect = bbExtension + " encrypt_post_generate \"" + metadataTagsJSON + "\"";
			logger.info("Suppose no action in Post Generate bcz file");
			Process psReProtect = Runtime.getRuntime().exec(cmdReProtect);
			psReProtect.waitFor();
		} catch (Exception ex) {
			logger.error("NextLabs PostGen Failed: ", ex);
		}
	}

}
