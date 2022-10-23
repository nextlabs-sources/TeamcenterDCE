package com.nextlabs.cadrmx.sw.aspect;

import java.io.File;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.CADFile;
import com.transcendata.util.LogHolder;
import com.transcendata.util.UserCancelException;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.util.RMXLogHolder;

@Aspect
public class CADFileAspect {
	@Around("execution(public boolean com.transcendata.cadpdm.CADFile.setReadOnly(..)) " +
			"&& !within(com.nextlabs.cadrmx.sw.aspect.CADFileAspect)")
			public boolean around_setReadOnly(ProceedingJoinPoint jp) throws Exception {
				RMXLogHolder.enteringAspect("around_CADFile.setReadOnly");
				
				boolean result = (boolean)jp.proceed();
				
				if (SwimRMXSession.isRMXRunning()) {
					CADFile instance = (CADFile)jp.getThis();
					RMXLogHolder.debug("around_CADFile.setReadOnly: " + instance.getPath());
					//SwimRMXSession.setReadOnly(instance.getPath());	
				}

				RMXLogHolder.exitAspect("around_CADFile.setReadOnly");
				return result;
	}
}
