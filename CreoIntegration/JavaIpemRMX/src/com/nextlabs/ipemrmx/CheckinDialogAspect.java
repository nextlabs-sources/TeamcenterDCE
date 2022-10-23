/*
 * Created on Nov 24, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.ipemrmx;

import java.lang.reflect.Field;
import java.util.Map;
import java.util.ResourceBundle;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.After;

import com.transcendata.cadpdm.CheckInCollection;
import com.transcendata.cadpdm.gui.AuxFileEnablerPanel;
import com.transcendata.ipemsoa.gui.CheckInDialogModel;
import com.transcendata.cadpdm.ServiceManagerHolder;
import com.transcendata.util.ObjectUtil;

import com.nextlabs.ipemrmx.RMXLogHolder;
import com.nextlabs.ipemrmx.IpemRMXBase;
import com.nextlabs.ipemrmx.IpemRMXSession;

@Aspect
public class CheckinDialogAspect extends IpemRMXBase {

	@After("execution(protected void com.transcendata.ipemsoa.gui.CheckInDialog.cancelAction(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.CheckinDialogAspect)")
	public void After_cancelAction(JoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("After_CheckInDialog.cancelAction");
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			RMXLogHolder.error("Ignore to check right. IpemRMX not initialized");
			return;
		}
		// Reset
		rmxSess.setSaveAuxFileEnabled(null);
		RMXLogHolder.exitAspect("After_CheckInDialog.cancelAction");
	}
	
	@Before("execution(protected void com.transcendata.ipemsoa.gui.CheckInDialog.okAction(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.CheckinDialogAspect)")
	public void before_okAction(JoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("before_CheckInDialog.okAction");
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			RMXLogHolder.error("Ignore to check right. IpemRMX not initialized");
			return;
		}

		Object instance = jp.getThis();

		Field private_enablers = instance.getClass().getDeclaredField("_enablers");
		private_enablers.setAccessible(true);
		AuxFileEnablerPanel _enablers = (AuxFileEnablerPanel) private_enablers.get(instance);

		Field private_model = instance.getClass().getDeclaredField("_model");
		private_model.setAccessible(true);
		CheckInDialogModel _dlgModel = (CheckInDialogModel) private_model.get(instance);
		Boolean saveAuxFile = Boolean.FALSE;
		if (_enablers != null && _dlgModel != null) {
			ResourceBundle rb = ServiceManagerHolder.get().getResourceBundle();
			String eaiLabel = rb.getString("com.transcendata.cadpdm.eai.EAICommand.lblEnabler");
			CheckInCollection c = (CheckInCollection) _dlgModel.getOperationCollection();
			if (c != null) {
				Map<String, Boolean> enablers = c.getAuxFileEnablingMap();
				if (!ObjectUtil.isEmpty(enablers)) {
					int n = _enablers.getAuxFileCount();
					for (int i = 0; i < n; i++) {
						String label = _enablers.getAuxFileLabel(i);
						saveAuxFile = _enablers.isAuxFileEnabled(i);
						RMXLogHolder.debug("AuxFileEnabler: " + label + ", " + saveAuxFile);
						// One o f save auxiliary file is selected 
						if (saveAuxFile) {							
							break;
						}		
					}
				}
			}
		}

		rmxSess.setSaveAuxFileEnabled(saveAuxFile);
		RMXLogHolder.info("Save auxiliary files:" + saveAuxFile);
		RMXLogHolder.exitAspect("before_CheckInDialog.okAction");
	}
}
