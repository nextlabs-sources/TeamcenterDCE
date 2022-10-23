package com.nextlabs.drm.tonxlfile.configuration;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import java.util.HashMap;
import java.util.Map;

import com.teamcenter.services.strong.administration.PreferenceManagementService;
import com.teamcenter.services.strong.administration._2012_09.PreferenceManagement.GetPreferencesResponse;
import com.teamcenter.soa.client.Connection;
import static com.nextlabs.drm.tonxlfile.configuration.NextLabsConstants.*;

public class TeamcenterPreferences implements NextLabsConfigInterface {
	
	private PreferenceManagementService prefService;
	
	// 3.5 feature
	private String pdpDefaultAction;
	private String pdpDefaultMessage;
	
	private String[] transferredAttr;
	private String[] evaluationAttr;
	private boolean removeDRMSrcFile;
	
	// 3.0 feature
	private String[] nrExclusionList;
	private int dsCheckoutRetryCount;
	private int dsCheckoutWaitTime;
	
	// 4.5 feature
	private Map<String, String> installedFeatures = new HashMap<>();
	
	// 5.2 feature
	private boolean purgeRevSeq;
	
	public TeamcenterPreferences(Connection conn) {
		prefService = PreferenceManagementService.getService(conn);
		
		initInstalledFeatures();
	}
	
	// cache the preference value during first time setup
	private void initTransferredAttr() {
		transferredAttr = readTCPreference(NXL_TRANSFERRED_ATTRIBUTES);
	}
	
	// 5.2 feature
	@Override
	public boolean getPurgeRevSeq() {
		initPurgeRevSeq();
		
		return purgeRevSeq;
	}
		
	// 5.2 feature
	private void initPurgeRevSeq() {
		purgeRevSeq = false;
		
		try {
			prefService.refreshPreferences();
			
			purgeRevSeq = Boolean.parseBoolean(readTCPreference(NXL_PURGE_REV_SEQ)[0]);
		} catch (Exception ex) {
			ex.printStackTrace(System.out);
		}
	}
	
	// 4.5 feature
	private void initInstalledFeatures() {
		String[] installedFeaturesArr = readTCPreference(NXL_INSTALLED_FEATURES);
		
		for (String feature: installedFeaturesArr) {
			installedFeatures.put(feature, feature);
		}
	}
	
	// 3.5 feature
	// read the policy evaluation attributes
	private void initEvaluationAttr() {
		evaluationAttr = readTCPreference(NXL_EVALUATION_ATTRIBUTES);
	}
	
	// 3.5 feature
	private void initPdpDefaultAction() {
		pdpDefaultAction = readTCPreference(NXL_PDP_DEFAULT_ACTION)[0];
	}
	
	// 3.5 feature
	private void initPdpDefaultMessage() {
		pdpDefaultMessage = readTCPreference(NXL_PDP_DEFAULT_MESSAGE)[0];
	}
		
	private void initRemoveDRMSrcFile() {
		removeDRMSrcFile = false; // default is keeping the both files
		
		try {
			// improvement 18 Oct 2016: refresh TC preferences
			prefService.refreshPreferences();
			
			removeDRMSrcFile = Boolean.parseBoolean(readTCPreference(NXL_REMOVE_SOURCEFILE)[0]);
		} catch (Exception ex) {
			ex.printStackTrace(System.out);
		}
	}
	
	// 3.0 feature
	private void initNRExclusionList() {
		nrExclusionList = readTCPreference(NXL_NR_BLACKLIST);
	}
	
	// 3.0 feature
	private void initDSCheckoutRetryCount() {
		dsCheckoutRetryCount = 0;
		
		if (readTCPreference(NXL_DS_CHKOUT_RETRYCOUNT).length > 0 && 
				readTCPreference(NXL_DS_CHKOUT_RETRYCOUNT)[0] != null) {
			try {
				dsCheckoutRetryCount = Integer.parseInt(
						readTCPreference(NXL_DS_CHKOUT_RETRYCOUNT)[0]);
			} catch (NumberFormatException nfe) {
				// default value is zero
			}
		}
	}
	
	// 3.0 feature
	private void initDSCheckoutWaitTime() {
		dsCheckoutWaitTime = 0;
		
		if (readTCPreference(NXL_DS_CHKOUT_WAITTIME).length > 0 && 
				readTCPreference(NXL_DS_CHKOUT_WAITTIME)[0] != null) {
			try {
				dsCheckoutWaitTime = Integer.parseInt(
						readTCPreference(NXL_DS_CHKOUT_WAITTIME)[0]);
			} catch (NumberFormatException nfe) {
				// default value is zero
			}
		}
	}
	
