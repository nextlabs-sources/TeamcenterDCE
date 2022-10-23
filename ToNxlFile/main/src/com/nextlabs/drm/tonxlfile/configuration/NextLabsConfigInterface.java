package com.nextlabs.drm.tonxlfile.configuration;

import java.util.Map;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public interface NextLabsConfigInterface {
	
	// 5.2 feature
	public boolean getPurgeRevSeq();
	
	// 4.5 feature
	public Map<String, String> getInstalledFeatures();
	
	// 3.5 feature
	public String getPdpHost();
	
	// 3.5 feature
	public String getPdpPort();
	
	public String getPdpHttps();
	
	public String getPdpIgnoreHttps();
	
	public String getOAuth2Host();
	
	public String getOAuth2Port();
	
	public String getOAuth2Https();
	
	public String getOAuth2ClientID();
	
	public String getOAuth2ClientSecret();
	
	// 3.5 feature
	public String getPdpDefaultAction();
	
	// 3.5 feature
	public String getPdpDefaultMessage();
	
	public String[] getTransferredAttr();
	
	public String[] getEvaluationAttr();
	
	public boolean getRemoveDRMSrcFile();
	
	// 3.0 feature
	public String[] getNRExclusionList();
	
	// 3.0 feature
	public int getDSCheckoutRetryCount();
	
	// 3.0 feature
	public int getDSCheckoutWaitTime();
	
}
