/*
 * Created on Dec 16, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.cadrmx.sw.aspect;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.awt.Component;


import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CADFile;
import com.transcendata.cadpdm.CADIdentifier;
import com.transcendata.cadpdm.OperationElement;
import com.transcendata.util.UserCancelException;
import com.transcendata.cadpdm.CADPDMException;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.sw.SwRMXResult;
import com.nextlabs.cadrmx.sw.SwRMXStatus;
import com.nextlabs.cadrmx.util.RMXLogHolder;

@Aspect
public class SOAPropertiesPanelAspect {
	
	private static final String ERR_MSG_SAVEAS_DENY = "Operation Denied. You do not have 'Save As' permission to revise the following model(s). \nTo continue, return to the dialog and unselect them:\n";
	private static final String ERR_MSG_EDIT_DENY = "Operation Denied. You do not have 'Edit' permission to revise the following modified model(s). \nTo continue, return to the dialog and unselect them:\n";
	
	// private void validateModelConfigurationsForRevise(Collection selectedElems, boolean retainSelection) throws UserCancelException, CADPDMException
	@Around("execution(private void com.transcendata.cadpdm.soa.gui.SOAPropertiesPanel.validateModelConfigurationsForRevise(..)) " +
	"&& !within(com.nextlabs.cadrmx.sw.aspect.SOAPropertiesPanelAspect)")
	public void around_validateModelConfigurationsForRevise(ProceedingJoinPoint jp) throws UserCancelException, CADPDMException, Throwable{
		RMXLogHolder.enteringAspect("around_SOAPropertiesPanel.validateModelConfigurationsForRevise");

		if (!SwimRMXSession.isRMXRunning()) {		
			RMXLogHolder.error("Ignore to check right. SwimRMX not initialized");
			jp.proceed();
			return;
		}
		
		Object[] args = jp.getArgs();
		ArrayList<String> denyModels = new ArrayList();
		ArrayList<String> denyEditModels = new ArrayList();
		for (Object arg : args) {
			if (arg instanceof Collection) {
				Collection selectedElems = (Collection)arg;
				for(Object e : selectedElems) {
					OperationElement elem = (OperationElement)e;
					CADFile cadFile = elem.getCADFile();
					SwRMXResult ret = SwimRMXSession.checkSaveAsRight(cadFile.getPath());
					if (!ret.getOperationStatus()) {
						if (ret.getErrorReason() == 3 /*TC_AUTHFAIL_REVISE_NOEDITRIGHT*/) {
							denyEditModels.add(elem.getCADIdentifier().getCADIdForMessage());
						} else {
							denyModels.add(elem.getCADIdentifier().getCADIdForMessage());
						}	
					}
				}
			}	
		}
		if (denyEditModels.size() > 0) {
			String errMsg = ERR_MSG_EDIT_DENY + denyEditModels;	
			RMXLogHolder.error("Ignore to proceed SOAItemIDHelper.updateIDs( error: " +  errMsg + " )");
			SwimRMXSession.promptAlertDialog(errMsg);
			throw new UserCancelException();
		}
		else if (denyModels.size() > 0){
			String errMsg = ERR_MSG_SAVEAS_DENY + denyModels;	
			RMXLogHolder.error("Ignore to proceed SOAItemIDHelper.updateIDs( error: " +  errMsg + " )");
			SwimRMXSession.promptAlertDialog(errMsg);
			throw new UserCancelException();
		}
		else {
			RMXLogHolder.debug("Proceeding SOAPropertiesPanel.validateModelConfigurationsForRevise...");
			
			jp.proceed();
			
			RMXLogHolder.exitAspect("around_SOAPropertiesPanel.validateModelConfigurationsForRevise");
		}
	}
}
