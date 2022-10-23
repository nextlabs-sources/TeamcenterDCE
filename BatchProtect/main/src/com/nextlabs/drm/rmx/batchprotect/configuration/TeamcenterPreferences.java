package com.nextlabs.drm.rmx.batchprotect.configuration;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import static com.nextlabs.drm.rmx.batchprotect.configuration.NextLabsPolicyConstants.*;
import com.nextlabs.drm.rmx.batchprotect.tcsession.TCSession;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.loose.core.SessionService;
import com.teamcenter.services.loose.core._2006_03.Session.PreferencesResponse;

public class TeamcenterPreferences {
	
	private TCSession session;
	private SessionService sessionService;
	private String prefNames[] = new String[] { 
			NXL_PDP_DEFAULT_ACTION,
			NXL_PDP_DEFAULT_MESSAGE, 
			NXL_PDPHOST_SERVERAPP,
			NXL_PDPPORT_SERVERAPP,
			NXL_PDP_HTTPS_SERVERAPP,
			NXL_PDP_IGNOREHTTPS_SERVERAPP,
			NXL_OAUTH2HOST,
			NXL_OAUTH2PORT,
			NXL_OAUTH2_HTTPS,
			NXL_OAUTH2_CLIENTID,
			NXL_OAUTH2_CLIENTSECRET,
			NXL_EVALUATION_ATTRIBUTES};
	PreferencesResponse preferenceResponse;
	private String pdpDefaultAction;
	private String pdpDefaultMessage;

	@SuppressWarnings("deprecation")
	public TeamcenterPreferences(TCSession tcsession) throws ServiceException {
		this.session = tcsession;
		sessionService = SessionService.getService(session.getConnection());
		preferenceResponse = sessionService.getPreferences("site", prefNames);
	}

	public String getPdpHost() {
		String pdpHostServerApp = preferenceResponse.preferences.getPreference(
				NXL_PDPHOST_SERVERAPP).get(0);
		return pdpHostServerApp;
	}

	public String getPdpPort() {
		String pdpPortServerApp = preferenceResponse.preferences.getPreference(
				NXL_PDPPORT_SERVERAPP).get(0);
		return pdpPortServerApp;
	}
	
	public String getPdpHttps() {
		String pdpHttpsServerApp = preferenceResponse.preferences.getPreference(
				NXL_PDP_HTTPS_SERVERAPP).get(0);
		return pdpHttpsServerApp;
	}
	
	public String getPdpIgnoreHttps() {
		String pdpIgnoreHttps = preferenceResponse.preferences.getPreference(
				NXL_PDP_IGNOREHTTPS_SERVERAPP).get(0);
		return pdpIgnoreHttps;
	}
	
	public String getOAuth2Host() {
		String oauth2Host = preferenceResponse.preferences.getPreference(
				NXL_OAUTH2HOST).get(0);
		return oauth2Host;
	}
	
	public String getOAuth2Port() {
		String oauth2Port = preferenceResponse.preferences.getPreference(
				NXL_OAUTH2PORT).get(0);
		return oauth2Port;
	}
	
	public String getOAuth2Https() {
		String oauth2Https = preferenceResponse.preferences.getPreference(
				NXL_OAUTH2_HTTPS).get(0);
		return oauth2Https;
	}
	
	public String getOAuth2ClientID() {
		String oauth2ClientID = preferenceResponse.preferences.getPreference(
				NXL_OAUTH2_CLIENTID).get(0);
		return oauth2ClientID;
	}
	
	public String getOAuth2ClientSecret() {
		String oauth2ClientSecret = preferenceResponse.preferences.getPreference(
				NXL_OAUTH2_CLIENTSECRET).get(0);
		return oauth2ClientSecret;
	}

	public String[] getEvaAttrArray() {
		String[] evaAttrArray = preferenceResponse.preferences.getPreference(
				NXL_EVALUATION_ATTRIBUTES).toArray(new String[0]);
		return evaAttrArray;
	}

	public String getPdpDefaultAction() {
		pdpDefaultAction = preferenceResponse.preferences.getPreference(
				NXL_PDP_DEFAULT_ACTION).get(0);
		return pdpDefaultAction;
	}

	public String getPdpDefaultMessage() {
		pdpDefaultMessage = preferenceResponse.preferences.getPreference(
				NXL_PDP_DEFAULT_MESSAGE).get(0);
		return pdpDefaultMessage;
	}

}
