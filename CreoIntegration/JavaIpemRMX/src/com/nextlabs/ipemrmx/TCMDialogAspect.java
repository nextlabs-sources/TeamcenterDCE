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

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.After;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.OperationCollection;
import com.transcendata.cadpdm.OperationSubType;
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
public class TCMDialogAspect extends IpemRMXBase {
	
	//private void refreshSelectedModels(OperationSubType subType) {
	@After("execution(private void com.transcendata.ipemsoatwm.gui.TCMDialog.refreshSelectedModels(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.TCMDialogAspect)")
	public void after_refreshSelectedModels(JoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("after_TCMDialog.refreshSelectedModels");
		
		IpemRMXSession rmxSess = getRMXSession();
		if(rmxSess != null)  {
			rmxSess.fixNxlExtInWorkspace();
		}
		
		RMXLogHolder.exitAspect("after_TCMDialog.refreshSelectedModels");
	}
}