/*
 * Created on Nov 24, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.cadrmx.sw.aspect;

import java.lang.reflect.Field;
import java.util.Map;
import java.util.ResourceBundle;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.After;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CheckInCollection;
import com.transcendata.cadpdm.gui.AuxFileEnablerPanel;
import com.transcendata.swimsoa.gui.CheckInDialogModel;
import com.transcendata.cadpdm.ServiceManagerHolder;
import com.transcendata.util.ObjectUtil;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.util.RMXLogHolder;

@Aspect
public class CheckinDialogAspect {

	@After("execution(protected void com.transcendata.swimsoa.gui.CheckInDialog.cancelAction(..)) "
			+ "&& !within(com.nextlabs.cadrmx.sw.aspect.CheckinDialogAspect)")
	public void after_cancelAction(JoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("after_CheckInDialog.cancelAction");
		
		// Reset
		SwimRMXSession.setSaveAuxFileEnabled(null);
		
		RMXLogHolder.exitAspect("after_CheckInDialog.cancelAction");
	}
	
	@Before("execution(protected void com.transcendata.swimsoa.gui.CheckInDialog.okAction(..)) "
			+ "&& !within(com.nextlabs.cadrmx.sw.aspect.CheckinDialogAspect)")
	public void before_okAction(JoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("before_CheckInDialog.okAction");
		try {
			Object instance = jp.getThis();
	
			Field private_enablers = instance.getClass().getDeclaredField("_enablers");
			private_enablers.setAccessible(true);
			AuxFileEnablerPanel _enablers = (AuxFileEnablerPanel) private_enablers.get(instance);
	
			Field private_model = instance.getClass().getDeclaredField("_model");
			private_model.setAccessible(true);
			CheckInDialogModel _dlgModel = (CheckInDialogModel) private_model.get(instance);
			Boolean saveJT = Boolean.FALSE;
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
							if (label.equals(eaiLabel)) {
								saveJT = _enablers.isAuxFileEnabled(i) ? Boolean.TRUE : Boolean.FALSE;
								break;
							}
						}
					}
				}
			}
	
			SwimRMXSession.setSaveAuxFileEnabled(saveJT);
			RMXLogHolder.info("Save JT Files:" + saveJT);
		} catch(Exception e) {
			RMXLogHolder.error("Error occurs when querying Save JT Files option ", e);
		} finally {
			RMXLogHolder.exitAspect("before_CheckInDialog.okAction");
		}
	}
}
