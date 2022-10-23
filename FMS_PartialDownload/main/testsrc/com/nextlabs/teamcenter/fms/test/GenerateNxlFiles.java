package com.nextlabs.teamcenter.fms.test;

import java.util.HashMap;
import java.util.Map;

import com.nextlabs.nxl.RightsManager;
import static com.nextlabs.common.shared.Constants.TokenGroupType.*;
import com.nextlabs.teamcenter.fms.configuration.NextLabsConfigInterface;
import com.nextlabs.teamcenter.fms.configuration.NextLabsConfigManager;
import static com.nextlabs.teamcenter.fms.test.TestConstants.*;

public class GenerateNxlFiles {
	
	public static void main(String[] args) throws Exception {
		NextLabsConfigManager nxlConfigManager = NextLabsConfigManager
				.getService();
		NextLabsConfigInterface nxlConfig = nxlConfigManager.getConfiguration();
		RightsManager rightsManager = new RightsManager(nxlConfig.getRouterURL(), 
				nxlConfig.getAppID(), nxlConfig.getAppKey());
		
		
		Map<String, String[]> tagMap = new HashMap<String, String[]>();
		rightsManager.encrypt(JT_PLAINTEXTFILE_PATH, 
				JT_ENCRYPTEDFILE_PATH, null, null, tagMap, null, TOKENGROUP_SYSTEMBUCKET);
		rightsManager.encrypt(TestConstants.PRT_PLAINTEXTFILE_PATH, 
				PRT_ENCRYPTEDFILE_PATH, null, null, tagMap, null, TOKENGROUP_SYSTEMBUCKET);
		rightsManager.encrypt(TestConstants.TXT_PLAINTEXTFILE_PATH, 
				TXT_ENCRYPTEDFILE_PATH, null, null, tagMap, null, TOKENGROUP_SYSTEMBUCKET);
	}

}
