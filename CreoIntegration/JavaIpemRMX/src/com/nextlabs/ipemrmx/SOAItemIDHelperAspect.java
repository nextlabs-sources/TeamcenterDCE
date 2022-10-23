/*
 * Created on September 17, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.ipemrmx;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.awt.Component;


import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CADFile;
import com.transcendata.cadpdm.CADIdentifier;
import com.transcendata.cadpdm.CADOperationHandler;
import com.transcendata.cadpdm.OperationElement;
import com.transcendata.cadpdm.pe.PEIdentifier;
import com.transcendata.cadpdm.ServiceManager;
import com.transcendata.util.UserCancelException;

import com.nextlabs.ipemrmx.IpemRMXBase;
import com.nextlabs.ipemrmx.IpemRMXSession;
import com.nextlabs.ipemrmx.RMXLogHolder;

@Aspect
public class SOAItemIDHelperAspect extends IpemRMXBase {
	
	private static final String ERR_MSG_SAVEAS_DENY = "Operation Denied. You do not have 'Save As/Edit' permission to save as the following model(s). \nTo continue, return to the dialog and unselect them:\n";
	
	//protected void updateIDs(OperationElement[] elems, String[] newIDs, String[] newRevIDs, String[] newModelNames, Component cpt)
	@Around("execution(protected void com.transcendata.cadpdm.soa.gui.SOAItemIDHelper.updateIDs(..)) " +
	"&& !within(com.nextlabs.ipemrmx.SOAItemIDHelperAspect)")
	public void around_updateIDs(ProceedingJoinPoint jp) throws Throwable, UserCancelException{
		RMXLogHolder.enteringAspect("around_SOAItemIDHelper.updateIDs");
		
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			
			RMXLogHolder.error("Ignore to check right. IpemRMX not initialized");
			jp.proceed();
			return;
		}
		
		Object[] args = jp.getArgs();
		ArrayList<String> denyModels = new ArrayList();
		for (Object arg : args) {
			if (arg instanceof OperationElement[]) {
				OperationElement[] elems = (OperationElement[])arg;
				for(OperationElement elem : elems) {
					CADFile cadFile = elem.getCADFile();
					RMXLogHolder.info("Checking if file has permission to save as...");
					if (!rmxSess.checkSaveAsRight(cadFile.getPath())) {
						denyModels.add(elem.getCADIdentifier().getCADIdForMessage());		
					}
					
					/*else {
						  ServiceManager sm = ServiceManagerHolder.get();
						  CADOperationHandler opHandler = sm.getCADOperationHandler();
						  if(opHandler.isModelModifiedInSession(elem) && !IpemRMXHolder.AllowToSave(cadFile.getPath())) {
							  denyModels.add((PEIdentifier)elem.getCADIdentifier());
							  RMXLogHolder.error("Don't have edit permission on modified model: " + elem.getCADIdentifier());
								
						  }
					}*/
				}
			}	
		}
		
		if (denyModels.size() == 0) {
			RMXLogHolder.debug("Proceeding SOAItemIDHelper.updateIDs...");
			
			jp.proceed();
			
			RMXLogHolder.exitAspect("around_SOAItemIDHelper.updateIDs");
		}
		else {
			String errMsg = ERR_MSG_SAVEAS_DENY + denyModels;	
			RMXLogHolder.error("Ignore to proceed SOAItemIDHelper.updateIDs( error: " +  errMsg + " )");
			rmxSess.promptAlertDialog(errMsg);
			throw new UserCancelException();
		}
	}
}
