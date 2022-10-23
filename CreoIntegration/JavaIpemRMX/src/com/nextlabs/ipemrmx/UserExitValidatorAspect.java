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
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.awt.Component;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CADType;
import com.transcendata.cadpdm.CADFile;
import com.transcendata.cadpdm.CADIdentifier;
import com.transcendata.cadpdm.CADOperationHandler;
import com.transcendata.cadpdm.CheckInCollection;
import com.transcendata.cadpdm.IteratorHelper;
import com.transcendata.cadpdm.OperationCollection;
import com.transcendata.cadpdm.OperationElement;
import com.transcendata.cadpdm.pe.PEIdentifier;
import com.transcendata.cadpdm.ServiceManagerHolder;
import com.transcendata.cadpdm.AuxFileInfo;
import com.transcendata.util.ObjectUtil;

import javafx.util.Pair;

import com.nextlabs.ipemrmx.IpemRMXBase;
import com.nextlabs.ipemrmx.IpemRMXSession;
import com.nextlabs.ipemrmx.RMXLogHolder;

@Aspect
public class UserExitValidatorAspect extends IpemRMXBase {

	private static final String ERR_MSG_SAVE_DENY = "Operation Denied. You do not have 'Edit' permission to save the following model(s). \nTo continue, return to the dialog and unselect them:\n";
	private static final String ERR_MSG_SAVEJT_DENY = "Operation Denied. You do not have 'Save As' permission to export auxiliary file(s) on the following model(s). \nTo continue, return to the dialog and unselect them:\n";
	private static final String ERR_MSG_SAVEJT_DENY_DEP="Operation Denied.\nYou do not have 'Save As' permission to export auxiliary file(s) on the following dependent file(s):";
	
	@Around("execution(public boolean com.transcendata.cadpdm.soa.UserExitValidator.validate(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.UserExitValidatorAspect)")
	public boolean around_validate(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_UserExitValidator.validate");

		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {

			RMXLogHolder.error("Ignore to check right. IpemRMX not initialized");
			return (boolean) jp.proceed();
		}

		ArrayList<String> denyModels = new ArrayList();
		ArrayList<String> denySaveJTModels = new ArrayList();
		String denySaveJTDepModels = "";
		
		Object instance = jp.getThis();
		Field private_coll = instance.getClass().getDeclaredField("_collection");
		private_coll.setAccessible(true);

		CheckInCollection _collection = (CheckInCollection) private_coll.get(instance);
		if (_collection != null) {
			Boolean saveAuxFile = rmxSess.saveAuxFileEnabled();
			if (saveAuxFile == null) {
				saveAuxFile = Boolean.FALSE;
				// In case CheckInDialog not shown, check from default values.
				Map<String, Boolean> auxMap = _collection.getAuxFileEnablingMap();
				for (Map.Entry<String, Boolean> entry : auxMap.entrySet()) {
					if (entry.getValue() == Boolean.TRUE) {
						RMXLogHolder.debug("Auxi file save: " + entry.getKey());
						saveAuxFile = Boolean.TRUE;
						break;
					}
				}
			}
			Iterator<OperationElement> iter = IteratorHelper.newOperandIterator(_collection.getIterator());
			while (iter.hasNext()) {
				OperationElement elem = iter.next();
				CADFile cadFile = elem.getCADFile();
				RMXLogHolder.info("Checking if file has permission to save...");
				if (!rmxSess.checkSaveRight(cadFile.getPath())) {
					denyModels.add(elem.getCADIdentifier().getCADIdForMessage());
				}
				if (saveAuxFile == Boolean.TRUE) {
					RMXLogHolder.info("Checking if file has permission to save Aux(JT) files...");
					Pair<Boolean, String> ret = rmxSess.checkSaveAsRight(cadFile.getPath(), true);
					if (!ret.getKey()) {
						if (ret.getValue().isEmpty())
							denySaveJTModels.add(elem.getCADIdentifier().getCADIdForMessage());
						else
							denySaveJTDepModels = ret.getValue();
					}
				}
			}
		}
		if (denyModels.size() > 0) {
			String errMsg = ERR_MSG_SAVE_DENY + denyModels;
			RMXLogHolder.error("Ignore to proceed UserExitValidator.validate( error: " + errMsg + " )");
			rmxSess.promptAlertDialog(errMsg);
			return false;
		} else if (denySaveJTModels.size() > 0) {
			String errMsg = ERR_MSG_SAVEJT_DENY + denySaveJTModels;
			RMXLogHolder.error("Ignore to proceed UserExitValidator.validate( error: " + errMsg + " )");
			rmxSess.promptAlertDialog(errMsg);
			return false;
		} else if (!denySaveJTDepModels.isEmpty()) {
			String errMsg = ERR_MSG_SAVEJT_DENY_DEP + denySaveJTDepModels;
			RMXLogHolder.error("Ignore to proceed UserExitValidator.validate( error: " + errMsg + " )");
			rmxSess.promptAlertDialog(errMsg);
			return false;
		} else {
			RMXLogHolder.debug("IpemRMX - Proceeding UserExitValidator.validate...");

			boolean result = (boolean) jp.proceed();

			RMXLogHolder.exitAspect("around_UserExitValidator.validate");

			return result;
		}
	}
}
