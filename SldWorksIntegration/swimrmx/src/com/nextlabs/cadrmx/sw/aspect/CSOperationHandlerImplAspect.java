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
import com.transcendata.cadpdm.cs.CSModelHelper;
import com.transcendata.cadpdm.OperationElement;
import com.transcendata.cadpdm.ServiceManagerHolder;
import com.transcendata.cadpdm.OperationCollection;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.util.RMXLogHolder;

@Aspect
public class CSOperationHandlerImplAspect {
	
	//boolean CSOperationHandlerImpl.openModels(OperationElement elem, boolean display) throws CADPDMException
	@Around("execution(protected boolean com.transcendata.cadpdm.cs.CSOperationHandlerImpl.openModels(..)) " +
			"&& !within(com.nextlabs.cadrmx.sw.aspect.CSOperationHandlerImplAspect)")
	public boolean around_openModels(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_CSOperationHandlerImpl.openModels");
		
		if (!SwimRMXSession.isRMXRunning()) {
			
			RMXLogHolder.error("Ignore to check right. SwimRMX not initialized");
			return (boolean)jp.proceed();
		}
		
		Object[] args = jp.getArgs();
		for (Object arg : args) {
			if (arg instanceof OperationElement) {
				OperationElement masterElem = CSModelHelper.getTopGenericElement((OperationElement)arg);
				String path = masterElem.getCADFile().getPath();
				RMXLogHolder.debug("Original path: " + path);
				if (path.endsWith(".nxl")) {
					path = path.substring(0, path.length() - 4);
					masterElem.setCADFile(ServiceManagerHolder.get().newCADFile(path));
					RMXLogHolder.debug("New path by removing .nxl extention: " + path);
				}
				
				break;
			}
		}
		
		RMXLogHolder.exitAspect("around_CSOperationHandlerImpl.openModels");
		return (boolean)jp.proceed();
		
	}
	
	//public void openInBackground(OperationCollection c, Map<Object, Object> failedCadIdMap, Collection modelstoOpen) throws CADPDMException {
	@Around("execution(public void com.transcendata.cadpdm.cs.CSOperationHandlerImpl.openInBackground(..)) " +
			"&& !within(com.nextlabs.cadrmx.sw.aspect.CSOperationHandlerImplAspect)")
	public void around_openInBackground(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_CSOperationHandlerImpl.openInBackground");
		
		if (!SwimRMXSession.isRMXRunning()) {
			
			RMXLogHolder.error("Ignore to check right. SwimRMX not initialized");
			return;
		}
		
		OperationCollection c = null;
		Collection modelstoOpen = null;
		Object[] args = jp.getArgs();
		for (Object arg : args) {
			if (arg instanceof OperationCollection) {
				c = (OperationCollection)arg;
			}
			if (arg instanceof Collection) {
				modelstoOpen = (Collection)arg;
			}
		}
		if (c != null && modelstoOpen != null) {
			Iterator<OperationElement> iter = IteratorHelper.newOperandIterator(c.getIterator());
			if (modelstoOpen != null) {
				iter = modelstoOpen.iterator();
			}
			
			while (iter.hasNext()) {
				OperationElement elem = iter.next();
				if (elem.isMaster() && !CSModelHelper.isToolboxPart(elem)) {
					String path = elem.getCADFile().getPath();
					RMXLogHolder.debug("Original path: " + path);
					if (path.endsWith(".nxl")) {
						path = path.substring(0, path.length() - 4);
						elem.setCADFile(ServiceManagerHolder.get().newCADFile(path));
						RMXLogHolder.debug("New path by removing .nxl extention: " + path);
					}
				}
			}
		}
		
		jp.proceed();
		RMXLogHolder.exitAspect("around_CSOperationHandlerImpl.openInBackground");
	}
}
