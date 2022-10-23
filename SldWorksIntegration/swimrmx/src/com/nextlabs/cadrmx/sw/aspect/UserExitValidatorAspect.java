/*
 * Created on September 17, 2020
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
import java.util.Iterator;
import java.util.Map;
import java.awt.Component;


import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CADFile;
import com.transcendata.cadpdm.CADIdentifier;
import com.transcendata.cadpdm.CADOperationHandler;
import com.transcendata.cadpdm.CheckInCollection;
import com.transcendata.cadpdm.IteratorHelper;
import com.transcendata.cadpdm.OperationElement;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.util.RMXLogHolder;

@Aspect
public class UserExitValidatorAspect {
	
	private static final String ERR_MSG_SAVE_DENY = "Operation Denied. You do not have 'Edit' permission to save the following model(s). \nTo continue, return to the dialog and unselect them:\n";
	private static final String ERR_MSG_SAVEJT_DENY = "Operation Denied. You do not have 'Save As' permission to export the following model(s) as JT file(s). \nTo continue, return to the dialog and unselect them:\n";

	//public boolean validate()
	@Around("execution(public boolean com.transcendata.cadpdm.soa.UserExitValidator.validate(..)) " +
			"&& !within(com.nextlabs.cadrmx.sw.aspect.UserExitValidatorAspect)")
	public boolean around_validate(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_UserExitValidator.validate");
		
		if (!SwimRMXSession.isRMXRunning()) {
			
			RMXLogHolder.error("Ignore to check right. SwimRMX not initialized");
			return (boolean)jp.proceed();
		}
		
		ArrayList<String> denyModels = new ArrayList();
		ArrayList<String> denySaveJTModels = new ArrayList();
		
		Object instance = jp.getThis();
		Field private_coll = instance.getClass().getDeclaredField("_collection");
		private_coll.setAccessible(true);
		
		CheckInCollection _collection = (CheckInCollection)private_coll.get(instance);
		if (_collection != null) {
			Boolean saveJT = SwimRMXSession.saveAuxFileEnabled();
			if (saveJT == null) {
				saveJT = Boolean.FALSE;
				// In case CheckInDialog not shown, check from default values.
				Map<String, Boolean> auxMap = _collection.getAuxFileEnablingMap();
				for (Map.Entry<String, Boolean> entry : auxMap.entrySet()) {
					if (entry.getValue() == Boolean.TRUE) {
						RMXLogHolder.debug("Auxi file save: " + entry.getKey());
						saveJT = Boolean.TRUE;
						break;
					}
				}
			}
			Iterator<OperationElement> iter = IteratorHelper.newOperandIterator(_collection.getIterator());
			while (iter.hasNext()) {
				 OperationElement elem = iter.next();
				 CADFile cadFile = elem.getCADFile();
				 
				 RMXLogHolder.info("Checking if file has permission to save...");
				if(!SwimRMXSession.checkSaveRight(cadFile.getPath()).getOperationStatus()) {
					denyModels.add(elem.getCADIdentifier().getCADIdForMessage());		
				}		
				else if (saveJT) {
					RMXLogHolder.info("Checking if file has permission to save JT files...");
					if (!SwimRMXSession.checkExportRight(cadFile.getPath()).getOperationStatus()) {
						denySaveJTModels.add(elem.getCADIdentifier().getCADIdForMessage());
					}
				}
			}
		}
		if (denyModels.size() > 0) {
			String errMsg = ERR_MSG_SAVE_DENY + denyModels;	
			RMXLogHolder.error("Ignore to proceed UserExitValidator.validate( error: " +  errMsg + " )");
			SwimRMXSession.promptAlertDialog(errMsg);
			return false;
		} else if (denySaveJTModels.size() > 0) {
			String errMsg = ERR_MSG_SAVEJT_DENY + denySaveJTModels;
			RMXLogHolder.error("Ignore to proceed UserExitValidator.validate( error: " + errMsg + " )");
			SwimRMXSession.promptAlertDialog(errMsg);
			return false;
		}
		else {
			RMXLogHolder.debug("SwimRMX - Proceeding UserExitValidator.validate...");
			
			boolean result = (boolean)jp.proceed();
			
			RMXLogHolder.exitAspect("around_UserExitValidator.validate");
			
			return result;
		}
	}

}
