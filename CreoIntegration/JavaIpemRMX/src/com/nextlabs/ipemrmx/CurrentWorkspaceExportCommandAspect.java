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
public class CurrentWorkspaceExportCommandAspect extends IpemRMXBase {
	
	@Around("execution(protected void com.transcendata.cadpdm.soa.CurrentWorkspaceExportCommand.invokeCommand(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.CurrentWorkspaceExportCommandAspect)")
	public void around_invokeCommand(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_CurrentWorkspaceExportCommand.invokeCommand");
		
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess != null) {
			rmxSess.setCopyMonitorEnabled(Boolean.TRUE);
		}
		
		jp.proceed();

		if (rmxSess != null) {
			rmxSess.setCopyMonitorEnabled(Boolean.FALSE);
		}
		
		RMXLogHolder.exitAspect("around_CurrentWorkspaceExportCommand.invokeCommand");
	}
}