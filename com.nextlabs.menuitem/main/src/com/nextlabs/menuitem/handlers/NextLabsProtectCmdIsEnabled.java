package com.nextlabs.menuitem.handlers;

import java.util.HashMap;

import java.util.Map;

import org.eclipse.ui.AbstractSourceProvider;
import org.eclipse.ui.ISources;

import com.teamcenter.rac.aif.kernel.AbstractAIFSession;
import com.teamcenter.rac.aifrcp.AIFUtility;
import com.teamcenter.rac.kernel.TCPreferenceService;
import com.teamcenter.rac.kernel.TCSession;

import static com.nextlabs.menuitem.configuration.NextLabsConstants.*;

/**
 * Created on May 11, 2020
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2020 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public class NextLabsProtectCmdIsEnabled extends AbstractSourceProvider {
	
	public final static String NXL_PROTECT_CMD_IS_ENABLED = "com.nextlabs.menuitem.cmd.protect.isEnabled";
	private final static String TRUE = "ON";
	private final static String FALSE = "OFF";
	boolean isEnabled = true;
		
	@Override
	public String[] getProvidedSourceNames() {
		return new String[] {NXL_PROTECT_CMD_IS_ENABLED};
	}
	
	@Override
	public Map<String, String> getCurrentState() {
		getNxlPreference();
		
		Map<String, String> currentState = new HashMap<String, String>(1);
		String currentStateVal = isEnabled ? TRUE : FALSE;
		currentState.put(NXL_PROTECT_CMD_IS_ENABLED, currentStateVal);
		
		return currentState;
	}
	
	private void getNxlPreference() {
		//get TC preference: NXL_PROTECT_ENABLE
		try {
			AbstractAIFSession localAbstractAIFSession = AIFUtility.getDefaultSession();
			if ((localAbstractAIFSession instanceof TCSession) && 
					(localAbstractAIFSession.isLoggedIn())) {
				TCSession session = (TCSession) localAbstractAIFSession;
				
				TCPreferenceService tcPrefService = session.getPreferenceService();
				tcPrefService.refresh();
				isEnabled = tcPrefService.getLogicalValue(NXL_PROTECT_ENABLE);
				setNxlProtectCmdIsEnabled(isEnabled);
			}
		} catch (Exception ex) {
			System.out.println("NextLabsProtectCmdIsEnabled.getNxlPreference() caught exception: " + ex.getMessage());
			ex.printStackTrace(System.out);
		}
	}
	
	@Override
	public void dispose() {
		
	}
	
	public void setNxlProtectCmdIsEnabled(boolean enable) {
		if (isEnabled == enable)
			return; // no change
		
		isEnabled = enable;
		String currentStateVal = isEnabled ? TRUE : FALSE;
		fireSourceChanged(ISources.WORKBENCH, NXL_PROTECT_CMD_IS_ENABLED, currentStateVal);
	}

}
