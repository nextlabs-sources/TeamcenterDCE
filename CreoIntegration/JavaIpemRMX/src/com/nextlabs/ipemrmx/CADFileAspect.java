package com.nextlabs.ipemrmx;

import java.io.File;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CADFile;
import com.transcendata.util.LogHolder;
import com.transcendata.util.UserCancelException;

import com.nextlabs.ipemrmx.IpemRMXBase;
import com.nextlabs.ipemrmx.IpemRMXSession;
import com.nextlabs.ipemrmx.RMXLogHolder;

@Aspect
public class CADFileAspect extends IpemRMXBase {
	@Around("execution(public boolean com.transcendata.cadpdm.CADFile.setReadOnly(..)) " +
			"&& !within(com.nextlabs.ipemrmx.CADFileAspect)")
			public boolean around_setReadOnly(ProceedingJoinPoint jp) throws Exception {
				RMXLogHolder.enteringAspect("around_CADFile.updateIDs");
				
				boolean result = (boolean)jp.proceed();
				
				RMXLogHolder.exitAspect("around_CADFile.updateIDs");
				
				IpemRMXSession rmxSess = getRMXSession();
				if (rmxSess != null) {
					CADFile instance = (CADFile)jp.getThis();	
					if (!rmxSess.isShareFolder(instance.getFile())) {
						rmxSess.setReadOnly(instance.getPath());	
					}
				}

				RMXLogHolder.exitAspect("around_CADFile.updateIDs");
				return result;
	}
}
