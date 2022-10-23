/*
 * Created on 28 Mar, 2021
 *
 * All sources, binaries and HTML pages (C) copyright 2021 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.ipemrmx;

import java.io.File;
import java.lang.reflect.Field;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.OperationCollection;
import com.transcendata.ipemsoa.gui.CheckInDialogModel;
import com.transcendata.util.LogHolder;
import com.nextlabs.ipemrmx.RMXLogHolder;
import com.nextlabs.ipemrmx.IpemRMXBase;
import com.nextlabs.ipemrmx.IpemRMXSession;

/**
 * @author tcadmin
 *
 */
@Aspect
public class CurrentWorkspaceImportCommandAspect extends IpemRMXBase {
	
	@Around("execution(protected void com.transcendata.cadpdm.soa.CurrentWorkspaceImportCommand.invokeCommand(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.CurrentWorkspaceImportCommandAspect)")
	public void around_invokeCommand(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_CurrentWorkspaceImportCommand.invokeCommand");
		
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			RMXLogHolder.error("IpemRMX not initialized");
			jp.proceed();
			RMXLogHolder.exitAspect("around_CurrentWorkspaceImportCommand.invokeCommand");
			return;
		}
		
		rmxSess.setCopyMonitorEnabled(Boolean.TRUE);

		jp.proceed();

		rmxSess.setCopyMonitorEnabled(Boolean.FALSE);

		RMXLogHolder.exitAspect("around_CurrentWorkspaceImportCommand.invokeCommand");
	}
}