	private String[] readTCPreference(String pref_name) {
		String[] pref_values = new String[] {};
			
		try {
			// improvement 18 Oct 2016: refresh TC preferences
			prefService.refreshPreferences();
			
			GetPreferencesResponse prefResponse = prefService.getPreferences(
					new String[] {pref_name}, false);
			
			if (prefResponse.response != null &&
					prefResponse.response.length >= 1) {
				
				if (prefResponse.response[0].values != null && 
						prefResponse.response[0].values.values != null) {
					
					pref_values = prefResponse.response[0].values.values;
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace(System.out);
		}
		
		return pref_values;
	}
	
	// 4.5 feature
	// need to restart DC to refresh this list
	// assume the features does not alter during runtime.
	@Override
	public Map<String, String> getInstalledFeatures() {		
		return installedFeatures;
	}
	
	// 3.5 feature
	@Override
	public String getPdpHost() {
		String pdpHostServerApp = readTCPreference(NXL_PDPHOST_SERVERAPP)[0];
		return pdpHostServerApp;
	}
	
	// 3.5 feature
	@Override
	public String getPdpPort() {
		String pdpPortServerApp = readTCPreference(NXL_PDPPORT_SERVERAPP)[0];
		return pdpPortServerApp;
	}
	
	@Override
	public String getPdpHttps() {
		String pdpHttpsServerApp = readTCPreference(NXL_PDP_HTTPS_SERVERAPP)[0];
		return pdpHttpsServerApp;
	}
	
	@Override
	public String getPdpIgnoreHttps() {
		String pdpIgnoreHttps = readTCPreference(NXL_PDP_IGNOREHTTPS_SERVERAPP)[0];
		return pdpIgnoreHttps; 
	}
	
	@Override
	public String getOAuth2Host() {
		String oauth2Host = readTCPreference(NXL_OAUTH2HOST)[0];
		return oauth2Host;
	}
	
	@Override
	public String getOAuth2Port() {
		String oauth2Port = readTCPreference(NXL_OAUTH2PORT)[0];
		return oauth2Port;
	}
	
	@Override
	public String getOAuth2Https() {
		String oauth2Https = readTCPreference(NXL_OAUTH2_HTTPS)[0];
		return oauth2Https;
	}
	
	@Override
	public String getOAuth2ClientID() {
		String oauth2ClientID = readTCPreference(NXL_OAUTH2_CLIENTID)[0];
		return oauth2ClientID;
	}
	
	@Override
	public String getOAuth2ClientSecret() {
		String oauth2ClientSecret = readTCPreference(NXL_OAUTH2_CLIENTSECRET)[0];
		return oauth2ClientSecret;
	}
	
	// 3.5 feature
	@Override
	public String getPdpDefaultAction() {
		initPdpDefaultAction();
		
		return pdpDefaultAction;
	}
	
	// 3.5 feature
	@Override
	public String getPdpDefaultMessage() {
		initPdpDefaultMessage();
		
		return pdpDefaultMessage;
	}
	
	// 3.0 feature
	@Override
	public String[] getNRExclusionList() {
		initNRExclusionList();
		
		return nrExclusionList;
	}
	
	@Override
	public String[] getTransferredAttr() {
		// improvement 18 Oct 2016: Don't need to restart service after changes on preferences
		// drawback, overhead increases during access to the getTransferredAttr()
		initTransferredAttr();
		
		return transferredAttr;
	}
	
	// 3.5 feature
	@Override
	public String[] getEvaluationAttr() {
		initEvaluationAttr();
		
		return evaluationAttr;
	}
	
	@Override
	public boolean getRemoveDRMSrcFile() {
		// improvement 18 Oct 2016: Don't need to restart service after changes on preferences
		// drawback, overhead increases during access to the getRemoveDRMSrcFile()
		initRemoveDRMSrcFile();
		
		return removeDRMSrcFile;
	}
	
	// 3.0 feature
	@Override
	public int getDSCheckoutRetryCount() {
		initDSCheckoutRetryCount();
		
		return dsCheckoutRetryCount;
	}
	
	// 3.0 feature
	@Override
	public int getDSCheckoutWaitTime() {
		initDSCheckoutWaitTime();
		
		return dsCheckoutWaitTime;
	}
	
}
