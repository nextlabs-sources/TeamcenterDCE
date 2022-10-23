/*
 * Created on 23 April, 2021
 *
 * All sources, binaries and HTML pages (C) copyright 2021 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.cadrmx.sw.aspect;

import java.io.File;
import java.lang.reflect.Field;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.util.RMXLogHolder;

/**
 * @author tcadmin
 *
 */
@Aspect
public class CurrentWorkspaceExportCommandAspect {
	
	@Around("execution(protected void com.transcendata.cadpdm.soa.CurrentWorkspaceExportCommand.invokeCommand(..)) "
			+ "&& !within(com.nextlabs.cadrmx.sw.aspect.CurrentWorkspaceExportCommandAspect)")
	public void around_invokeCommand(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_CurrentWorkspaceExportCommand.invokeCommand");
		
		if(!SwimRMXSession.isRMXRunning())  {
			jp.proceed();
			RMXLogHolder.exitAspect("around_CurrentWorkspaceExportCommand.invokeCommand");
			return;
		}
		
		SwimRMXSession.setCopyMonitorEnabled(Boolean.TRUE);

		jp.proceed();

		SwimRMXSession.setCopyMonitorEnabled(Boolean.FALSE);

		RMXLogHolder.exitAspect("around_CurrentWorkspaceExportCommand.invokeCommand");
	}
}